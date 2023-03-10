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

#include <CKSDK/GPU.h>

#include <CKSDK/OS.h>
#include <CKSDK/TTY.h>

#include <CKSDK/Util/Queue.h>

namespace CKSDK
{
	namespace GPU
	{
		// GPU constants
		static constexpr unsigned VSYNC_TIMEOUT = 0x100000;

		// GPU globals
		bool g_pal = false;

		// GPU buffers
		static Buffer buffers[2];
		Buffer *g_bufferp;

		// VBlank callback
		static VBlankCallback vblank_callback = nullptr;
		static volatile uint32_t vblank_counter;

		static void IRQ_VBlank()
		{
			// Call vblank callback
			vblank_counter++;
			if (vblank_callback != nullptr)
				vblank_callback();
		}

		// GPU draw queue
		static QueueCallback queue_callback = nullptr;

		struct DrawQueueArgs
		{
			uint32_t arg[6];
		};
		static Queue::Queue<DrawQueueArgs, 16> draw_queue;

		static void IRQ_DMA()
		{
			// Dispatch next draw queue command
			if (draw_queue.Dispatch())
			{
				// Disable DMA request and call queue callback
				GP1_Cmd((GP1_DMADirection << 24) | 0);
				if (queue_callback != nullptr)
					queue_callback();
			}
		}

		// Queue commands
		void Queue_DMAImage(const DrawQueueArgs &args)
		{
			// Get arguments
			uint32_t addr = args.arg[0];
			uint32_t xy = args.arg[1];
			uint32_t wh = args.arg[2];
			uint32_t bcr = args.arg[3];
			uint32_t write = args.arg[4];

			// Disable DMA
			DataSync();

			GP1_Cmd((GP1_DMADirection << 24) | 0);
			GP0_Cmd(GP0_FlushCache << 24);

			// Set DMA command
			GP0_Cmd(write ? (GP0_ToVRAM << 24) : (GP0_FrVRAM << 24));
			GP0_Data(xy);
			GP0_Data(wh);
			// DataSync();

			// Set DMA direction
			GP1_Cmd((GP1_DMADirection << 24) | (write ? 2 : 3));

			// Start DMA
			DMA_CTRL(OS::DMA::GPU).madr = addr;
			DMA_CTRL(OS::DMA::GPU).bcr = bcr;
			DMA_CTRL(OS::DMA::GPU).chcr = 0x01000200 | write;
		}

		void Queue_DrawOT(const DrawQueueArgs &args)
		{
			// Get arguments
			size_t ot = args.arg[0];

			// Set DMA direction
			DataSync();
			GPU_GP1 = (GP1_DMADirection << 24) | 2;

			// Wait for DMA to be ready
			CmdSync();
			DataSync();
			CHCRSync();

			// Start DMA
			DMA_CTRL(OS::DMA::GPU).madr = ot;
			DMA_CTRL(OS::DMA::GPU).bcr = 0;
			DMA_CTRL(OS::DMA::GPU).chcr = 0x01000401;
		}

