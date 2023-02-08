cmake_minimum_required(VERSION 3.8)

# MkExe
project(MkExe LANGUAGES CXX)
add_executable(MkExe
	"MkExe/MkExe.cpp"
	"MkExe/elf.h"
)

# Dependency interface
project(CKSDK_Tools)
add_library(CKSDK_Tools INTERFACE)
add_dependencies(CKSDK_Tools MkExe)
