/*****************************************************
 * PS5 SDK - Payload Main
 * Contains struct for arguments to entrypoint. This
 * should be included by every project!
 ****************************************************/

#ifndef PS5SDK_PAYLOAD_MAIN_H
#define PS5SDK_PAYLOAD_MAIN_H

#include <ps5/dlsym.h>

struct payload_args
{
	dlsym_t* dlsym;             // 0x00
	int *rwpipe;                // 0x08
	int *rwpair;                // 0x10
	uint64_t kpipe_addr;        // 0x18
	uint64_t kdata_base_addr;   // 0x20
	int *payloadout;            // 0x28
};

#endif