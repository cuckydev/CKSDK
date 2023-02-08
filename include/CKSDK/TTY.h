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

/// @file CKSDK/TTY.h
/// @brief CKSDK TTY API

#pragma once

#include <CKSDK/CKSDK.h>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK TTY namespace
	namespace TTY
	{
		// TTY functions
		/// @brief Initialize TTY API
		/// @note For internal use only
		void Init();
		/// @brief Output string to TTY
		/// @param str String to output
		void Out(const char *str);

		/// @brief Output hex word to TTY
		/// @param x Word to output
		/// @tparam B Byte count
		template<int B>
		inline void OutHex(uint32_t x)
		{
			char s[(B << 1) + 1];
			for (int i = 0; i < (B << 1); i++, x <<= 4)
				s[i] = "0123456789ABCDEF"[(x >> ((B << 3) - 4)) & 0xF];
			s[(B << 1)] = '\0';
			Out(s);
		}

		/// @brief Output type to TTY
		/// @param x Type to output
		/// @tparam T Type
		template<typename T>
		inline void OutHex(T x)
		{
			uint8_t *p = (uint8_t*)&x;
			uint8_t *e = p + sizeof(T);
			for (; p != e; p++)
				OutHex<1>(*p);
		}
	}
}
