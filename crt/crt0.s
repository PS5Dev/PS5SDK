#
#   crt0.s, intel format 
#
.intel_syntax noprefix
.text

.global _start
_start:
	jmp		__ps5sdk_crt_start
