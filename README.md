# PS5 SDK

[![Release State](https://img.shields.io/badge/release%20state-beta-yellow.svg)](https://github.com/PS5Dev/PS5SDK) [![Platforms](https://img.shields.io/badge/platform-linux%20%7C%20windows%20%7C%20macos-lightgrey)](https://github.com/PS5Dev/PS5SDK)
[![CMake](../../actions/workflows/cmake.yml/badge.svg)](../../actions/workflows/cmake.yml)

**Note: As this SDK is a work-in-progress, it's subject to major changes between releases until it approaches a stable version (1.0).**

This repository contains source code and documentation for a work-in-progress Software Development Kit (SDK) for the PS5. It contains the headers, libraries, and helpers to build ELF files for the PS5. Currently, it's intended mostly for payloads to load into the WebKit-based ELF loader, though it can also be used for simple homebrew. It *cannot* build full proper applications at the moment, as we do not have full homebrew support.

Header files as well as the C Runtime (CRT) is a WIP and will require updating; feel free to fork and submit pull requests to update support. This is especially true for Sony-specific library functions.

C++ standard library (STL) is not supported, though bare-metal C++ should work.



## Entrypoint

For applications/payloads, `payload_main()` is the entry routine. It takes a `struct payload_args` pointer for an argument. These arguments are provided by WebKit when loading the ELF. They contain information necessary for dlsym as well as kernel hacking if needed. This struct is provided in the `ps5/payload_main.h` header, and should be included in every project.

```c
struct payload_args
{
    dlsym_t* dlsym;
    int *rwpipe;
    int *rwpair;
    uint64_t kpipe_addr;
    uint64_t kdata_base_addr;
    int *payloadout;
};
```

`dlsym` and `payloadout` should be ignored as they're used by the CRT internally. `rwpipe`, `rwpair`, and `kpipe_addr` should be passed through to kernel helper `kernel_init_rw()` for internal use. `kdata_base_addr` is provided for convenience for kernel hacking. To see how these args should be used and how kernel hacking works, see the [pipe_pirate](./examples/pipe_pirate) example.



## Kernel hacking support

Some examples (such as [pipe_pirate](./examples/pipe_pirate)) are firmware-dependent as they have offsets that are specific to that FW. For these types of projects, ensure the `PS5SDK_FW` environment variable is set. For example, if you're targeting 4.03, `PS5SDK_FW` should be set to `0x403`. Below are currently supported firmwares for kernel hacking:

- 3.00 (`0x300`)
- 3.20 (`0x320`)
- 3.21 (`0x321`)
- 4.02 (`0x402`)
- 4.03 (`0x403`)
- 4.50 (`0x450`)
- 4.51 (`0x451`)

Offsets that are common such as ones used by examples are in [/ps5/kernel_offsets](./ps5/kernel_offsets).

There is a `kernel_helper` header and CRT support for arbitrary read/write. For more information, see [docs/kernel_hacking.md](docs/kernel_hacking.md)



## Dependencies

- CMake (version >= 3.20)
- Ninja
- Clang/lld



## Build system

This SDK utilizes cmake and ninja for it's build system. The toolchain file can be found at [cmake/toolchain-ps5.cmake](./cmake/toolchain-ps5.cmake). If you wish to build a project outside the source tree, ensure to set the `PS5SDK` environment variable to the root path of this repository.

The root [CMakeLists.txt](./CMakeLists.txt) builds the CRT in `/crt` and `/examples` projects. Each example project has it's own build files, which can be referenced or copied to use in your own projects.



## Build instructions

First, clone or extract this SDK to a directory, and set the `PS5SDK` environment variable to point to it. These instructions can be used for the SDK root as well as any projects that use the example CMakeLists. The SDK library must be built first if you're not using a release version.

### CLI
*Note: The build.sh script contains the below commands to run easily, assuming the `PS5SDK` env var is set.*

1. Configure cmake.
```
$ cmake -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_TOOLCHAIN_FILE=$PS5SDK/cmake/toolchain-ps5.cmake .
```
2. Build with `ninja`
```
$ ninja
```

### Clion IDE
1. Go to File -> Settings -> Build, Execution, Deployment -> CMake
2. Set "CMake options":
```
-G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_TOOLCHAIN_FILE=[PS5 SDK root]/cmake/toolchain-ps5.cmake .
```
3. Set "Environment" to environment variables
```
PS5SDK=[PS5 SDK root]
PS5SDK_FW=[target FW]
```

### VSCode
1. Ensure ninja is available on your system and can be found in your PATH.
2. You may need to update `CMakePresets.json`'s `toolchainFile` path to `[PS5 SDK root]/cmake/toolchain-ps5.cmake`, particularly if building outside the source tree.
3. Once you open the project, it should prompt to configure cmake. Alternatively, go to the "CMake" tab and "Configure All Projects" + "Build All Projects"



## Dlsym

Dynamic library symbols are resolved at runtime via `dlsym()` from libkernel. This is handled by the CRT before the payload entrypoint is ever called. Libraries that are supported in the SDK will have a source file in [/crt](./crt) as well as an accompanying header file in [/ps5](./ps5). Over time, we'll add resolving and declarations for more functions.

To start with, most standard library functions from libkernel/libc have been added. There are a few Sony functions which have been added thanks to reversing from the PS4 efforts.

See [docs/dlsym.md](./docs/dlsym.md) for more information on how to add support for functions or libraries.



## License

This project is licensed under the GPLv2 license - see the [LICENSE](./LICENSE) file for details.



## Maintainers + Special  Thanks

- Specter (lead maintainer)
- Znullptr (lead maintainer)
- ChendoChap (lead maintainer)
