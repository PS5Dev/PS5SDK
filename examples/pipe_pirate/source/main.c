// Required header
#include <ps5/payload_main.h>

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ps5/libkernel.h>
#include <ps5/kernel.h>

#define PC_IP   "10.0.0.193"
#define PC_PORT 5655

void sock_print(int sock, char *str)
{
	size_t size;

	size = strlen(str);
	_write(sock, str, size);
}

int payload_main(struct payload_args *args)
{
	int ret;
	int sock;
	char printbuf[128];
	struct sockaddr_in addr;
	uint64_t kdata_base;
	pid_t pid;

	kdata_base = args->kdata_base_addr;

	// Open a debug socket to log to PC
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		return -1;
	}

	inet_pton(AF_INET, PC_IP, &addr.sin_addr);
	addr.sin_family = AF_INET;
	addr.sin_len    = sizeof(addr);
	addr.sin_port   = htons(PC_PORT);

	ret = connect(sock, (const struct sockaddr *) &addr, sizeof(addr));
	if (ret < 0) {
		return -1;
	}

	// Print basic info
	sprintf(printbuf, "[+] kernel .data base is %p, pipe %d->%d, rw pair %d->%d, pipe addr is %p\n",
			args->kdata_base_addr, args->rwpipe[0], args->rwpipe[1], args->rwpair[0], args->rwpair[1], args->kpipe_addr);
	sock_print(sock, printbuf);

	pid = getpid();

	sprintf(printbuf, "[+] PID = 0x%x\n", pid);
	sock_print(sock, printbuf);

	// Initialize kernel read/write helpers
	kernel_init_rw(args->rwpair[0], args->rwpair[1], args->rwpipe, args->kpipe_addr);

	// Find allproc and iterate the list to find our process
	uint64_t allproc_addr        = kdata_base + OFFSET_KERNEL_DATA_BASE_ALLPROC;
	uint64_t cur_proc_addr       = 0;
	uint64_t next_proc_addr      = 0;
	uint32_t cur_proc_pid        = 0;
	uint64_t cur_proc_ucred_addr = 0;

	kernel_copyout(allproc_addr, &cur_proc_addr, sizeof(cur_proc_addr));

	for (;;) {
		// Get next proc
		kernel_copyout(cur_proc_addr, &next_proc_addr, sizeof(next_proc_addr));

		// Check PID
		kernel_copyout(cur_proc_addr + OFFSET_KERNEL_PROC_P_PID, &cur_proc_pid, sizeof(cur_proc_pid));
		if (cur_proc_pid == pid) {
			// Get ucred
			kernel_copyout(cur_proc_addr + OFFSET_KERNEL_PROC_P_UCRED, &cur_proc_ucred_addr, sizeof(cur_proc_ucred_addr));
			break;
		}

		if (next_proc_addr == 0)
			break;

		// Move to next proc
		cur_proc_addr = next_proc_addr;
	}

	// If we failed to find ucred, go no further - avoid panic for no reason
	if (cur_proc_ucred_addr == 0) {
		sprintf(printbuf, "[!] failed to find ucred :(\n");
		sock_print(sock, printbuf);
		goto out;
	}

	sprintf(printbuf, "[+] ucred = %p\n", cur_proc_ucred_addr);
	sock_print(sock, printbuf);

	// Patch creds to be a tagged uid so we know write works
	uint32_t tagged_uid = 0x1337;

	kernel_copyin(&tagged_uid, cur_proc_ucred_addr + OFFSET_KERNEL_UCRED_CR_UID, sizeof(tagged_uid));
	kernel_copyin(&tagged_uid, cur_proc_ucred_addr + OFFSET_KERNEL_UCRED_CR_RUID, sizeof(tagged_uid));
	kernel_copyin(&tagged_uid, cur_proc_ucred_addr + OFFSET_KERNEL_UCRED_CR_SVUID, sizeof(tagged_uid));
	kernel_copyin(&tagged_uid, cur_proc_ucred_addr + OFFSET_KERNEL_UCRED_CR_RGID, sizeof(tagged_uid));

	// Get uid and check for tag
	uid_t test_uid = getuid();

	sprintf(printbuf, "[+] did we patch uid? uid = 0x%x (tag is 0x%x)\n", test_uid, tagged_uid);
	sock_print(sock, printbuf);

out:
	_close(sock);
	return 0;
}
