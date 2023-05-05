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

#include <CKSDK/SPU.h>

#include <CKSDK/TTY.h>

namespace CKSDK
{
	namespace SPU
	{
		// SPU constants
		static constexpr unsigned STATUS_TIMEOUT = 0x100000;

		// Internal SPU functions
		void StatusSync(uint16_t mask, uint16_t value)
		{
			for (unsigned i = STATUS_TIMEOUT; i != 0; i--)
				if ((OS::SpuStat() & mask) == value)
					return;
			TTY::Out("SPU status sync timeout");
		}

		// SPU functions
		void Init()
		{
			// Reset SPU
			OS::SpuDelaySize() = 0x200931E1;

			OS::SpuCtrl() = 0x0000;
			StatusSync(0x001F, 0x0000);

			// Reset SPU state
			OS::SpuMasterVolL() = 0x3FFF;
			OS::SpuMasterVolR() = 0x3FFF;

			OS::SpuReverbVolL() = 0;
			OS::SpuReverbVolR() = 0;

			OS::SpuKeyOff() = 0xFFFFFF;

			OS::SpuFmMode() = 0;
			
			OS::SpuNoiseMode() = 0;

			OS::SpuReverbOn() = 0;
			OS::SpuReverbAddr() = 0xFFFE;

			OS::SpuCdVolL() = 0x7FFF;
			OS::SpuCdVolR() = 0x7FFF;
			
			OS::SpuExtVolL() = 0;
			OS::SpuExtVolR() = 0;

			OS::DmaDpcr() = OS::DpcrSet(OS::DmaDpcr(), OS::DMA::SPU, 3); // Enable DMA4
			OS::DmaCtrl(OS::DMA::SPU).chcr = 0x00000201; // Stop DMA4

			OS::SpuDmaCtrl() = 0x0004; // Reset transfer mode
			OS::SpuCtrl() = 0xC001; // Enable SPU, DAC, CD audio, disable DMA request
			StatusSync(0x003F, 0x0001);
		}
	}
}
