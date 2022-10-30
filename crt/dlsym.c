/*****************************************************
 * PS5 SDK - Dlsym
 * Contains initialization routine for dlsym as well as
 * underlying `dynlib_resolve_symbol` function.
 ****************************************************/

#define CRT_BUILD
#include "ps5/dlsym.h"

// Internal variables for tracking
dlsym_t *__dlsym;
int __dlsym_init = 0;

// Called once by CRT entrypoint
void init_dlsym(dlsym_t *f_dlsym)
{
	__dlsym = f_dlsym;
	__dlsym_init = 1;
}

// Called by macros
uint64_t dynlib_resolve_symbol(int module, const char *symbol)
{
	uint64_t addr;
	__dlsym(module, symbol, &addr);
	return addr;
}
