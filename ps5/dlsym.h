/*****************************************************
 * PS5 SDK - Dlsym header
 * Contains macros and types for internal dlsym stuff.
 ****************************************************/

#ifndef PS5SDK_DLSYM_H
#define PS5SDK_DLSYM_H

#include "./stdint.h"

#ifndef CRT_BUILD
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <float.h>
#include <math.h>
#include <time.h>

#include <sys/types.h>
#include <sys/select.h>

#include <locale.h>
#else
#define _CrtLinkage_ 1
#endif // CRT_BUILD

// Macros for library implementation
#define DYNLIB_RESOLVE(module, name) fptr_##name = (void *)dynlib_resolve_symbol(module, #name)
#define DLD_RESOLVE(module, name, type) name = (type *)dynlib_resolve_symbol(module, #name)

// Types for internal use
typedef int dlsym_t(int, const char*, void*);

extern dlsym_t *__dlsym;
extern int __dlsym_init;

// Init and dynlib_resolve_symbol routines
void init_dlsym(dlsym_t *f_dlsym);
uint64_t dynlib_resolve_symbol(int module, const char *symbol);

// Hacky garbage to write stubs for linking functions. Maybe we can do better than this...
#if _CrtLinkage_
//uint64_t (* b) = DLSYM(b)
#define _Fn_(a,b,...) uint64_t (* fptr_##b)(); \
	asm(                                       \
		".intel_syntax noprefix\n" \
		".global " #b "\n" #b ":\n" \
		"jmp qword ptr [rip + fptr_" #b "]\n")
#else
#define _Fn_(a,b,...) a b(__VA_ARGS__)
#endif // _CrtLinkage_

// Linking for data
#ifndef _CrtLinkage_
#define _Data_(Type, X) extern Type X
#else
#define _Data_(Type, X) Type* X
#endif // _CrtLinkage_

#endif // PS5SDK_DLSYM_H