/*****************************************************
 * PS5 SDK - Kernel
 * Exposes kernel hacking API for doing kernel read/
 * write and includes header for offsets for the
 * targeted FW.
 ****************************************************/

#ifndef PS5SDK_KERNEL_H
#define PS5SDK_KERNEL_H

#include <stdint.h>
#include <stddef.h>

// Firmware includes
#if PS5_FW_VERSION == 0x403
#include <ps5/kernel_offsets/offsets_403.h>
#else
#error SDK does not support this kernel version for kernel hacking.
#endif // PS5_FW_VERSION

// Public hacking API
void kernel_init_rw(int master_sock, int victim_sock, int *rw_pipe, uint64_t pipe_addr);
void kernel_copyin(void *src, uint64_t kdest, size_t length);
void kernel_copyout(uint64_t ksrc, void *dest, size_t length);

#endif // PS5SDK_KERNEL_H