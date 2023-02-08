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

#include <CKSDK/ExScreen.h>

#include <CKSDK/OS.h>
#include <CKSDK/GPU.h>
#include <CKSDK/SPI.h>

namespace CKSDK
{
	namespace ExScreen
	{
		// Exception screen globals
		static constexpr uint32_t WIDTH = 640;
		static constexpr uint32_t HEIGHT = 480;
		static constexpr uint32_t MARGIN = 24;

		OS::Thread g_fp;
		const char *g_reason = nullptr;

		// Exception screen text drawing
		#include "OS/ExScreen_Font.h"

		static void Out(const char *str, unsigned x, unsigned y)
		{
			while (1)
			{
				char c = *str++;
				if (c == '\0')
					return;
				GPU::GP0_Cmd((GPU::GP0_Rect | GPU::GP0_Rect_8x8 | GPU::GP0_Rect_Tex | GPU::GP0_Rect_Raw) << 24);
				GPU::GP0_Data((x << 0) | (y << 16));
				GPU::GP0_Data((((c & 0xF) << 3) << 0) | ((((c - 0x20) >> 4) << 3) << 8) | ((62) << 16));
				x += 8;
			}
		}

		static void OutWord(uint32_t value, unsigned x, unsigned y)
		{
			static const char *map = "0123456789ABCDEF";
			char vstr[9];
			vstr[8] = '\0';
			for (unsigned i = 0; i < 8; i++, value <<= 4)
				vstr[i] = map[value >> 28];
			Out(vstr, x, y);
		}

		static void OutRegister(const char *str, uint32_t value, unsigned x, unsigned y)
		{
			Out(str, x, y);
			x += 7 * 8;
			OutWord(value, x, y);
		}

		// Exception screen
		static void RegisterDump()
		{
			// Write registers
			Out("REGISTER DUMP", MARGIN, MARGIN + (8 * 0));

			OutRegister("CAUSE", g_fp.cause, MARGIN + (128 * 0), (MARGIN + 32) + (8 * 0));
			OutRegister("EPC", g_fp.epc, MARGIN + (128 * 1), (MARGIN + 32) + (8 * 0));
			OutRegister("SR", g_fp.sr, MARGIN + (128 * 0), (MARGIN + 32) + (8 * 1));
			
			OutRegister("AT", g_fp.at, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 0));