		// GPU functions
		void Init()
		{
			// Disable IRQs while we work on setting up the GPU
			OS::DisableIRQ();

			// Get PAL flag from GPU
			if ((GPU_GP1 >> 20) & 1)
				g_pal = true;
			
			// Disable display
			GP1_Cmd((GP1_DisplayEnable << 24) | 1);
			
			// Setup IRQ
			OS::SetIRQ(OS::IRQ::VBLANK, IRQ_VBlank);
			OS::SetDMA(OS::DMA::GPU, IRQ_DMA);

			// Enable DMA2 and DMA6
			DMA_DPCR = OS::DPCR_Set(OS::DPCR_Set(DMA_DPCR, OS::DMA::GPU, 3), OS::DMA::OTC, 3);
			DMA_CTRL(OS::DMA::GPU).chcr = 0x201;
			DMA_CTRL(OS::DMA::OTC).chcr = 0x200;

			// Reset GPU
			GP1_Cmd(GP1_Reset << 24);
			GP1_Cmd(GP1_Flush << 24);

			TIMER_CTRL(0).ctrl = 0x0500;
			TIMER_CTRL(1).ctrl = 0x0500;

			// Initialize GTE
			{
				uint32_t v0, v1, t0;
				INLINE_ASM(
					// Enable COP2
					"mfc0    %0, $12;"
					"lui     %1, 0x4000;"
					"or      %0, $v1;"
					"mtc0    %0, $12;"
					"nop;"
					// Initialize average Z registers
					"li      %2, 0x155;"
					"ctc2    %2, $29;"
					"nop;"
					"li      %2, 0x100;"
					"ctc2    %2, $30;"
					"nop;"
					// Initialize focal length and depth queing registers
					"li      %2, 0x3E8;"
					"ctc2    %2, $26;"
					"nop;"
					"li      %2, 0xFFFFEF9E;"
					"ctc2    %2, $27;"
					"nop;"
					"lui     %2, 0x140;"
					"ctc2    %2, $28;"
					"nop;"
					// Initialize screen offset X and Y registers
					"ctc2    $zero, $24;"
					"ctc2    $zero, $25;"
					"nop;"
					: "=r"(v0), "=r"(v1), "=r"(t0) ::
				);
			}
			
			// Restore IRQs
			OS::EnableIRQ();
		}

		void SetBuffer(Word *buffer, size_t size, size_t ot_size)
		{
			// Setup buffers
			Word *bufferp = buffer;
			size >>= 1;
			for (auto &i : buffers)
			{
				i.buffer = bufferp;
				i.ot_size = ot_size;
				bufferp += size;
			}

			// Initialize buffers
			g_bufferp = &buffers[0];
			g_bufferp->Init();
		}

		void SetScreen(uint32_t w, uint32_t h, uint32_t ox, uint32_t oy, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)
		{
			// Set buffer framebuffer commands
			buffers[0].gp0.tl  = (GP0_DrawTL << 24)     | ((x0) << 0)         | ((y0) << 10);
			buffers[0].gp0.br  = (GP0_DrawBR << 24)     | ((x0 + w - 1) << 0) | ((y0 + h - 1) << 10);
			buffers[0].gp0.off = (GP0_DrawOffset << 24) | ((x0 + ox) << 0)    | ((y0 + oy) << 11);

			buffers[0].gp1_vram = (GP1_DisplayVRAM << 24) | (x0 << 0) | (y0 << 10);

			buffers[1].gp0.tl  = (GP0_DrawTL << 24)     | ((x1) << 0)         | ((y1) << 10);
			buffers[1].gp0.br  = (GP0_DrawBR << 24)     | ((x1 + w - 1) << 0) | ((y1 + h - 1) << 10);
			buffers[1].gp0.off = (GP0_DrawOffset << 24) | ((x1 + ox) << 0)    | ((y1 + oy) << 11);

			buffers[1].gp1_vram = (GP1_DisplayVRAM << 24) | (x1 << 0) | (y1 << 10);
			
			buffers[0].gp0.mode = (GP0_DrawMode << 24) | (0 << 9) | (1 << 10);
			buffers[1].gp0.mode = (GP0_DrawMode << 24) | (0 << 9) | (1 << 10);

			// Setup mode
			uint32_t mode = (GP1_DisplayMode << 24);
			if (w == 368)
				mode |= (1 << 6); // 368 pixels wide
			else if (w == 256)
				mode |= (0 << 0); // 256 pixels wide
			else if (w == 320)
				mode |= (1 << 0); // 320 pixels wide
			else if (w == 512)
				mode |= (2 << 0); // 512 pixels wide
			else if (w == 640)
				mode |= (3 << 0); // 640 pixels wide
			else
				ExScreen::Abort("Invalid width for SetScreen");
			
			if (h > 256)
			{
				mode |= (1 << 2) | (1 << 5); // Interlaced
				h /= 2;
			}

			// Setup spans
			uint32_t hspan = (GP1_DisplayHSpan << 24);
			hspan |= (0x260 + 0) << 0; // The 260h value is the first visible pixel on standard TV Sets
			hspan |= (0x260 + w * 8) << 12;

			uint32_t vspan = (GP1_DisplayVSpan << 24);
			if (g_pal)
			{
				// The A3h value is the middle scanline on normal TV Sets (PAL)
				vspan |= (0xA3 - (h / 2)) << 0;
				vspan |= (0xA3 + (h / 2)) << 10;
			}
			else
			{
				// The 88h value is the middle scanline on normal TV Sets (NTSC)
				vspan |= (0x88 - (h / 2)) << 0;
				vspan |= (0x88 + (h / 2)) << 10;
			}

			// Setup display commands
			for (auto &i : buffers)
			{
				i.gp1_hspan = hspan;
				i.gp1_vspan = vspan;
				i.gp1_mode = mode;
			}
		}

