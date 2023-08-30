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
		static constexpr uintptr_t ALIGNMENT = 8;
		
		template<typename T>
		static constexpr T Align(T x) { return T(((uintptr_t)x + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1)); }
		
		template<typename T>
		static constexpr T AlignEnd(T x) { return T((uintptr_t)x & ~(ALIGNMENT - 1)); }
		
		// Mem heap
		struct Block
		{
			Block *prev, *next;
			size_t size;
		};
		static Block *mem;

		// Mem block search
		static Block *Search(size_t size, Block **const out_prev)
		{
			// Get block pointer
			Block *head, *prev, *next;
			char *hpos = (char*)mem + Align(sizeof(Block));

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
						head = (Block*)hpos;
						break;
					}

					// Check next block
					hpos = (char*)next + next->size;
					prev = next;
					next = prev->next;
				}
				else
				{
					// Check against end of heap
					size_t cleft = ((char*)mem + mem->size) - hpos;
					if (cleft < size)
						return nullptr;

					// Set pointer
					head = (Block*)hpos;
					break;
				}
			}

			// Return pointers
			*out_prev = prev;
			return head;
		}

		// Mem functions
		KEEP void Init(void *ptr, size_t size)
		{
			// Initialize block
			mem = (Block*)Align(ptr);
			mem->prev = nullptr;
			mem->next = nullptr;
			mem->size = AlignEnd(((uintptr_t)ptr + size) - (uintptr_t)mem);
		}
		
		KEEP void *Alloc(size_t size)
		{
			// Align size
			size = Align(size) + Align(sizeof(Block));

			// Search for free block
			Block *head, *prev;
			head = Search(size, &prev);
			if (head == nullptr)
				return nullptr;

			// Link block
			head->size = size;
			head->prev = prev;
			if ((head->next = prev->next) != nullptr)
				head->next->prev = head;
			prev->next = head;

			// Return pointer
			return (void*)((char*)head + Align(sizeof(Block)));
		}

		KEEP void *Realloc(void *ptr, size_t size)
		{
			// Get block
			if (ptr == nullptr)
				return nullptr;
			Block *head = (Block*)((char*)ptr - Align(sizeof(Block)));

			// Unlink block
			if ((head->prev->next = head->next) != nullptr)
				head->next->prev = head->prev;

			// Align size
			size = Align(size) + Align(sizeof(Block));

			// Search for free block
			Block *newhead, *newprev;
			newhead = Search(size, &newprev);
			if (newhead == nullptr)
				return nullptr;

			// Copy data over
			if (head->size > size)
				__builtin_memcpy((char*)ptr, (char*)newhead + Align(sizeof(Block)), size - Align(sizeof(Block)));
			else
				__builtin_memcpy((char*)ptr, (char*)newhead + Align(sizeof(Block)), head->size - Align(sizeof(Block)));

			// Link block
			head->size = size;
			head->prev = newprev;
			if ((head->next = newprev->next) != nullptr)
				head->next->prev = head;
			newprev->next = head;

			return head;
		}

		KEEP void Free(void *ptr)
		{
			// Get block
			if (ptr == nullptr)
				return;
			Block *head = (Block*)((char*)ptr - Align(sizeof(Block)));

			// Unlink block
			if ((head->prev->next = head->next) != nullptr)
				head->next->prev = head->prev;
		}

		KEEP void Profile(size_t *used, size_t *total, size_t *blocks)
		{
			if (used != nullptr)
			{
				size_t u = 0;
				for (Block *head = mem->next; head != nullptr; head = head->next)
					u += head->size;
				*used = u;
			}
			if (total != nullptr)
				*total = mem->size - Align(sizeof(Block));
			if (blocks != nullptr)
			{
				size_t b = 0;
				for (Block *head = mem->next; head != nullptr; head = head->next)
					b++;
				*blocks = b;
			}
		}
	}
}
