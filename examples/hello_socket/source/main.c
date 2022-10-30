// Required header
#include <ps5/payload_main.h>

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ps5/libkernel.h>

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

	sprintf(printbuf, "Hello from the PS5!\n");
	sock_print(sock, printbuf);

	_close(sock);
	return 0;
}