		static FlipCallback flip_callback = nullptr;
		void Flip()
		{
			Buffer *bufferp = g_bufferp;

			// Sync
			QueueSync();
			VBlankSync();
			
			// Set display framebuffer
			GP1_Cmd(bufferp->gp1_vram);
			GP1_Cmd(bufferp->gp1_hspan);
			GP1_Cmd(bufferp->gp1_vspan);
			GP1_Cmd(bufferp->gp1_mode);
			
			GP1_Cmd((GP1_DisplayEnable << 24) | 0);
			
			// Call flip callback
			if (flip_callback != nullptr)
				flip_callback();

			// Set draw framebuffer area
			AllocPacket<Buffer::GP0>(bufferp->ot_size - 1) = bufferp->gp0;

			// Send OT to GPU
			draw_queue.Enqueue(Queue_DrawOT, DrawQueueArgs{
				uint32_t(&bufferp->GetOT(bufferp->ot_size - 1))
			});

			// Flip and initialize buffer
			bufferp = (bufferp == &buffers[0]) ? &buffers[1] : &buffers[0];
			g_bufferp = bufferp;
			bufferp->Init();
		}
		
		void VBlankSync()
		{
			// Wait for vblank
			uint32_t counter = vblank_counter;
			for (unsigned i = VSYNC_TIMEOUT; i != 0; i--)
			{
				if (counter != vblank_counter)
					return;
			}

			TTY::Out("GPU vsync timeout\n");
		}

		void QueueSync()
		{
			// Wait for queue to clear up
			draw_queue.Sync();

			// Wait for DMA to finish
			if (GPU_GP1 & (3 << 29))
			{
				DataSync();
				CHCRSync();
			}
			
			// Wait until GPU is ready to receive a new command
			CmdSync();
		}

		void QueueReset()
		{
			draw_queue.Reset();
		}
		
		void DMAImage(const void *addr, uint32_t xy, uint32_t wh, uint32_t bcr)
		{
			draw_queue.Enqueue(Queue_DMAImage, DrawQueueArgs{
				uint32_t(addr), xy, wh, bcr, 1
			});
		}

		FlipCallback SetFlipCallback(FlipCallback cb)
		{
			FlipCallback old_cb = flip_callback;
			flip_callback = cb;
			return old_cb;
		}
		FlipCallback GetFlipCallback()
		{
			return flip_callback;
		}

		VBlankCallback SetVBlankCallback(VBlankCallback cb)
		{
			VBlankCallback old_cb = vblank_callback;
			vblank_callback = cb;
			return old_cb;
		}
		VBlankCallback GetVBlankCallback()
		{
			return vblank_callback;
		}

		QueueCallback SetQueueCallback(QueueCallback cb)
		{
			QueueCallback old_cb = queue_callback;
			queue_callback = cb;
			return old_cb;
		}
		QueueCallback GetQueueCallback()
		{
			return queue_callback;
		}
	}
}
