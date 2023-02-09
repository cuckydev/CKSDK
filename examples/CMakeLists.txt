cmake_minimum_required(VERSION 3.8)

include(ExternalProject)

# Project
project(Example LANGUAGES C CXX ASM)

# CKSDK setup
set(CKSDK_DIR "${CMAKE_SOURCE_DIR}/../../CKSDK")

set(CKSDK_TOOLS_DIR "${CMAKE_BINARY_DIR}/CKSDK_Tools")
add_subdirectory("${CKSDK_DIR}/tools" ${CKSDK_TOOLS_DIR})

set(CKSDK_ARGS
	-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}

	-DCMAKE_TOOLCHAIN_FILE:FILEPATH="${CKSDK_DIR}/cmake/Toolchain.cmake"
	-DCKSDK_TOOLS_DIR:FILEPATH=${CKSDK_TOOLS_DIR}
)

# Compile Poly example
ExternalProject_Add(Poly
	SOURCE_DIR       "${PROJECT_SOURCE_DIR}/Poly"
	BINARY_DIR       Poly-build
	CMAKE_CACHE_ARGS ${CKSDK_ARGS}
	CMAKE_ARGS       ${CKSDK_ARGS}
	INSTALL_COMMAND  ""
	BUILD_ALWAYS     1
	DEPENDS          CKSDK_Tools
)
