###################################################################################################
# PS5SDK - Example: klog server
# Forks and establishes a klog server that can be connected to over TCP.
# @authors Specter
###################################################################################################

cmake_minimum_required (VERSION 3.16)

set(basename "klog_server")
project(${basename} C CXX ASM)

# Language Standard Defaults
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_CXX_STANDARD}" STREQUAL "")
	set(CMAKE_CXX_STANDARD 23)
endif()

# Check for sub-project as part of main build or external build
if (NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(IS_SUBPROJECT TRUE)
else()
	set(IS_SUBPROJECT FALSE)
endif()

message("IS_SUBPROJECT: ${IS_SUBPROJECT}")

set(D_CWD "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${D_CWD}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${D_CWD}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${D_CWD}/bin)	# static libs are archive

# Headers
include_directories	(SYSTEM "${D_PS5SDK}")
include_directories	(SYSTEM "${D_PS5SDK}/include")

add_executable(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")

# Must build with clang
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "[Cc]lang")
	set(IS_CLANG 1)
else()
	message(FATAL_ERROR "${PROJECT_NAME} is meant to be built with clang! CompilerID: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Finalize main target sources
target_compile_options(${PROJECT_NAME} PUBLIC
	$<$<COMPILE_LANGUAGE:C>:${C_DEFS} ${C_FLAGS}>
	$<$<COMPILE_LANGUAGE:CXX>:${CXX_DEFS} ${CXX_FLAGS}>
	$<$<COMPILE_LANGUAGE:ASM>:${ASM_FLAGS}>
)

message("========== build: ${PROJECT_NAME} ==========")

set(D_SRC ${D_CWD}/source)

file(GLOB SrcFiles ${D_SRC}/*.c ${D_SRC}/*.h ${D_SRC}/*.s)

target_sources(${PROJECT_NAME} PRIVATE ${SrcFiles})
target_link_directories	(${PROJECT_NAME} PUBLIC "${D_PS5SDK}/lib")
target_link_libraries	(${PROJECT_NAME} PUBLIC ps5sdk_crt)
