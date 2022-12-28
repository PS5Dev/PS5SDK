/*****************************************************
 * PS5 SDK - CRT
 * Implements the CRT entrypoint to perform resolving.
 ****************************************************/

#define CRT_BUILD
#include <ps5/payload_main.h>
#include <ps5/dlsym.h>

// Tagged error values
#define ERR_CRT_UNKNOWN             0xDEAD0000
#define ERR_CRT_LIBKERNEL_INIT_FAIL 0xDEAD0001
#define ERR_CRT_MODULE_INIT_FAIL    0xDEAD0002

extern int payload_main(struct payload_args *args);
extern int libc_init();
extern int libkernel_init();

void __ps5sdk_crt_start(struct payload_args *args)
{
	int rv = ERR_CRT_UNKNOWN;

	// Dlsym must be initialized first to resolve everything else
	init_dlsym(args->dlsym);

	// Kickstart libkernel and libc
	if (libkernel_init() != 0) {
		rv = ERR_CRT_LIBKERNEL_INIT_FAIL;
		goto out;
	}

	if (libc_init() != 0) {
		rv = ERR_CRT_MODULE_INIT_FAIL;
		goto out;
	}

	// Any other libraries to be initialized go here...

	// Call into main
	rv = payload_main(args);

out:
	*(int *)args->payloadout = rv;
}