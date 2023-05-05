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

#include <CKSDK/OS.h>

#include <CKSDK/GPU.h>
#include <CKSDK/TTY.h>
#include <CKSDK/DLL.h>

#include <CKSDK/ExScreen.h>

// C externs
extern "C"
{
	void CKSDK_OS_ISR(void);
	uint32_t CKSDK_OS_irq_recurse;
}

namespace CKSDK
{
	namespace OS
	{
		// OS globals
		static InterruptCallback interrupt_callbacks[16] = {};
		static InterruptCallback dma_callbacks[8] = {};
		static uint32_t dma_callbacks_count = 0;

		static void **bios_a0_tbl = (void**)0x80000200;
		static void (*bios_FlushICache)();

		// ISR callback
		extern "C" Thread *CKSDK_OS_ISR_Callback(Thread *fp)
		{
			// Handle excode
			switch ((fp->cause & 0x7C) >> 2)
			{
				case 0: // Interrupt
					// Increment EPC if we interrupted a GTE instruction
					// Apparently this isn't the correct solution
					// if (((*((uint32_t*)fp->epc) >> 24) & 0xFE) == 0x4A)
					// 	fp->epc += 4;
					break;
				case 8: // Syscall
					// Increment EPC so we don't run the syscall again
					fp->epc += 4;
					break;
				default:
					// Enter exception screen
					DisableIRQ_SR_RFE(); // Actually disable IRQs when we return
					::CKSDK_OS_irq_recurse++;

					ExScreen::g_fp = *fp;
					fp->epc = (uint32_t)ExScreen::Main;
					return fp;
			}

			// Issue IRQ callbacks
			uint16_t stat = OS::IrqStat() & OS::IrqMask();
			for (; stat; stat = OS::IrqStat() & OS::IrqMask())
			{
				for (int i = 0, mask = 1; stat; i++, stat >>= 1, mask <<= 1)
				{
					// Check if this IRQ was triggered
					if (!(stat & 1))
						continue;

					// Acknowledge IRQ and call callback
					OS::IrqStat() = (uint16_t)(mask ^ 0xFFFF);
					if (interrupt_callbacks[i] != nullptr)
						(interrupt_callbacks[i])();
				}
			}

			return fp;
		}

		// DMA callback
		static void DMACallback()
		{
			// Issue DMA callbacks
			uint32_t dicr = OS::DmaDicr();
			uint32_t stat = (dicr >> 24) & 0x7F;

			for (; stat; dicr = OS::DmaDicr(), stat = (dicr >> 24) & 0x7F)
			{
				uint32_t base = dicr & 0x00FFFFFF;
				for (int i = 0, mask = (1 << 24); stat; i++, stat >>= 1, mask <<= 1)
				{
					// Check if this DMA was triggered
					if (!(stat & 1))
						continue;

					// Acknowledge DMA and call callback
					OS::DmaDicr() = base | mask;
					if (dma_callbacks[i] != nullptr)
						(dma_callbacks[i])();
				}
			}
		}

		// OS functions
		void Init()
		{
			// Reset interrupt controller
			DisableIRQ_SR();
			OS::IrqMask() = 0;
			OS::IrqStat() = 0;

			// Reset DMA controller
			OS::DmaDpcr() &= ~0x0888888;
			OS::DmaDicr() = 0;
			
			// Recall OS functions
			bios_FlushICache = (void(*)())bios_a0_tbl[0x44];

			// Install ISR
			uint32_t *ip = (uint32_t*)0x80000080;

			// j CKSDK_OS_ISR
			ip[0] = (2 << 26) | (((uintptr_t)::CKSDK_OS_ISR >> 2) & 0x3FFFFFF);
			// nop
			ip[1] = 0;

			// Flush I-cache
			bios_FlushICache();

			// Enable ISR
			EnableIRQ_SR();
		}

		InterruptCallback SetIRQ(IRQ irq, InterruptCallback cb)
		{
			unsigned i = unsigned(irq);
			InterruptCallback old_cb = interrupt_callbacks[i];
			if (cb != nullptr)
			{
				// Set interrupt callback
				OS::IrqMask() |= (1 << i);
				interrupt_callbacks[i] = cb;
			}
			else
			{
				// Clear interrupt callback
				OS::IrqMask() &= ~(1 << i);
				interrupt_callbacks[i] = nullptr;
			}

			return old_cb;
		}

		InterruptCallback GetIRQ(IRQ irq)
		{
			unsigned i = unsigned(irq);
			return interrupt_callbacks[i];
		}

		InterruptCallback SetDMA(DMA dma, InterruptCallback cb)
		{
			// Set DMA callback
			unsigned i = unsigned(dma);
			InterruptCallback old_cb = dma_callbacks[i];
			dma_callbacks[i] = cb;

			if (cb != nullptr && old_cb == nullptr)
			{
				OS::DmaDicr() |= (0x10000 << i) | (1 << 23);

				if (dma_callbacks_count++ == 0)
					SetIRQ(IRQ::DMA, DMACallback);
			}
			else if (cb == nullptr && old_cb != nullptr)
			{
				if (--dma_callbacks_count != 0)
				{
					OS::DmaDicr() &= ~(0x10000 << i);
				}
				else
				{
					OS::DmaDicr() = 0;
					SetIRQ(IRQ::DMA, nullptr);
				}
			}

			return old_cb;
		}

		InterruptCallback GetDMA(DMA dma)
		{
			unsigned i = unsigned(dma);
			return dma_callbacks[i];
		}
		
		void FlushICache()
		{
			bios_FlushICache();
		}
	}
}
