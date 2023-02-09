cmake_minimum_required(VERSION 3.8)

# Project and executable
project(Poly LANGUAGES C CXX ASM)

set(TARGET_EXE "${CMAKE_CURRENT_BINARY_DIR}/Poly${CKSDK_EXECUTABLE_SUFFIX}")
set(TARGET_MAP "${CMAKE_CURRENT_BINARY_DIR}/Poly${CKSDK_SYMBOL_MAP_SUFFIX}")
cksdk_executable(Poly ${TARGET_EXE} ${TARGET_MAP}
	"Poly.cpp"
)

target_compile_definitions(Poly PRIVATE CKSDK_NO_CD)
