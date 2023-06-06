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


/**
 * Pointer to a syscall instruction.
 **/
static __attribute__ ((used)) long ptr_syscall;


/**
 * Make a syscall.
 **/
asm(".intel_syntax noprefix\n"
    ".global syscall\n"
    ".type syscall @function\n"
    "syscall:\n"
    "  mov rax, rdi\n"                      // sysno
    "  mov rdi, rsi\n"                      // arg1
    "  mov rsi, rdx\n"                      // arg2
    "  mov rdx, rcx\n"                      // arg3
    "  mov r10, r8\n"                       // arg4
    "  mov r8,  r9\n"                       // arg5
    "  mov r9,  qword ptr [rsp + 8]\n"      // arg6
    "  jmp qword ptr [rip + ptr_syscall]\n" // syscall
    "  ret\n"
    );


void __ps5sdk_crt_start(struct payload_args *args)
{
	int rv;

	// Dlsym must be initialized first to resolve everything else
	init_dlsym(args->dlsym);

	// Resolve a pointer to a syscall instruction
	args->dlsym(0x2001, "getpid", &ptr_syscall);
	ptr_syscall += 0xa; // jump directly to syscall instruction

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
