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

#include <EASTL/allocator.h>

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

// EASTL allocator
namespace eastl
{
	KEEP inline allocator::allocator(const char *EASTL_NAME(pName))
	{
		#if EASTL_NAME_ENABLED
		mpName = pName ? pName : EASTL_ALLOCATOR_DEFAULT_NAME;
		#endif
	}

	KEEP inline allocator::allocator(const allocator &EASTL_NAME(alloc))
	{
		#if EASTL_NAME_ENABLED
		mpName = alloc.mpName;
		#endif
	}

	KEEP inline allocator::allocator(const allocator &, const char *EASTL_NAME(pName))
	{
		#if EASTL_NAME_ENABLED
		mpName = pName ? pName : EASTL_ALLOCATOR_DEFAULT_NAME;
		#endif
	}


	KEEP inline allocator &allocator::operator=(const allocator &EASTL_NAME(alloc))
	{
		#if EASTL_NAME_ENABLED
		mpName = alloc.mpName;
		#endif
		return *this;
	}

	KEEP inline const char *allocator::get_name() const
	{
		#if EASTL_NAME_ENABLED
		return mpName;
		#else
		return EASTL_ALLOCATOR_DEFAULT_NAME;
		#endif
	}

	KEEP inline void allocator::set_name(const char *EASTL_NAME(pName))
	{
		#if EASTL_NAME_ENABLED
		mpName = pName;
		#endif
	}

	KEEP inline void *allocator::allocate(size_t n, int flags)
	{
		return CKSDK::Mem::Alloc(n);
	}

	KEEP inline void *allocator::allocate(size_t n, size_t alignment, size_t offset, int flags)
	{
		return CKSDK::Mem::Alloc(n);
	}

	KEEP inline void allocator::deallocate(void *p, size_t)
	{
		CKSDK::Mem::Free(p);
	}

	KEEP inline bool operator==(const allocator &, const allocator &)
	{
		return true; // All allocators are considered equal, as they merely use global new/delete.
	}

	#if !defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	KEEP inline bool operator!=(const allocator &, const allocator &)
	{
		return false; // All allocators are considered equal, as they merely use global new/delete.
	}
	#endif
} // namespace eastl
