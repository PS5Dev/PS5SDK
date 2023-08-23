// Required header
#include <ps5/payload_main.h>

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <ps5/libkernel.h>
#include <ps5/kernel.h>

#define ENABLE_LOGS 0
#define PC_IP   "10.0.3.3"
#define PC_PORT 5655

#define NAND_A53IO_OPEN                 0x80046101
#define NAND_A53IO_DISABLE_CONTROLLER   0x80046104

#define PUP_UPDATER_READ_NAND_GROUP     0xC018440A

struct ioctl_readnandgroup_args
{
    uint64_t group_id;
    uint64_t p_out;
    uint64_t size;
};

void sock_print(int sock, char *str)
{
	size_t size;

#if ENABLE_LOGS
	size = strlen(str);
	_write(sock, str, size);
#endif
}

int payload_main(struct payload_args *args)
{
    int ret;
    int sock;
    int out_fds[3];
    int a53_fd;
    int pupupdate_fd;
    int zero;
    int written_bytes;
    char printbuf[128];
    struct sockaddr_in addr;
    struct OrbisKernelSwVersion version;
    void *out_data;
    uint64_t nand_size;
    uint64_t kdata_base;
    pid_t pid;

    zero         = 0;
    a53_fd       = -1;
    pupupdate_fd = -1;
    out_data     = NULL;
    kdata_base   = args->kdata_base_addr;

#if ENABLE_LOGS
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
#endif

    // Get system version
    sceKernelGetProsperoSystemSwVersion(&version);

    sprintf(printbuf, "[+] fw version 0x%x (%s)\n", version.version, version.version_str);
    sock_print(sock, printbuf);

    // Create output directory and files
    mkdir("/mnt/usb0/PS5", 0777);

    out_fds[0] = _open("/mnt/usb0/PS5/nandgroup0.bin", O_WRONLY | O_CREAT, 0644);
    if (out_fds[0] < 0) {
        sprintf(printbuf, "[+] failed to open output file (%d)\n", errno);
        sock_print(sock, printbuf);
        close(sock);
        return -1;
    }

    out_fds[1] = _open("/mnt/usb0/PS5/nandgroup1.bin", O_WRONLY | O_CREAT, 0644);
    if (out_fds[0] < 0) {
        sprintf(printbuf, "[+] failed to open output file (%d)\n", errno);
        sock_print(sock, printbuf);
        close(sock);
        return -1;
    }

    out_fds[2] = _open("/mnt/usb0/PS5/nandgroup2.bin", O_WRONLY | O_CREAT, 0644);
    if (out_fds[0] < 0) {
        sprintf(printbuf, "[+] failed to open output file (%d)\n", errno);
        sock_print(sock, printbuf);
        close(sock);
        return -1;
    }

    sprintf(printbuf, "[+] opened output files: %d, %d, %d\n", out_fds[0], out_fds[1], out_fds[2]);
    sock_print(sock, printbuf);

    // Print basic info
    sprintf(printbuf, "[+] kernel .data base is %p, pipe %d->%d, rw pair %d->%d, pipe addr is %p\n",
            args->kdata_base_addr, args->rwpipe[0], args->rwpipe[1], args->rwpair[0], args->rwpair[1], args->kpipe_addr);
    sock_print(sock, printbuf);

    pid = getpid();

    sprintf(printbuf, "[+] PID = 0x%x\n", pid);
    sock_print(sock, printbuf);

    // Initialize kernel read/write helpers
    kernel_init_rw(args->rwpair[0], args->rwpair[1], args->rwpipe, args->kpipe_addr);

    sprintf(printbuf, "[+] kernel r/w initialized...\n");
    sock_print(sock, printbuf);

    // Find allproc and iterate the list to find our process
    uint64_t allproc_addr        = kdata_base;
    uint64_t cur_proc_addr       = 0;
    uint64_t next_proc_addr      = 0;
    uint32_t cur_proc_pid        = 0;
    uint64_t cur_proc_ucred_addr = 0;

    // Do per-fw tailoring for allproc offset
    switch (version.version) {
    case 0x3000038:
    case 0x3100003:
    case 0x3200004:
    case 0x3210000:
        allproc_addr += 0x276DC58;
        break;
    case 0x4030000:
    case 0x4500005:
    case 0x4510001:
        allproc_addr += 0x27EDCB8;
        break;
    default:
        sprintf(printbuf, "[!] unsupported firmware, bailing...\n");
        sock_print(sock, printbuf);
        goto out;
    }

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

    // Patch UID
    uint32_t root_uid = 0;

    kernel_copyin(&root_uid, cur_proc_ucred_addr + OFFSET_KERNEL_UCRED_CR_UID, sizeof(root_uid));
    kernel_copyin(&root_uid, cur_proc_ucred_addr + OFFSET_KERNEL_UCRED_CR_RUID, sizeof(root_uid));
    kernel_copyin(&root_uid, cur_proc_ucred_addr + OFFSET_KERNEL_UCRED_CR_SVUID, sizeof(root_uid));
    kernel_copyin(&root_uid, cur_proc_ucred_addr + OFFSET_KERNEL_UCRED_CR_RGID, sizeof(root_uid));
    kernel_copyin(&root_uid, cur_proc_ucred_addr + OFFSET_KERNEL_UCRED_CR_SVGID, sizeof(root_uid));

    // Patch program auth ID
    uint64_t paid = 0x4801000000000013l;
    uint64_t caps = 0xFFFFFFFFFFFFFFFF;

    kernel_copyin(&paid, cur_proc_ucred_addr + 0x58, sizeof(paid));
    for (int i = 0x60; i < 0x70; i += 0x8) {
        kernel_copyin(&caps, cur_proc_ucred_addr + i, sizeof(caps));
    }

    // Open A53IO device to configure NAND
    a53_fd = _open("/dev/a53io", 2, 0);
    sprintf(printbuf, "[+] a53io dev = 0x%x (errno = %d)\n", a53_fd, errno);
    sock_print(sock, printbuf);

    if (a53_fd < 0) {
        sprintf(printbuf, "[!] failed to open a53 :(\n");
        sock_print(sock, printbuf);
        goto out;
    }

    // Disable the A53 controller, which is necessary to expose NAND to read
    ret = _ioctl(a53_fd, NAND_A53IO_DISABLE_CONTROLLER, &zero);
    sprintf(printbuf, "[+] disable controller = 0x%x (errno = %d)\n", ret, errno);
    sock_print(sock, printbuf);

    // Open the NAND groups for reading
    ret = _ioctl(a53_fd, NAND_A53IO_OPEN, &zero);
    sprintf(printbuf, "[+] open nand = %d (errno = %d)\n", ret, errno);
    sock_print(sock, printbuf);

    // Map buffer for output NAND group data
    out_data = mmap(0, 0x4000000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (out_data == NULL) {
        sprintf(printbuf, "[!] failed to map memory for nand data (%d) :(\n", errno);
        sock_print(sock, printbuf);
        goto out;
    }

    // Open pup update FD to do reading
    pupupdate_fd = _open("/dev/pup_update0", 2, 0);
    if (pupupdate_fd < 0) {
        sprintf(printbuf, "[!] failed to open pup_update0 (%d) :(\n",  errno);
        sock_print(sock, printbuf);
        goto out;
    }

    sprintf(printbuf, "[+] pup update dev = 0x%x (errno = %d)\n", pupupdate_fd, errno);
    sock_print(sock, printbuf);

    // Max 3 NAND groups
    for (int i = 0; i < 3; i++) {
        memset(out_data, 0, 0x4000000);

        struct ioctl_readnandgroup_args ioc_args = {};
        ioc_args.group_id = i;
        ioc_args.p_out = out_data;

        // NAND groups have different sizes:
        // group 0 is 0x4000000
        // group 1 is 0x3e00000
        // group 2 is 0x237800 on late revisions
        if (i == 0) {
            nand_size = 0x4000000;
        } else if (i == 1) {
            nand_size = 0x3e00000;
        } else {
            nand_size = 0x237800;
        }

        ioc_args.size = nand_size;

        // Read NAND data
        ret = _ioctl(pupupdate_fd, PUP_UPDATER_READ_NAND_GROUP, &ioc_args);
        if (ret != 0) {
            sprintf(printbuf, "[!] failed to read NAND (%d) :(\n", errno);
            sock_print(sock, printbuf);
            goto out;
        }

        sprintf(printbuf, "[+] read nand group %d = %d (errno = %d)\n", i, ret, errno);
        sock_print(sock, printbuf);

        // Dump to file
        written_bytes = _write(out_fds[i], out_data, nand_size);
        if (written_bytes != nand_size) {
            sprintf(printbuf, "[!] failed to write nand to file, %d != %d.\n", written_bytes, nand_size);
            sock_print(sock, printbuf);
            goto out;
        }

        sprintf(printbuf, "[+] wrote %d bytes...\n", written_bytes);
        sock_print(sock, printbuf);
    }

    sprintf(printbuf, "Done!\n");
    sock_print(sock, printbuf);

out:
    if (out_data != NULL)
        munmap(out_data, 0x4000000);

    if (a53_fd >= 0)
        _close(a53_fd);

    if (pupupdate_fd >= 0)
        _close(pupupdate_fd);

    _close(out_fds[0]);
    _close(out_fds[1]);
    _close(out_fds[2]);
#if ENABLE_LOGS
    _close(sock);
#endif

    return 0;
}
