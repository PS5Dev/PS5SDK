###################################################################################################
# PS5SDK - Root build script
# Builds CRT and examples
# @author Znullptr
###################################################################################################

cmake_minimum_required (VERSION 3.20)

set(basename "example_project")
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

set(D_CWD "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${D_CWD}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${D_CWD}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${D_CWD}/lib)	# static libs are archive

include_directories	(SYSTEM "${D_PS5SDK}")
include_directories	(SYSTEM "${D_PS5SDK}/include")

add_subdirectory("crt")

add_subdirectory("examples/hello_socket")
add_subdirectory("examples/pipe_pirate")

if (DEFINED V_FW AND ${V_FW} STREQUAL "0x403")
	add_subdirectory("examples/klog_server")
endif()

MACRO(SUBDIRLIST result curdir)
	FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
	SET(dirlist "")
	FOREACH(child ${children})
		IF(IS_DIRECTORY ${curdir}/${child})
			LIST(APPEND dirlist ${child})
		ENDIF()
	ENDFOREACH()
	SET(${result} ${dirlist})
ENDMACRO()

SUBDIRLIST(SUBDIRS ${D_CWD}/examples)

foreach(subdir ${SUBDIRS})
	message("subdir: ${subdir}")
endforeach()
