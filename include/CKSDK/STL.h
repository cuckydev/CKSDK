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

/// @file CKSDK/STL.h
/// @brief CKSDK STL functions

#pragma once

#include <CKSDK/CKSDK.h>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK STL namespace
	namespace STL
	{
		// String functions
		/// @brief String functions
		namespace String
		{
			/// @brief Compare two strings
			/// @param s1 String 1
			/// @param s2 String 2
			/// @return 0 if strings are equal, -1 if s1 < s2, 1 if s1 > s2
			inline int Compare(const char *s1, const char *s2)
			{
				const char *p1 = (const char *)s1;
				const char *p2 = (const char *)s2;
				for (; *p1 != '\0'; p1++, p2++)
				{
					char u1 = *p1;
					char u2 = *p2;
					if (u1 > u2)
						return 1;
					if (u1 < u2)
						return -1;
				}
				return 0;
			}
		}

		// Memory functions
		/// @brief Memory functions
		namespace Memory
		{
			/// @brief Compare two memory blocks
			/// @param s1 Memory block 1
			/// @param s2 Memory block 2
			/// @param n Number of bytes to compare
			/// @return 0 if memory blocks are equal, -1 if s1 < s2, 1 if s1 > s2
			inline int Compare(const void *s1, const void *s2, size_t n)
			{
				const char *p1 = (const char*)s1;
				const char *p2 = (const char*)s2;
				for (; n--; p1++, p2++)
				{
					char u1 = *p1;
					char u2 = *p2;
					if (u1 > u2)
						return 1;
					if (u1 < u2)
						return -1;
				}
				return 0;
			}

			/// @brief Copy memory
			/// @param d Destination
			/// @param s Source
			/// @param n Number of bytes to copy
			inline void Copy(void *d, const void *s, size_t n)
			{
				char *pd = (char*)d;
				const char *ps = (const char *)s;
				for (; n--; pd++, ps++)
					*pd = *ps;
			}
		}
	}
}
