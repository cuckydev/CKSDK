## Find toolchain paths

set(
	CKSDK_TC  ""
	CACHE PATH "Path to the GCC toolchain's installation directory (if not in PATH)"
)
set(
	CKSDK_TARGET mipsel-none-elf
	CACHE STRING  "GCC toolchain target triplet"
)

# Attempt to find GCC using a list of common installation locations.
# CKSDK_TC can be left unset if the toolchain can be found in any of these
# or in the PATH environment variable.
find_program(
	_gcc ${CKSDK_TARGET}-gcc
	HINTS
		${CKSDK_TC}/bin
		${CKSDK_TC}/../bin
		# Same as ${CMAKE_INSTALL_PREFIX}/bin
		${CMAKE_CURRENT_LIST_DIR}/../../../bin
		# Same as ${CMAKE_INSTALL_PREFIX}/${CKSDK_TARGET}/bin
		${CMAKE_CURRENT_LIST_DIR}/../../../${CKSDK_TARGET}/bin
	PATHS
		"C:/Program Files/${CKSDK_TARGET}/bin"
		"C:/Program Files (x86)/${CKSDK_TARGET}/bin"
		"C:/${CKSDK_TARGET}/bin"
		/opt/${CKSDK_TARGET}/bin
		/usr/local/${CKSDK_TARGET}/bin
		/usr/${CKSDK_TARGET}/bin
	NO_CACHE REQUIRED
)
cmake_path(GET _gcc PARENT_PATH _bin)
cmake_path(GET _bin PARENT_PATH _toolchain)

# Overwrite the empty cache entry, so it won't have to be found again.
if(NOT IS_DIRECTORY CKSDK_TC)
	set(
		CKSDK_TC ${_toolchain}
		CACHE PATH   "Path to the GCC toolchain's installation directory (if not in PATH)"
		FORCE
	)
endif()

## Toolchain executables

# As we have overridden ${CMAKE_EXECUTABLE_SUFFIX} we can't rely on it to
# determine the host OS extension for executables. A workaround is to extract
# the extension from the path returned by find_program() using a regex.
set(_prefix ${_bin}/${CKSDK_TARGET})
string(REGEX MATCH ".+-gcc(.*)$" _dummy ${_gcc})

set(FOUND_CMAKE_ASM_COMPILER ${_prefix}-gcc${CMAKE_MATCH_1})
set(FOUND_CMAKE_C_COMPILER   ${_prefix}-gcc${CMAKE_MATCH_1})
set(FOUND_CMAKE_CXX_COMPILER ${_prefix}-g++${CMAKE_MATCH_1})
set(FOUND_CMAKE_AR           ${_prefix}-ar${CMAKE_MATCH_1})
set(FOUND_CMAKE_LINKER       ${_prefix}-ld${CMAKE_MATCH_1})
set(FOUND_CMAKE_RANLIB       ${_prefix}-ranlib${CMAKE_MATCH_1})
set(FOUND_CMAKE_OBJCOPY      ${_prefix}-objcopy${CMAKE_MATCH_1})
set(FOUND_CMAKE_SIZE         ${_prefix}-size${CMAKE_MATCH_1})
set(FOUND_CMAKE_STRIP        ${_prefix}-strip${CMAKE_MATCH_1})
set(FOUND_TOOLCHAIN_NM       ${_prefix}-nm${CMAKE_MATCH_1})
