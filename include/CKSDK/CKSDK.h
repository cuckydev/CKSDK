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

/// @file CKSDK/CKSDK.h
/// @brief CKSDK API

#pragma once

// Common headers
#include <cstddef>
#include <cstdint>
#include <new>
#include <iterator>

// ASM macro
#define INLINE_ASM(...) asm __volatile__( __VA_ARGS__ )

// Common macros
#ifndef PACKED
	#define PACKED __attribute__((packed))
#endif

// Keep section
/// @brief Keep section
/// @details This is used to keep a function in the final binary. You should use this for any functions that are called from DLLs.
#define KEEP __attribute__((used))
