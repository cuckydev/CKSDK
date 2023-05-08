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

/// @file CKSDK/Timer.h
/// @brief CKSDK Timer API

#pragma once

#include <CKSDK/CKSDK.h>

#include <CKSDK/OS.h>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK Timer namespace
	namespace Timer
	{
		// Timer functions
		/// @brief Initialize timer API
		/// @note For internal use only
		void Init();

		/// @brief Timer callback type
		typedef OS::Function<void> TimerCallback;
		/// @brief Set timer
		/// @param rate Timer rate (Hz)
		/// @param cb Timer callback
		/// @note `rate` must be greater than 64
		void Set(uint32_t rate, TimerCallback cb);
	}
}
