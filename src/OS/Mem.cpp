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

#include <CKSDK/Mem.h>

#include <CKSDK/OS.h>
#include <CKSDK/ExScreen.h>

#include <CKSDK/TTY.h>

namespace CKSDK
{
	namespace Mem
	{
		// Mem alignment
		static constexpr uintptr_t ALIGNMENT = 0x10;
		
		template<typename T>
		static T Align(T x) { return T((uintptr_t(x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1)); }
		
		template<typename T>
		static T AlignEnd(T x) { return T(uintptr_t(x) & ~(ALIGNMENT - 1)); }
		
		// Mem heap
		struct Header
		{
			Header *prev, *next;
			size_t size;
		};
		static Header *mem;

		// Mem functions
		void Init(void *ptr, size_t size)
		{
			// Initialize header
			mem = (Header*)Align(ptr);
			mem->prev = nullptr;
			mem->next = nullptr;
			mem->size = ((char*)ptr + size) - (char*)mem;
		}
		
		void *Alloc(size_t size)
		{
			// Align size
			size = Align(size) + Align(sizeof(Header));

			// Get header pointer
			Header *head, *prev, *next;
			char *hpos = (char*)mem + Align(sizeof(Header));
			
			prev = mem;
			next = prev->next;
			
			while (1)
			{
				if (next != nullptr)
				{
					// Check against the next block
					size_t cleft = (char*)next - hpos;
					if (cleft >= size)
					{
						// Set pointer
						head = (Header*)hpos;
						break;
					}
					
					// Check next header
					hpos = (char*)next + next->size;
					prev = next;
					next = prev->next;
				}
				else
				{
					// Check against end of heap
					size_t cleft = ((char*)mem + mem->size) - hpos;
					if (cleft < size)
					{
						ExScreen::Abort("Mem::Alloc failed");
						return nullptr;
					}
					
					// Set pointer
					head = (Header*)hpos;
					break;
				}
			}

			// Link header
			head->size = size;
			head->prev = prev;
			if ((head->next = prev->next) != nullptr)
				head->next->prev = head;
			prev->next = head;

			// Return pointer
			return (void*)(hpos + Align(sizeof(Header)));
		}

		void Free(void *ptr)
		{
			// Get header
			if (ptr == nullptr)
				return;
			Header *head = (Header*)((char*)ptr - Align(sizeof(Header)));

			// Unlink header
			if ((head->prev->next = head->next) != nullptr)
				head->next->prev = head->prev;
		}

		void Profile(size_t *used, size_t *total, size_t *blocks)
		{
			if (used != nullptr)
			{
				size_t u = 0;
				for (Header *head = mem->next; head != nullptr; head = head->next)
					u += head->size;
				*used = u;
			}
			if (total != nullptr)
				*total = mem->size - Align(sizeof(Header));
			if (blocks != nullptr)
			{
				size_t b = 0;
				for (Header *head = mem->next; head != nullptr; head = head->next)
					b++;
				*blocks = b;
			}
		}
	}
}
