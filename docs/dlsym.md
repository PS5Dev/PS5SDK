# DLSym Information

This document contains information related to dlsym and runtime linking with system libraries. To demonstrate how a library is implemented, lets look at libc.

## Header
First, we implement a header file in `/ps5/libc.h`. We need to include `<ps5/dlsym.h>` for dlsym macros.

```c
#ifndef PS5SDK_LIBC_H
#define PS5SDK_LIBC_H

#include <ps5/dlsym.h>
#include <ps5/net.h>
```

Secondly, we include any headers that contain detailed type information that may be used by applications. These are not needed for the CRT library, so we only include them if `CRT_BUILD` is not defined.

```c
#ifndef CRT_BUILD
#include <stdio.h>
#endif
```

Every library needs an `_init()` routine for the CRT to call into before the entrypoint is invoked.

```
int libc_init();
```

### Functions
Now, we add declarations for functions from the library. The `_Fn_` macro is provided for this, and must be used. It'll handle making the stubs to link with while also not clashing with the BSD headers. `_Fn_` consists of a `type`, `name`, and `args...`.

For example, `memcmp` will have a declaration like this:

```c
_Fn_(int    	, memcmp,		const void *, const void *, size_t);
```

### Data
For data, the `_Data_` macro is provided. It consists of a `type` followed by the `name`. For example, `optarg`:

```c
_Data_(char*, optarg);
```

## Library source file
The source file goes in `/crt`, again for libc this is `/crt/libc.c`. Since this is for the CRT, we must define `CRT_BUILD` and include the accompanying header file. `CRT_BUILD` signifies that standard header includes should be ignored. We'll also include `

```c
#define CRT_BUILD
#include <ps5/libc.h>
```

Next, we need to declare an external for `sceKernelLoadStartModule()`. This is a dependency from libkernel that will allow us to load the module for use, and return a handle to us.

```c
extern int sceKernelLoadStartModule(const char *name, uint64_t argc, const void *argv, uint32_t flags, void *, int *result);
```

Next, we'll implement the `_init()` routine, which is responsible for loading the module and resolving functions and data from it. We'll also double check that dlsym has been initialized so we don't end up calling null pointers.

```c
int libc_init()
{
    int libc = sceKernelLoadStartModule("libSceLibcInternal.sprx", 0, 0, 0, 0, 0);
    if (__dlsym_init != 1) {
        return -1;
    }
    
    // ...
    return 0;
}
```

From here, it's a matter of using the `DYNLIB_RESOLVE` and `DLD_RESOLVE` macros to resolve functions and data respectively.

`DYNLIB_RESOLVE` consists of the `handle` variable followed by the `name` of the function.
`DLD_RESOLVE` consists of the `handle`, `name`, and `type`.

### Functions
Sticking with `memcmp()`, to resolve it, we'd do:
```c
DYNLIB_RESOLVE(libc, memcmp);
```

### Data
To resolve `optarg`, we'd do:
```c
DLD_RESOLVE(libc, optarg, char *);
```

## Adding to the CRT
We now have a header and source file to implement the library, but we need to ensure the `_init()` routine is called before any applications/payloads can use anything from it. The `__ps5sdk_crt_start()` function in [/crt/crt.c](/crt/crt.c) is responsible for this. It's called before the payload entrypoint.

We must check the return value of the `_init()` call before calling `payload_main()`. We use tagged return values (mainly `ERR_CRT_MODULE_INIT_FAIL`) for fails. The return value is written to `args->payloadout` provided by WebKit, and is printed to WebKit.

The first module to be initialized should always be `libkernel`.

```c
void __ps5sdk_crt_start(struct payload_args *args)
{
	// ...
	if (libc_init() != 0) {
        rv = ERR_CRT_MODULE_INIT_FAIL;
        goto out;
    }
    // ...
}
```

## Special cases
In some cases (especially libkernel) declarations with `_Fn_` will directly conflict with BSD headers. In these cases, they're put in the library source file instead of the header. This is because it's not needed since a declaration is provided in standard headers, it's only required for linking, which is handled by the source.

Example: `socket()`. Instead of being implemented in the header, the declaration was moved to the libc.c source file.