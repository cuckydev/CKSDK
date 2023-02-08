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

/// @file CKSDK/Util/Hash.h
/// @brief CKSDK string hash utility

#pragma once

#include <CKSDK/CKSDK.h>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK hash namespace
	/// @details This is an implementation of a Fowler–Noll–Vo hash function
	/// @see https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
	namespace Hash
	{
		/// @brief Hash type
		typedef uint32_t Hash;
		
		/// @brief FNV prime value
		static const Hash FNV32_PRIME = 0x01000193;
		/// @brief FNV initial value
		static const Hash FNV32_IV    = 0x811C9DC5;

		/// @brief Hash a string literal
		/// @param literal String literal
		/// @param max_length Maximum length of the string literal
		/// @param accumulator Accumulator value
		/// @return Hash value
		constexpr static inline Hash FromConst(const char *const literal, size_t max_length = 0xFFFFFFFF, Hash accumulator = FNV32_IV)
		{
			if (*literal && max_length)
				return FromConst(&literal[1], max_length - 1, (accumulator ^ Hash(*literal)) * FNV32_PRIME);
			return accumulator;
		}

		/// @brief Hash a buffer
		/// @param data Buffer data
		/// @param length Buffer length
		/// @return Hash value
		static inline Hash FromBuffer(const uint8_t *data, size_t length)
		{
			Hash accumulator = FNV32_IV;
			while (length-- > 0)
				accumulator = (accumulator ^ Hash(*data++)) * FNV32_PRIME;
			return accumulator;
		}

		/// @brief Hash a string
		/// @param string String
		/// @return Hash value
		static inline Hash FromString(const char *string)
		{
			Hash accumulator = FNV32_IV;
			while (*string != '\0')
				accumulator = (accumulator ^ Hash(*string++)) * FNV32_PRIME;
			return accumulator;
		}
	}
}

/// @brief Hash literal operator
/// @param literal Literal string
/// @param length Literal length
/// @return Hash value
constexpr static inline CKSDK::Hash::Hash operator"" _h(const char *const literal, size_t length)
{
	return CKSDK::Hash::FromConst(literal, length);
}
