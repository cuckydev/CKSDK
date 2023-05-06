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

#include <CKSDK/OS.h>
#include <CKSDK/ExScreen.h>

#include <CKSDK/Mem.h>

extern "C"
{
	// GCC built-in new and delete
	void *__builtin_new(size_t size) { return CKSDK::Mem::Alloc(size); }
	void  __builtin_delete(void *ptr) { CKSDK::Mem::Free(ptr); }

	// Pure virtual call
	void __cxa_pure_virtual(void) { CKSDK::ExScreen::Abort("Pure virtual call"); }

	// C library functions
	void *memset(void *dst, int val, register size_t len)
	{
		char *ptr = (char *)dst;
		while (len-- > 0)
			*ptr++ = val;
		return dst;
	}
}

// C++ new and delete
void *operator new(size_t size) noexcept { return CKSDK::Mem::Alloc(size); }
void *operator new[](size_t size) noexcept { return CKSDK::Mem::Alloc(size); }

void *operator new(size_t size, std::align_val_t align) noexcept { (void)align; return CKSDK::Mem::Alloc(size); }
void *operator new[](size_t size, std::align_val_t align) noexcept { (void)align; return CKSDK::Mem::Alloc(size); }

void operator delete(void *ptr) noexcept { CKSDK::Mem::Free(ptr); }
void operator delete[](void *ptr) noexcept { CKSDK::Mem::Free(ptr); }
void operator delete(void *ptr, size_t size) noexcept { (void)size; CKSDK::Mem::Free(ptr); }
void operator delete[](void *ptr, size_t size) noexcept { (void)size; CKSDK::Mem::Free(ptr); }