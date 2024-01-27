cmake_minimum_required(VERSION 3.8)

## CMake configuration

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR mipsel)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Tell CMake not to run the linker when compiling test programs, and to pass
# toolchain settings to the generated test projects. This dodges missing C++
# standard library errors.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES CKSDK_TC CKSDK_TARGET)

# Always generate compile_commands.json alongside build scripts. This allows
# some IDEs and tools (such as clangd) to automatically configure include
# directories and other options.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## Toolchain path setup

include(${CMAKE_CURRENT_LIST_DIR}/FindToolchain.cmake)

set(CMAKE_ASM_COMPILER ${FOUND_CMAKE_ASM_COMPILER})
set(CMAKE_C_COMPILER   ${FOUND_CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${FOUND_CMAKE_CXX_COMPILER})
set(CMAKE_AR           ${FOUND_CMAKE_AR})
set(CMAKE_LINKER       ${FOUND_CMAKE_LINKER})
set(CMAKE_RANLIB       ${FOUND_CMAKE_RANLIB})
set(CMAKE_OBJCOPY      ${FOUND_CMAKE_OBJCOPY})
set(CMAKE_SIZE         ${FOUND_CMAKE_SIZE})
set(CMAKE_STRIP        ${FOUND_CMAKE_STRIP})
set(TOOLCHAIN_NM       ${FOUND_TOOLCHAIN_NM})

## SDK setup

# Continue initialization by running CKSDK.cmake after project() is
# invoked.
set(CMAKE_PROJECT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/CKSDK.cmake)
