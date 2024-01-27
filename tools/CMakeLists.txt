cmake_minimum_required(VERSION 3.8)

project(CKSDK_Tools)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory("MkExe")

# Dependency interface
add_library(CKSDK_Tools INTERFACE)
add_dependencies(CKSDK_Tools MkExe)
