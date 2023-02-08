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
				if ((SPU_STAT & mask) == value)
					return;
			TTY::Out("SPU status sync timeout");
		}

		// SPU functions
		void Init()
		{
			// Reset SPU
			SPU_DELAY_SIZE = 0x200931E1;

			SPU_CTRL = 0x0000;
			StatusSync(0x001F, 0x0000);

			// Reset SPU state
			SPU_MASTER_VOL_L	= 0x3FFF;
			SPU_MASTER_VOL_R	= 0x3FFF;

			SPU_REVERB_VOL_L	= 0;
			SPU_REVERB_VOL_R	= 0;

			SPU_KEY_OFF1		= 0xFFFF;
			SPU_KEY_OFF2		= 0x00FF;

			SPU_FM_MODE1		= 0;
			SPU_FM_MODE2		= 0;

			SPU_NOISE_MODE1		= 0;
			SPU_NOISE_MODE2		= 0;

			SPU_REVERB_ON1		= 0;
			SPU_REVERB_ON2		= 0;
			SPU_REVERB_ADDR		= 0xFFFE;

			SPU_CD_VOL_L		= 0x7FFF;
			SPU_CD_VOL_R		= 0x7FFF;
			
			SPU_EXT_VOL_L		= 0;
			SPU_EXT_VOL_R		= 0;

			DMA_DPCR = OS::DPCR_Set(DMA_DPCR, OS::DMA::SPU, 3); // Enable DMA4
			DMA_CTRL(OS::DMA::SPU).chcr = 0x00000201; // Stop DMA4

			SPU_DMA_CTRL = 0x0004; // Reset transfer mode
			SPU_CTRL     = 0xC001; // Enable SPU, DAC, CD audio, disable DMA request
			StatusSync(0x003F, 0x0001);
		}
	}
}
