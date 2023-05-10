# This script is included automatically when using the toolchain file and
# defines helper functions.

cmake_minimum_required(VERSION 3.8)

include(GNUInstallDirs)
set(CKSDK_DIR "${CMAKE_CURRENT_LIST_DIR}/..")

# Find tools
find_program(MKEXE MkExe HINTS "${CKSDK_TOOLS_DIR}")

## CMake configuration

# Setting these variables and properties would technically be the toolchain
# script's responsibility, however they are overridden by project() so their
# setting is deferred to this script.
set(CMAKE_EXECUTABLE_SUFFIX     ".elf")
set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
set(CMAKE_SHARED_MODULE_PREFIX  "")
set(CMAKE_SHARED_MODULE_SUFFIX  ".so")

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS ON)

## Settings (can be overridden by projects)

set(CKSDK_EXECUTABLE_SUFFIX     ".exe")
set(CKSDK_SHARED_LIBRARY_SUFFIX ".dll")
set(CKSDK_SYMBOL_MAP_SUFFIX     ".map")

define_property(
	TARGET PROPERTY CKSDK_TARGET_TYPE
	BRIEF_DOCS      "Type of this target (EXECUTABLE_GPREL, EXECUTABLE_NOGPREL or SHARED_LIBRARY)"
	FULL_DOCS       "Type of this target (if executable or DLL) or of the executable/DLL this target is going to be linked to (if static library)"
)

## Include paths
set(CKSDK_LDSCRIPTS "${CKSDK_DIR}/ld")

# CKSDK defs
add_library(CKSDK_defs INTERFACE)

target_compile_options(
	CKSDK_defs INTERFACE
		# Options common to all target types
		-g
		-Wa,--strip-local-absolute
		-O3
		-ffreestanding
		-fno-builtin
		-nostdlib
		-fdata-sections
		-ffunction-sections
		-fsigned-char
		-fno-strict-overflow
		-fdiagnostics-color=always
		-msoft-float
		-march=r3000
		-mtune=r3000
		-mabi=32
		-mno-mt
		-mno-llsc
		-mdivide-breaks
		-fomit-frame-pointer
	$<$<COMPILE_LANGUAGE:CXX>:
		# Options common to all target types (C++)
		-fno-exceptions
		-fno-rtti
		-fno-unwind-tables
		-fno-threadsafe-statics
		-fno-use-cxa-atexit
		-fcoroutines
		# Use C++20
		-std=c++20
	>
	$<$<COMPILE_LANGUAGE:C>:
		# Use C17
		-std=c17
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:CKSDK_TARGET_TYPE>>,EXECUTABLE_GPREL>:
		# Options for executables with $gp-relative addressing
		-G8
		-fno-pic
		-mno-abicalls
		-mgpopt
		-mno-extern-sdata
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:CKSDK_TARGET_TYPE>>,EXECUTABLE_NOGPREL>:
		# Options for executables without $gp-relative addressing
		-G0
		-fno-pic
		-mno-abicalls
		-mno-gpopt
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:CKSDK_TARGET_TYPE>>,SHARED_LIBRARY>:
		# Options for DLLs
		-G0
		-fPIC
		-mabicalls
		-mno-gpopt
		-mshared
	>
)

target_link_options(
	CKSDK_defs INTERFACE
		# Options common to all target types
		-nostdlib
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:CKSDK_TARGET_TYPE>>,EXECUTABLE_GPREL>:
		# Options for executables with $gp-relative addressing
		-G8
		-static
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:CKSDK_TARGET_TYPE>>,EXECUTABLE_NOGPREL>:
		# Options for executables without $gp-relative addressing
		-G0
		-static
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:CKSDK_TARGET_TYPE>>,SHARED_LIBRARY>:
		# Options for DLLs
		-G0
		-shared
	>
)

target_compile_definitions(
	CKSDK_defs INTERFACE
		MIPSEL=1
		$<$<CONFIG:Release>:NDEBUG=1>
)

target_link_libraries(CKSDK_defs INTERFACE -lgcc)

target_include_directories(CKSDK_defs INTERFACE "${CKSDK_DIR}/include")

