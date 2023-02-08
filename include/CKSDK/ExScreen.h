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

/// @file CKSDK/ExScreen.h
/// @brief CKSDK Exception Screen API

#pragma once

#include <CKSDK/CKSDK.h>

#include <CKSDK/OS.h>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK Exception Screen namespace
	namespace ExScreen
	{
		// Exception screen globals
		/// @brief Exception screen thread context
		/// @note For internal use only
		extern OS::Thread g_fp;
		/// @brief Exception screen display reason
		/// @note For internal use only
		extern const char *g_reason;

		// Exception screen functions
		/// @brief Exception screen main function
		/// @note For internal use only
		void Main();

		/// @brief Abort the program
		/// @param reason Display reason
		/// @note This will display the exception screen and halt the program
		inline void Abort(const char *reason = "ExScreen::Abort") { g_reason = reason; INLINE_ASM("break"); }
	}
}