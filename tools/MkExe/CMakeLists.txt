# Find CKSDK's toolchain
include(${CKSDK_DIR}/cmake/FindToolchain.cmake)

# Compile bootstrap binary
set(ASM_CFLAGS
	-march=r3000 -mtune=r3000 -mabi=32
)

add_custom_command(OUTPUT "Bootstrap.o"
	COMMAND ${FOUND_CMAKE_ASM_COMPILER} -c "${CMAKE_CURRENT_SOURCE_DIR}/Bootstrap.s" -o "Bootstrap.o" ${ASM_CFLAGS}
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Bootstrap.s"
)
add_custom_command(OUTPUT "Bootstrap.bin"
	COMMAND ${FOUND_CMAKE_OBJCOPY} -O binary -j .text "Bootstrap.o" "Bootstrap.bin"
	DEPENDS "Bootstrap.o"
)

add_executable(MkExe_Bin2h
	"Bin2h.cpp"
)
add_custom_command(OUTPUT "Include/Bootstrap.h"
	COMMAND MkExe_Bin2h "Bootstrap.bin" "Include/Bootstrap.h"
	DEPENDS "Bootstrap.bin"
)

add_custom_target(Ohmygodcompileit ALL DEPENDS "Include/Bootstrap.h")

# Compile tool
add_subdirectory("lz4")

add_executable(MkExe
	"MkExe.cpp"
	"elf.h"
)
target_link_libraries(MkExe PUBLIC lz4)
target_include_directories(MkExe PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/Include")
add_dependencies(MkExe Ohmygodcompileit)
