/*
	[ CKSDK ]
	Copyright 2023 Regan "CKDEV" Green

	Permission to use, copy, modify, and/or distribute this software for any
	purpose with or without fee is hereby granted, provided that the above
	copyright notice and this permission notice appear in all copies.

	THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
	WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
	ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
	WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
	ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
	OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <CKSDK/CKSDK.h>

#include <CKSDK/Mem.h>
#include <CKSDK/TTY.h>
#include <CKSDK/OS.h>
#include <CKSDK/GPU.h>
#include <CKSDK/CD.h>
#include <CKSDK/ISO.h>
#include <CKSDK/SPU.h>
#include <CKSDK/SPI.h>
#include <CKSDK/Timer.h>
#include <CKSDK/DLL.h>

// C externs
extern "C"
{
	extern uint8_t _end[];
}

namespace CKSDK
{
	// CKSDK functions
	KEEP void Init()
	{
		// Initialize systems
		Mem::Init(_end, 0x80200000 - uintptr_t(_end));
		TTY::Init();

		OS::Init();
		GPU::Init();
		#ifndef CKSDK_NO_CD
		CD::Init();
		#endif
		SPU::Init();
		SPI::Init();
		Timer::Init();
		DLL::Init();
		#ifndef CKSDK_NO_CD
		ISO::Init();
		#endif
	}
}
