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
		KEEP bool g_pal = false;

		// GPU buffers
		static Buffer buffers[2];
		KEEP Buffer *g_bufferp;

		// Callbacks
		static FlipCallback flip_callback = nullptr;
		static VBlankCallback vblank_callback = nullptr;
		static QueueCallback queue_callback = nullptr;

		// VBlank callback
		static volatile uint32_t vblank_counter;

		static void IRQ_VBlank()
		{
			// Call vblank callback
			vblank_counter = vblank_counter + 1;
			if (vblank_callback != nullptr)
				vblank_callback();
		}

		// GPU draw queue

		struct GPUQueueArgs
		{
			uint32_t arg[6];
		};
		static Queue::Queue<GPUQueueArgs, 16> gpu_queue;

		static void IRQ_DMA()
		{
			// Dispatch next draw queue command
			if (gpu_queue.Dispatch())
			{
				// Disable DMA request and call queue callback
				GP1_Cmd((GP1_DMADirection << 24) | 0);
				if (queue_callback != nullptr)
					queue_callback();
			}
		}

		// GPU functions
		KEEP void Init()
		{
			// Disable IRQs while we work on setting up the GPU
			OS::DisableIRQ();

			// Get PAL flag from GPU
			if ((OS::GpuGp1() >> 20) & 1)
				g_pal = true;
			
			// Disable display
			GP1_Cmd((GP1_DisplayEnable << 24) | 1);
			
			// Setup IRQ
			OS::SetIRQ(OS::IRQ::VBLANK, IRQ_VBlank);
			OS::SetDMA(OS::DMA::GPU, IRQ_DMA);

			// Enable DMA2 and DMA6
			OS::DmaDpcr() = OS::DpcrSet(OS::DpcrSet(OS::DmaDpcr(), OS::DMA::GPU, 3), OS::DMA::OTC, 3);
			OS::DmaCtrl(OS::DMA::GPU).chcr = 0x201;
			OS::DmaCtrl(OS::DMA::OTC).chcr = 0x200;

			// Reset GPU
			GP1_Cmd(GP1_Reset << 24);
			GP1_Cmd(GP1_Flush << 24);

			OS::TimerCtrl(0).ctrl = 0x0500;
			OS::TimerCtrl(1).ctrl = 0x0500;

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

			// Clear VRAM
			for (int x = 0; x < 1024; x += 512)
			{
				for (int y = 0; y < 512; y += 256)
				{
					GP0_Cmd((GP0_FillRect << 24) | (0x00 << 0) | (0x00 << 8) | (0x00 << 16));
					GP0_Data((x << 0) | (y << 16));
					GP0_Data((512 << 0) | (256 << 16));
				}
			}
			
			// Restore IRQs
			OS::EnableIRQ();
		}

		KEEP void SetBuffer(Word *buffer, size_t size, size_t ot_size)
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

		KEEP void SetScreen(uint32_t w, uint32_t h, uint32_t ox, uint32_t oy, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)
		{
			// Set buffer framebuffer commands
			g_bufferp = &buffers[0];

			buffers[0].draw_environment = DrawEnvironment(x0, y0, w, h, ox, oy);
			buffers[0].display_environment = DisplayEnvironment(x1, y1, w, h);

			buffers[1].draw_environment = DrawEnvironment(x0, y0, w, h, ox, oy);
			buffers[1].display_environment = DisplayEnvironment(x0, y0, w, h);

			// Setup mode
			uint32_t mode = (GP1_DisplayMode << 24);
			if (g_pal)
				mode |= (1 << 3);

			uint32_t h_dot;
			switch (w)
			{
				case 368:
					mode |= (1 << 6); // 368 pixels wide
					h_dot = 7;
					break;
				case 256:
					mode |= (0 << 0); // 256 pixels wide
					h_dot = 10;
					break;
				case 320:
					mode |= (1 << 0); // 320 pixels wide
					h_dot = 8;
					break;
				case 512:
					mode |= (2 << 0); // 512 pixels wide
					h_dot = 5;
					break;
				case 640:
					mode |= (3 << 0); // 640 pixels wide
					h_dot = 4;
					break;
				default:
					ExScreen::Abort("Invalid width for SetScreen");
					break;
			}

			if (h > 256)
			{
				mode |= (1 << 2) | (1 << 5); // Interlaced
				h /= 2;
			}

			// Setup spans
			uint32_t hspan = (GP1_DisplayHSpan << 24);

			uint32_t hspan_center = 0x760;
			uint32_t hspan_width = (w * h_dot) / 2;
			hspan |= (hspan_center - hspan_width) << 0;
			hspan |= (hspan_center + hspan_width) << 12;

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
				i.display_environment.hspan = hspan;
				i.display_environment.vspan = vspan;
				i.display_environment.mode = mode;
			}
		}

		KEEP void Flip()
		{
			Buffer *bufferp = g_bufferp;

			// Sync
			QueueSync();
			VBlankSync();
			
			// Set display framebuffer
			GP1_Packet(bufferp->display_environment);
			
			GP1_Cmd((GP1_DisplayEnable << 24) | 0);
			
			// Call flip callback
			if (flip_callback != nullptr)
				flip_callback();

			// Send GP0 setup packet
			// These commands are not safe to send during the OT, so we send them here
			GP0_Packet(bufferp->draw_environment);

			// Send OT to GPU
			Queue_OrderingTableDMA(bufferp->GetOT(bufferp->ot_size - 1));

			// Flip and initialize buffer
			bufferp = (bufferp == &buffers[0]) ? &buffers[1] : &buffers[0];
			g_bufferp = bufferp;
			bufferp->Init();
		}
		
		KEEP void VBlankSync()
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

		// Queue commands
		static bool Command_ImageDMA(const GPUQueueArgs &args)
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
			OS::DmaCtrl(OS::DMA::GPU).madr = addr;
			OS::DmaCtrl(OS::DMA::GPU).bcr = bcr;
			OS::DmaCtrl(OS::DMA::GPU).chcr = 0x01000200 | write;

			return false;
		}

		static bool Command_OrderingTableDMA(const GPUQueueArgs &args)
		{
			// Get arguments
			size_t ot = args.arg[0];

			// Set DMA direction
			DataSync();
			GP1_Cmd((GP1_DMADirection << 24) | 2);

			// Wait for DMA to be ready
			CmdSync();
			DataSync();
			CHCRSync();

			// Start DMA
			OS::DmaCtrl(OS::DMA::GPU).madr = ot;
			OS::DmaCtrl(OS::DMA::GPU).bcr = 0;
			OS::DmaCtrl(OS::DMA::GPU).chcr = 0x01000401;

			return false;
		}

		static bool Command_GP1(const GPUQueueArgs &args)
		{
			// Get arguments
			const Word *addr = reinterpret_cast<const Word *>(args.arg[0]);
			size_t size = args.arg[1];

			while (size-- > 0)
				GP1_Cmd(*addr++);

			return true;
		}

		KEEP void QueueSync()
		{
			// Wait for queue to clear up
			gpu_queue.Sync();

			// Sync
			CHCRSync();
			CmdSync();
			DataSync();
		}

		KEEP void QueueReset()
		{
			// Drop all queued commands
			gpu_queue.Reset();
		}

		KEEP void Queue_OrderingTableDMA(const Tag &buffer)
		{
			gpu_queue.Enqueue(Command_OrderingTableDMA, GPUQueueArgs{
				reinterpret_cast<uint32_t>(&buffer)
			});
		}
		
		KEEP void Queue_ImageDMA(const void *addr, uint32_t xy, uint32_t wh, uint32_t bcr)
		{
			gpu_queue.Enqueue(Command_ImageDMA, GPUQueueArgs{
				uint32_t(addr), xy, wh, bcr, 1
			});
		}

		KEEP void Queue_GP1(const Word *addr, size_t size)
		{
			gpu_queue.Enqueue(Command_GP1, GPUQueueArgs{
				uint32_t(addr), size
			});
		}

		// Callbacks
		KEEP FlipCallback SetFlipCallback(FlipCallback cb)
		{
			FlipCallback old_cb = flip_callback;
			flip_callback = cb;
			return old_cb;
		}
		KEEP FlipCallback GetFlipCallback()
		{
			return flip_callback;
		}

		KEEP VBlankCallback SetVBlankCallback(VBlankCallback cb)
		{
			VBlankCallback old_cb = vblank_callback;
			vblank_callback = cb;
			return old_cb;
		}
		KEEP VBlankCallback GetVBlankCallback()
		{
			return vblank_callback;
		}

		KEEP QueueCallback SetQueueCallback(QueueCallback cb)
		{
			QueueCallback old_cb = queue_callback;
			queue_callback = cb;
			return old_cb;
		}
		KEEP QueueCallback GetQueueCallback()
		{
			return queue_callback;
		}
	}
}
