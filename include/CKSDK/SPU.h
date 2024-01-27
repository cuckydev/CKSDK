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

/// @file CKSDK/SPU.h
/// @brief CKSDK SPU API

#pragma once

#include <CKSDK/CKSDK.h>

#include <CKSDK/OS.h>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK SPU namespace
	namespace SPU
	{
		// SPU functions
		/// @brief Initialize SPU
		/// @note For internal use only
		void Init();

		/// @brief DMA between SPU RAM and main RAM
		/// @param data Pointer in main RAM to DMA
		/// @param length Length in bytes to DMA
		/// @param addr Address in SPU RAM to DMA
		/// @param write `true` if DMAing from main RAM to SPU RAM, `false` if DMAing from SPU RAM to main RAM
		void DMA(void *data, size_t length, uint16_t addr, bool write);
		/// @brief Wait for SPU dma to finish
		void DMASync();
	}
}
