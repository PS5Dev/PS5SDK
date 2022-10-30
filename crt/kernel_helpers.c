/*****************************************************
 * PS5 SDK - Kernel helpers
 * Implements kernel hacking API for doing kernel read/
 * write.
 ****************************************************/

#include <ps5/kernel.h>

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ps5/libkernel.h>

// Store necessary sockets/pipe for corruption.
int _master_sock;
int _victim_sock;
int _rw_pipe[2];
uint64_t _pipe_addr;

// Arguments passed by way of entrypoint arguments.
void kernel_init_rw(int master_sock, int victim_sock, int *rw_pipe, uint64_t pipe_addr)
{
	_master_sock = master_sock;
	_victim_sock = victim_sock;
	_rw_pipe[0]  = rw_pipe[0];
	_rw_pipe[1]  = rw_pipe[1];
	_pipe_addr   = pipe_addr;
}

// Internal kwrite function - not friendly, only for setting up better primitives.
void kwrite(uint64_t addr, uint64_t *data) {
	uint64_t victim_buf[3];

	victim_buf[0] = addr;
	victim_buf[1] = 0;
	victim_buf[2] = 0;

	setsockopt(_master_sock, IPPROTO_IPV6, IPV6_PKTINFO, victim_buf, 0x14);
	setsockopt(_victim_sock, IPPROTO_IPV6, IPV6_PKTINFO, data, 0x14);
}

// Public API function to write kernel data.
void kernel_copyin(void *src, uint64_t kdest, size_t length)
{
	uint64_t write_buf[3];

	// Set pipe flags
	write_buf[0] = 0;
	write_buf[1] = 0x4000000000000000;
	write_buf[2] = 0;
	kwrite(_pipe_addr, (uint64_t *) &write_buf);

	// Set pipe data addr
	write_buf[0] = kdest;
	write_buf[1] = 0;
	write_buf[2] = 0;
	kwrite(_pipe_addr + 0x10, (uint64_t *) &write_buf);

	// Perform write across pipe
	_write(_rw_pipe[1], src, length);
}

// Public API function to read kernel data.
void kernel_copyout(uint64_t ksrc, void *dest, size_t length)
{
	uint64_t write_buf[3];

	// Set pipe flags
	write_buf[0] = 0x4000000040000000;
	write_buf[1] = 0x4000000000000000;
	write_buf[2] = 0;
	kwrite(_pipe_addr, (uint64_t *) &write_buf);

	// Set pipe data addr
	write_buf[0] = ksrc;
	write_buf[1] = 0;
	write_buf[2] = 0;
	kwrite(_pipe_addr + 0x10, (uint64_t *) &write_buf);

	// Perform read across pipe
	_read(_rw_pipe[0], dest, length);
}