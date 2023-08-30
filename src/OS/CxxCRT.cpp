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

// CKSDK initialization
namespace CKSDK
{
	// Initialize CKSDK
	void Init();
}

extern "C"
{
	// C++ start
	void main(void);

	KEEP void cksdk_start(void)
	{
		// Initialize CKSDK
		CKSDK::Init();

		// Call main
		main();
	}

	// GCC built-in new and delete
	KEEP void *__builtin_new(size_t size) { return CKSDK::Mem::Alloc(size); }
	KEEP void  __builtin_delete(void *ptr) { CKSDK::Mem::Free(ptr); }

	// Pure virtual call
	KEEP void __cxa_pure_virtual(void) { CKSDK::ExScreen::Abort("Pure virtual call"); }

	// memset
	KEEP void *memset(void *s, int c, size_t n)
	{
		unsigned char *p = (unsigned char*)s;
		while (n-- > 0)
			*p++ = (unsigned char)c;
		return s;
	}

	// memcpy
	KEEP void *memcpy(void *dest, const void *src, size_t n)
	{
		unsigned char *d = (unsigned char*)dest;
		const unsigned char *s = (const unsigned char*)src;
		while (n-- > 0)
			*d++ = *s++;
		return dest;
	}

	// memcmp
	KEEP int memcmp(const void *s1, const void *s2, size_t n)
	{
		const unsigned char *p1 = (const unsigned char*)s1;
		const unsigned char *p2 = (const unsigned char*)s2;

		while (n-- > 0)
		{
			if (*p1++ != *p2++)
				return p1[-1] < p2[-1] ? -1 : 1;
		}
		return 0;
	}

	// strcmp
	KEEP int strcmp(const char *s1, const char *s2)
	{
		const unsigned char *p1 = (const unsigned char *)s1;
		const unsigned char *p2 = (const unsigned char *)s2;

		while (*p1 && *p1 == *p2)
		{
			p1++;
			p2++;
		}

		return (*p1 > *p2) - (*p2 > *p1);
	}
}

// C++ new and delete
KEEP void *operator new(size_t size) noexcept { return CKSDK::Mem::Alloc(size); }
KEEP void *operator new[](size_t size) noexcept { return CKSDK::Mem::Alloc(size); }

KEEP void *operator new(size_t size, std::align_val_t align) noexcept { (void)align; return CKSDK::Mem::Alloc(size); }
KEEP void *operator new[](size_t size, std::align_val_t align) noexcept { (void)align; return CKSDK::Mem::Alloc(size); }

KEEP void operator delete(void *ptr) noexcept { CKSDK::Mem::Free(ptr); }
KEEP void operator delete[](void *ptr) noexcept { CKSDK::Mem::Free(ptr); }
KEEP void operator delete(void *ptr, size_t size) noexcept { (void)size; CKSDK::Mem::Free(ptr); }
KEEP void operator delete[](void *ptr, size_t size) noexcept { (void)size; CKSDK::Mem::Free(ptr); }