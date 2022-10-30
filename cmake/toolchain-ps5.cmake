###################################################################################################
# PS5SDK - cmake toolchain file
# @author Znullptr
###################################################################################################

set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_SYSTEM_NAME FreeBSD)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Clang target triple
set(TARGET x86_64-freebsd-pc-elf)
set(CLANG_TARGET_TRIPLE ${TARGET})

set(TC_CWD ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_MODULE_PATH "${TC_CWD}/cmake")

if (DEFINED ENV{PS5SDK})
	set(D_PS5SDK $ENV{PS5SDK})
endif()

if (DEFINED ENV{PS5SDK_FW})
	set(V_FW $ENV{PS5SDK_FW})
endif()

if (NOT DEFINED D_PS5SDK)
	cmake_path(GET TC_CWD PARENT_PATH D_PS5SDK)
	message("TOOLCHAIN: setting D_PS5SDK to: ${D_PS5SDK}")
endif()

if (NOT DEFINED V_FW)
	set(V_FW "0x403")
endif()

set(CMAKE_FIND_ROOT_PATH ${D_PS5SDK})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)	# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS "--target=x86_64-freebsd-pc-elf -O0 -DPPR -DPS5 -DPS5_FW_VERSION=${V_FW} ") # -D_KERNEL=1 x86_64-scei-ps5-elf
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_POSIX_SOURCE -D_POSIX_C_SOURCE=200112 -D__BSD_VISIBLE=1 -D__XSI_VISIBLE=500")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-builtin -nostdlib -Wall -m64") #  -nostartfiles
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfsgsbase -maes -mavx2 -mbmi -march=znver2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -fPIC -mcmodel=small -nostdinc -nodefaultlibs")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -nostdinc++ ")

set(CMAKE_ASM_FLAGS "--target=x86_64-freebsd-pc-elf -nostdlib -fPIC")

#-DCMAKE_LINKER=/path/to/linker -DCMAKE_CXX_LINK_EXECUTABLE="<CMAKE_LINKER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld -Xlinker -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.x -Wl,--build-id=none")
