# Kernel Hacking

**Note: Kernel hacking is for advanced users who are be comfortable in kernel environment. Incorrect use of this functionality will crash and potentially corrupt the system. Special care should be taken.**

A set of helper routines is provided for kernel hacking (read/write). To utilize it, include the `ps5/kernel.h` header and initialize it via `kernel_init_rw()`. Also be sure the `PS5SDK_FW` environment variable is set correctly. See main [README.md](/README.md) for information on how to set this.

```c
// ... [other includes]
#include <ps5/kernel.h>

int payload_main(struct payload_args *args)
{
    kernel_init_rw(args->rwpair[0], args->rwpair[1], args->rwpipe, args->kpipe_addr);
    // ...
    return 0;
}
```



## Read (Peek)

To read kernel memory, use `kernel_copyout()`.

```c
void kernel_copyout(uint64_t ksrc, void *dest, size_t length)
```

Example:

```c
uint64_t allproc_addr = kdata_base + OFFSET_KERNEL_DATA_BASE_ALLPROC;
uint64_t cur_proc_addr = 0;

kernel_copyout(allproc_addr, &cur_proc_addr, sizeof(cur_proc_addr));
```



## Write (Poke)

Writing is similar with the `kernel_copyin()` function.

```c
void kernel_copyin(void *src, uint64_t kdest, size_t length)
```

Example:

```c
uint64_t cur_proc_ucred_addr = ...;
uint32_t tagged_uid = 0x1337;

kernel_copyin(&tagged_uid, cur_proc_ucred_addr + OFFSET_KERNEL_UCRED_CR_UID, sizeof(tagged_uid));
```

