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

#include <CKSDK/GPU.h>

static constexpr unsigned g_width = 320;
static constexpr unsigned g_height = 240;

extern "C" void main()
{
	// Initialize CKSDK
	CKSDK::Init();
	
	// Initialize GPU buffer and screen
	static CKSDK::GPU::Word buffer[0x200];
	CKSDK::GPU::SetBuffer(buffer, sizeof(buffer) / sizeof(CKSDK::GPU::Word), 1);
	CKSDK::GPU::SetScreen(g_width, g_height, 0, 0, 0, 0, 0, g_height);

	// Test loop
	while (1)
	{
		// Allocate polys
		struct PolyTest
		{
			CKSDK::GPU::FillPrim<> bg;
			CKSDK::GPU::PolyPrim<true, true, false> poly;
		};

		PolyTest &poly_test = CKSDK::GPU::AllocPacket<PolyTest>(0);

		// Setup background polygon to fill entire screen
		poly_test.bg.c = CKSDK::GPU::Color(0x00, 0x80, 0x80);
		poly_test.bg.xy = CKSDK::GPU::ScreenCoord(0, 0);
		poly_test.bg.wh = CKSDK::GPU::ScreenDim(g_width, g_height);

		// Setup foreground polygon
		poly_test.poly.v0.c = CKSDK::GPU::Color(0xFF, 0x00, 0x00);
		poly_test.poly.v1.c = CKSDK::GPU::Color(0x00, 0xFF, 0x00);
		poly_test.poly.v2.c = CKSDK::GPU::Color(0x00, 0x00, 0xFF);
		poly_test.poly.v3.c = CKSDK::GPU::Color(0xFF, 0xFF, 0xFF);
		
		poly_test.poly.v0.xy = CKSDK::GPU::ScreenCoord(64, 64);
		poly_test.poly.v1.xy = CKSDK::GPU::ScreenCoord(256, 64);
		poly_test.poly.v2.xy = CKSDK::GPU::ScreenCoord(64, 176);
		poly_test.poly.v3.xy = CKSDK::GPU::ScreenCoord(256, 176);

		// Flip screen
		CKSDK::GPU::Flip();
	}
}
