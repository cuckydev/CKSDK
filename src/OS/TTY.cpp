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

#include <CKSDK/TTY.h>

#include <stdarg.h>

#include <CKSDK/OS.h>

namespace CKSDK
{
	namespace TTY
	{
		// TTY functions
		KEEP void Init()
		{
			// Initialize TTY on EXP2
			OS::Exp2DelaySize() = 0x70777;
		}

		KEEP void Out(const char *str)
		{
			// Check if TTY is idle
			if (OS::DuartSra() != 0x0C)
				return;
			
			while (1)
			{
				// Get character
				char c = *str++;
				if (c == '\0')
					break;
				
				// Out to TTY
				while (!(OS::DuartSra() & (1 << 2)));
				OS::DuartHra() = (uint8_t)c;
			}

			// Wait for TTY to return to idle
			while (OS::DuartSra() != 0x0C);
		}
	}
}
