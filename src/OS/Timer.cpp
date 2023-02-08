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

#include <CKSDK/Timer.h>

#include <CKSDK/OS.h>

#include <CKSDK/TTY.h>

namespace CKSDK
{
	namespace Timer
	{
		// Timer functions
		void Init()
		{
			
		}

		void Set(uint32_t rate, TimerCallback cb)
		{
			rate = (OS::F_CPU / 8) / rate;
			if (rate >= 0x10000)
				rate = 0xFFFF;
			
			TIMER_CTRL(2).ctrl = 0x0258;
			TIMER_CTRL(2).reload = rate;
			OS::SetIRQ(OS::IRQ::TIMER2, cb);
		}
	}
}
