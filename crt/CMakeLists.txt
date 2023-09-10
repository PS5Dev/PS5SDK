###################################################################################################
# PS5SDK - CRT build script
# @author Znullptr
###################################################################################################

cmake_minimum_required (VERSION 3.16)

set(basename "ps5sdk_crt")
project(${basename} C CXX ASM)

# Language Standard Defaults
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_CXX_STANDARD}" STREQUAL "")
	set(CMAKE_CXX_STANDARD 23)
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${D_CWD}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${D_CWD}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${D_CWD}/lib)	# static libs are archive

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "[Cc]lang")
	set(IS_CLANG 1)
else()
	message(FATAL_ERROR "${PROJECT_NAME} is meant to be built with clang! CompilerID: ${CMAKE_CXX_COMPILER_ID}")
endif()

if (NOT DEFINED D_PS5SDK)
	set(D_PS5SDK ".")
endif()

message("  C_FLAGS: ${CMAKE_C_FLAGS}  ")
message("CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message("ASM_FLAGS: ${CMAKE_ASM_FLAGS}")

########## finalize main target sources ##########

set(D_CRT_SRC ${D_CWD}/crt)

file(GLOB SrcFiles ${D_CRT_SRC}/*.c ${D_CRT_SRC}/*.s)

add_library(${PROJECT_NAME} STATIC ${SrcFiles})

target_compile_options(${PROJECT_NAME} PUBLIC
    $<$<COMPILE_LANGUAGE:C>:${C_DEFS} ${C_FLAGS}>
    $<$<COMPILE_LANGUAGE:CXX>:${CXX_DEFS} ${CXX_FLAGS}>
    $<$<COMPILE_LANGUAGE:ASM>:$<IF:${IS_CLANG},-Xclang,-x> assembler-with-cpp ${ASM_FLAGS}>
)
