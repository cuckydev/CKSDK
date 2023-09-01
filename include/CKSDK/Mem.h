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

/// @file CKSDK/Mem.h
/// @brief CKSDK Memory allocation API

#pragma once

#include <CKSDK/CKSDK.h>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK Memory allocation namespace
	/// @note `new` and `delete` wrap these functions and handle construction and destruction. Only use these if you know what you are doing.
	namespace Mem
	{
		// Mem functions
		/// @brief Initialize the memory allocator
		/// @param ptr Pointer to the heap to use
		/// @param addr Size of the heap to use
		/// @note For internal use only
		void Init(void *ptr, size_t addr);
		
		/// @brief Allocate memory
		/// @param size Size of the memory to allocate
		/// @return Pointer to the allocated memory
		void *Alloc(size_t size);
		/// @brief Reallocate memory
		/// @param ptr Pointer to the memory to reallocate
		/// @param size Size of the memory to allocate
		/// @return Pointer to the reallocated memory
		/// @note No-operation if ptr is null
		void *Realloc(void *ptr, size_t size);
		/// @brief Free memory
		/// @param ptr Pointer to the memory to free
		/// @note No-operation if ptr is null
		void Free(void *ptr);

		/// @brief Profile memory
		/// @param used Pointer to a size_t to store the used memory in
		/// @param total Pointer to a size_t to store the total memory in
		/// @param blocks Pointer to a size_t to store the number of blocks in
		void Profile(size_t *used, size_t *total, size_t *blocks);
	}
}
