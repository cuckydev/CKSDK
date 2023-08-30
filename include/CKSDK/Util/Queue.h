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
	
	- Queue.h -
	Queue for OS operations
*/

/// @file CKSDK/Util/Queue.h
/// @brief CKSDK queue utility

#pragma once

#include <CKSDK/CKSDK.h>

#include <CKSDK/OS.h>
#include <CKSDK/ExScreen.h>
#include <CKSDK/TTY.h>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK queue namespace
	namespace Queue
	{
		// Queue types
		/// @brief Queue template
		/// @tparam T Queue entry arguments type
		/// @tparam N Queue size
		/// @tparam W Wait for queue to empty when overflowing
		template <typename T, unsigned N, bool W = false>
		class Queue
		{
			private:
				// Queue entries
				using FT = void(*)(const T&);
				struct QueueEntry
				{
					FT func;
					T args;
				} queue[N];
				uint32_t queue_head = 0, queue_tail = 0, queue_length = 0;

			public:
				// Queue functions
				/// @brief Dispatch queue head
				/// @return True if queue is empty
				bool Dispatch()
				{
					// Check if queue is empty
					uint32_t length = queue_length;
					if (length == 0)
						return true;

					// Decrement queue length and check if we have another entry to run
					queue_length = --length;
					if (length != 0)
					{
						// Call next entry in queue
						uint32_t head = queue_head;
						QueueEntry *entry = &queue[head];
						queue_head = (head + 1) % N;

						entry->func(entry->args);
						return false;
					}
					return true;
				}

				/// @brief Enqueue function
				/// @param func Function to enqueue
				/// @param args Queue entry arguments
				void Enqueue(FT func, const T &args)
				{
					OS::DisableIRQ();

					uint32_t length = queue_length;
					if (length == 0)
					{
						queue_length = 1;
						func(args);
						OS::EnableIRQ();
						return;
					}
					if (length >= N)
					{
						OS::EnableIRQ();
						if (W)
							while (length >= N) INLINE_ASM("");
						else
							ExScreen::Abort("Queue overflow\n");
						return;
					}

					uint32_t tail = queue_tail;
					queue_tail    = (tail + 1) % N;
					queue_length  = length + 1;

					QueueEntry *entry = &queue[tail];
					entry->func = func;
					entry->args = args;

					OS::EnableIRQ();
				}

				/// @brief Wait for queue to empty
				void Sync()
				{
					// Wait for queue to clear up
					while (queue_length != 0);
				}

				/// @brief Reset queue
				void Reset()
				{
					queue_length = 0;
				}
		};
	}
}