			OutRegister("V0", g_fp.v0, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 1));
			OutRegister("V1", g_fp.v1, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 1));

			OutRegister("A0", g_fp.a0, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 2));
			OutRegister("A1", g_fp.a1, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 2));

			OutRegister("A2", g_fp.a2, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 3));
			OutRegister("A3", g_fp.a3, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 3));

			OutRegister("T0", g_fp.t0, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 4));
			OutRegister("T1", g_fp.t1, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 4));
			OutRegister("T2", g_fp.t2, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 5));
			OutRegister("T3", g_fp.t3, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 5));
			OutRegister("T4", g_fp.t4, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 6));
			OutRegister("T5", g_fp.t5, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 6));
			OutRegister("T6", g_fp.t6, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 7));
			OutRegister("T7", g_fp.t7, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 7));

			OutRegister("S0", g_fp.s0, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 8));
			OutRegister("S1", g_fp.s1, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 8));
			OutRegister("S2", g_fp.s2, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 9));
			OutRegister("S3", g_fp.s3, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 9));
			OutRegister("S4", g_fp.s4, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 10));
			OutRegister("S5", g_fp.s5, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 10));
			OutRegister("S6", g_fp.s6, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 11));
			OutRegister("S7", g_fp.s7, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 11));

			OutRegister("T8", g_fp.t8, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 12));
			OutRegister("T9", g_fp.t9, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 12));

			OutRegister("GP", g_fp.gp, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 13));
			OutRegister("SP", g_fp.sp, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 13));
			OutRegister("FP", g_fp.fp, MARGIN + (128 * 0), (MARGIN + 64) + (8 * 14));
			OutRegister("RA", g_fp.ra, MARGIN + (128 * 1), (MARGIN + 64) + (8 * 14));

			uint32_t excode = (g_fp.cause & 0x7C) >> 2;
			static const char *excode_reason[0x20] = {
				"INTERRUPT",
				"TLB MODIFICATION",
				"TLB LOAD",
				"TLB STORE",
				"ADDRESS ERROR IN LOAD",
				"ADDRESS ERROR IN STORE",
				"BUS ERROR ON I-FETCH",
				"BUS ERROR ON LOAD / STORE",
				"SYSCALL",
				"BREAKPOINT",
				"RESERVED INSTRUCTION",
				"COP UNUSABLE",
				"ARITHMETIC OVERFLOW"
			};

			if (excode == 0x09 && g_reason != nullptr)
				Out(g_reason, MARGIN, HEIGHT - MARGIN - 8);
			else if (excode_reason[excode] != nullptr)
				Out(excode_reason[excode], MARGIN, HEIGHT - MARGIN - 8);
		}

		static void StackDump()
		{
			Out("STACK DUMP", MARGIN, MARGIN + (8 * 0));

			OutRegister("SP", g_fp.sp, WIDTH - MARGIN - 128, MARGIN + (8 * 0));
			OutRegister("RA", g_fp.ra, WIDTH - MARGIN - 128, MARGIN + (8 * 1));

			if ((g_fp.sp & 3) != 0 || g_fp.sp < 0x80000000 || g_fp.sp >= 0x80200000)
			{
				Out("BAD SP", MARGIN, MARGIN + (8 * 2));
				return;
			}

			unsigned x = MARGIN;
			unsigned y = MARGIN + (8 * 4);
			for (uint32_t *p = (uint32_t*)g_fp.sp; p != (uint32_t*)0x80200000; p++)
			{
				OutWord(*p, x, y);
				if ((x += (8 * 9)) >= (WIDTH - MARGIN - (8 * 8)))
				{
					x = MARGIN;
					y += 8;
				}
			}
		}

		void Main()
		{
			// Reset GPU
			GPU::Init();
			GPU::SetScreen(WIDTH, HEIGHT, 0, 0, 0, 0, 0, 0);
			GPU::QueueReset();

			// Reset SPI
			SPI::Init();

			// Load font
			GPU::DataSync();
			GPU::CHCRSync();
			GPU::LoadImage((uint32_t*)os_font_tex, 1024 - 64, 0, 32, 48);
			GPU::QueueReset();

			GPU::DataSync();
			GPU::CHCRSync();
			GPU::LoadImage((uint32_t*)os_font_clut, 1024 - 32, 0, 16, 1);
			GPU::QueueReset();

			GPU::DataSync();
			GPU::CHCRSync();

			// Show screen
			static void (*screens[])() = {
				RegisterDump,
				StackDump
			};
			unsigned screen = 0;
			
			while (1)
			{
				// Prepare GPU
				GPU::GP1_Cmd(GPU::g_bufferp->gp1_vram);
				GPU::GP1_Cmd(GPU::g_bufferp->gp1_hspan);
				GPU::GP1_Cmd(GPU::g_bufferp->gp1_vspan);
				GPU::GP1_Cmd(GPU::g_bufferp->gp1_mode);

				GPU::GP1_Cmd((GPU::GP1_DisplayEnable << 24) | 0);

				GPU::GP0_Cmd(GPU::g_bufferp->gp0.tl);
				GPU::GP0_Cmd(GPU::g_bufferp->gp0.br);
				GPU::GP0_Cmd(GPU::g_bufferp->gp0.off);
				GPU::GP0_Cmd(GPU::g_bufferp->gp0.mode);

				GPU::GP0_Cmd((GPU::GP0_FillRect << 24) | (0xFF << 16) | (0 << 8) | (0 << 0));
				GPU::GP0_Data((0 << 0) | (0 << 16));
				GPU::GP0_Data((WIDTH << 0) | (HEIGHT << 16));
				
				GPU::GP0_Cmd((GPU::GP0_DrawMode << 24) | (15 << 0) | (0 << 9) | (1 << 10));

				// Display screen
				Out("[X] - SWITCH PAGE", MARGIN, MARGIN + (8 * 1));
				screens[screen]();

				// Wait for key press
				while (1)
				{
					SPI::PollPads();
					if (SPI::g_pad[0].press & SPI::PadButton::Cross)
					{
						screen = (screen + 1) % COUNTOF(screens);
						break;
					}
				}
			}
		}
	}
}
