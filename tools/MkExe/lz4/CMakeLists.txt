# Compile lz4
add_library(lz4 STATIC
	"Source/lz4.c"
	"Source/lz4file.c"
	"Source/lz4frame.c"
	"Source/lz4hc.c"
	"Source/xxhash.c"
	"Include/lz4.h"
	"Include/lz4file.h"
	"Include/lz4frame.h"
	"Include/lz4frame_static.h"
	"Include/lz4hc.h"
	"Include/xxhash.h"
)
target_include_directories(lz4 PUBLIC "Include")