# CKSDK library
function(cksdk_executable name target_exe target_map)
	# Get source directories
	set(SRC_DIR "${CKSDK_DIR}/src")
	set(INC_DIR "${CKSDK_DIR}/include/CKSDK")
	
	# Create executable
	add_executable(${name} ${ARGN}
		# CKSDK
		"${SRC_DIR}/CKSDK.cpp"

		"${INC_DIR}/CKSDK.h"

		# OS
		"${SRC_DIR}/OS/CxxCRT.cpp"
		"${SRC_DIR}/OS/CxxCRT.s"
		"${SRC_DIR}/OS/OS.cpp"
		"${SRC_DIR}/OS/OS.s"
		"${SRC_DIR}/OS/ExScreen.cpp"
		"${SRC_DIR}/OS/ExScreen_Font.h"
		"${SRC_DIR}/OS/TTY.cpp"
		"${SRC_DIR}/OS/Mem.cpp"
		"${SRC_DIR}/OS/Timer.cpp"

		"${INC_DIR}/OS.h"
		"${INC_DIR}/ExScreen.h"
		"${INC_DIR}/STL.h"
		"${INC_DIR}/TTY.h"
		"${INC_DIR}/Mem.h"
		"${INC_DIR}/Timer.h"

		# CD
		"${SRC_DIR}/CD/CD.cpp"
		"${SRC_DIR}/CD/ISO.cpp"

		"${INC_DIR}/CD.h"
		"${INC_DIR}/ISO.h"

		# GPU
		"${SRC_DIR}/GPU/GPU.cpp"

		"${INC_DIR}/GPU.h"
		"${INC_DIR}/GTE.h"

		# SPU
		"${SRC_DIR}/SPU/SPU.cpp"

		"${INC_DIR}/SPU.h"

		# SPI
		"${SRC_DIR}/SPI/SPI.cpp"

		"${INC_DIR}/SPI.h"

		# DLL
		"${SRC_DIR}/DLL/DLL.cpp"

		"${INC_DIR}/DLL.h"
		"${INC_DIR}/ELF.h"

		# Util
		"${INC_DIR}/Util/Hash.h"
		"${INC_DIR}/Util/Fixed.h"
		"${INC_DIR}/Util/Queue.h"
	)
	
	target_include_directories(${name} PRIVATE "${CKSDK_DIR}/src")

	# Compile as mipsel
	set_target_properties(${name} PROPERTIES CKSDK_TARGET_TYPE EXECUTABLE_NOGPREL)
	target_link_options(${name} PRIVATE -T$<SHELL_PATH:${CKSDK_LDSCRIPTS}/exe.ld>)
	target_link_libraries(${name} PUBLIC CKSDK_defs)
	
	# Output .exe and .map
	add_custom_command(
		TARGET ${name} POST_BUILD
		COMMAND
			${MKEXE}
			$<SHELL_PATH:$<TARGET_FILE:${name}>>
			$<SHELL_PATH:${TARGET_EXE}>
		COMMAND
			${TOOLCHAIN_NM} -f posix -l -n
			$<SHELL_PATH:$<TARGET_FILE:${name}>>
			$<ANGLE-R>$<SHELL_PATH:${target_map}>
		BYPRODUCTS
			${target_exe}
			${target_map}
		DEPENDS
			${name}
			${MKEXE}
	)
endfunction()

function(cksdk_dll name target_dll)
	# Compile library
	add_library(${name} SHARED ${ARGN})
	set_target_properties(${name} PROPERTIES CKSDK_TARGET_TYPE SHARED_LIBRARY)
	target_link_libraries(${name} PRIVATE CKSDK_defs)
	target_link_options(${name} PRIVATE -T$<SHELL_PATH:${CKSDK_LDSCRIPTS}/dll.ld>)

	# Output .dll
	add_custom_command(
		TARGET ${name} POST_BUILD
		COMMAND
			${CMAKE_OBJCOPY} -O binary
			$<SHELL_PATH:$<TARGET_FILE:${name}>>
			$<SHELL_PATH:${target_dll}>
		BYPRODUCTS ${target_dll}
	)
endfunction()
