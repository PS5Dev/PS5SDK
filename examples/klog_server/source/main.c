// Required header
#include <ps5/payload_main.h>

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ps5/libkernel.h>

// This payload is firmware dependent because fork() must be resolved manually.
#if PS5_FW_VERSION == 0x403
#define OFFSET_FORK_FROM_READ 0x1DE0
#else
#error Klog server does not support this firmware, must be updated to be able to find fork().
#endif

// Ran in child process
void run_kernel_log_server(int port)
{
	// Establish a server
	int client = 0;
	int s;

	s = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in sockaddr;
	bzero(&sockaddr, sizeof(sockaddr));

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = INADDR_ANY;

	if(bind(s, (const struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0)
		return;

	if(listen(s, 5) < 0)
		return;

	// Accept clients
	for(;;) 
	{
		client = _accept(s, 0, 0);

		if(client > 0)
		{
			// Open klog and read to client
			int fd, ret;
			char temp[0x2000];

			struct timespec ts;
			ts.tv_nsec = 100000000;
			ts.tv_sec = 0;

			fd = _open("/dev/klog", 0, 0);

			if(fd == -1)
			{
				_write(client, "Cannot open /dev/klog\n", 23);
				break;
			}

			_write(client, "Successfully opened klog!\n", 10);

			for(;;)
			{
				// Sleep to save CPU usage
				_nanosleep(&ts, 0);
				
				ret = _read(fd, temp, 0xFFF);

				if (ret > 0)
				{
					temp[ret + 1] = 0x00;

					if(_write(client, temp, ret + 1) != (ret + 1))
					{
						_close(fd);
						_close(client);

						break;
					}
				}
			}
		}
	}
}

extern void *fptr__read;

int payload_main(struct payload_args *args)
{
	int mainPID;
	uint64_t temp_addr;

	// Fork must be resolved manually, dlsym refuses to resolve it
	void (*fptr_fork)() = (void (*)())(fptr__read + OFFSET_FORK_FROM_READ);

	// Fork so we can keep a klog server outside the browser process
	mainPID = getpid();
	fptr_fork();

	if (getpid() != mainPID)
	{
		run_kernel_log_server(9081);
	}

	return 0;
}