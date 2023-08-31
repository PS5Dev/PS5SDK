/*****************************************************
 * PS5 SDK - Syscall
 * Implements the syscall() function by setting up
 * registers manually, then jumping to a syscall
 * instruction in libkernel.
 ****************************************************/

#include <ps5/payload_main.h>

static __attribute__ ((used)) long ptr_syscall;

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

int syscall_init(const struct payload_args *args) {
	if(args->dlsym(0x2001, "getpid", &ptr_syscall)) {
		return -1;
	}
	ptr_syscall += 0xa;
	return 0;
}
