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

		static constexpr unsigned DMA_CHUNK_LENGTH = 16;

		// Internal SPU functions
		void StatusSync(uint16_t mask, uint16_t value)
		{
			for (unsigned i = STATUS_TIMEOUT; i != 0; i--)
				if ((OS::SpuStat() & mask) == value)
					return;
			TTY::Out("SPU status sync timeout\n");
		}

		// SPU functions
		KEEP void Init()
		{
			// Initialize SPU bus config
			OS::SpuBIU() = 0
				| (1 << OS::BIU_WriteDelayShift)
				| (14 << OS::BIU_ReadDelayShift)
				| OS::BIU_Recovery
				| OS::BIU_Width16
				| OS::BIU_AutoIncrement
				| (9 << OS::BIU_SizeShift)
				| OS::BIU_UseDmaDelay
				| (0 << OS::BIU_DmaDelayShift);

			// Reset SPU
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
			OS::SpuCtrl() = 0
				| (1 << 0)   // CD audio enable
				| (1 << 14)  // Unmute SPU
				| (1 << 15); // Enable SPU
			StatusSync(0x003F, 0x0001);

			// DMA dummy block to 0x1000 and reset all channels
			uint32_t dummy[4] = { 0x0500 };
			DMA(dummy, sizeof(dummy), 0x1000 / 8, true);
			DMASync();

			for (int i = 0; i < 24; i++)
			{
				auto &voice = OS::SpuVoiceCtrl(i);
				voice.addr = voice.loop_addr = 0x1000 / 8;
				voice.vol_l = 0;
				voice.vol_r = 0;
				voice.freq = 0;
				voice.adsr = 0;
			}
			OS::SpuKeyOn() = 0xFFFFFF;
		}

		KEEP void DMA(void *data, size_t length, uint16_t addr, bool write)
		{
			// Check length
			if (length % 4)
				TTY::Out("SPU DMA requires 4 byte alignment\n");

			length /= 4;
			if ((length >= DMA_CHUNK_LENGTH) && (length % DMA_CHUNK_LENGTH))
			{
				TTY::Out("SPU DMA > 16 word chunks, but not 16 word aligned. Rounding up\n");
				length += DMA_CHUNK_LENGTH - 1;
			}

			// Increase bus delay for DMA reads
			if (write)
				OS::SpuBIU() &= ~OS::BIU_DmaDelay;
			else
				OS::SpuBIU() = (OS::SpuBIU() & ~OS::BIU_DmaDelay) | (2 << OS::BIU_DmaDelayShift);

			OS::SpuCtrl() &= ~(3 << 4); // Disable DMA request
			StatusSync(0x0030, 0x0000);

			// Enable DMA request for writing (2) or reading (3)
			uint16_t ctrl = write ? 0x0020 : 0x0030;

			OS::SpuAddr() = addr;
			OS::SpuCtrl() |= ctrl;
			StatusSync(0x0030, ctrl);

			OS::DmaCtrl(OS::DMA::SPU).madr = (uint32_t)data;
			if (length < DMA_CHUNK_LENGTH)
				OS::DmaCtrl(OS::DMA::SPU).bcr = 0x00010000 | length;
			else
				OS::DmaCtrl(OS::DMA::SPU).bcr = DMA_CHUNK_LENGTH | ((length / DMA_CHUNK_LENGTH) << 16);

			OS::DmaCtrl(OS::DMA::SPU).chcr = 0x01000200 | write;
		}

		KEEP void DMASync()
		{
			while ((CKSDK::OS::SpuStat() >> 10) & 1);
		}
	}
}
