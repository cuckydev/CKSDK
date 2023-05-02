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
		// Unique pointer
		/// @brief Unique pointer class
		template <typename T>
		class UniquePointer
		{
			private:
				T *m_ptr;
			
			public:
				// Constructor
				UniquePointer(T *ptr = nullptr) : m_ptr(ptr) {}

				// Destructor
				~UniquePointer()
				{
					delete m_ptr;
				}

				// Copy constructor is deleted to prevent copying of unique pointers
				UniquePointer(const UniquePointer&) = delete;

				// Copy assignment operator is also deleted
				UniquePointer& operator=(const UniquePointer&) = delete;

				// Move constructor
				UniquePointer(UniquePointer &&other) noexcept
				{
					m_ptr = other.m_ptr;
					other.m_ptr = nullptr;
				}

				// Move assignment operator
				UniquePointer &operator=(UniquePointer &&other) noexcept
				{
					if (this != &other)
					{
						delete m_ptr;
						m_ptr = other.m_ptr;
						other.m_ptr = nullptr;
					}
					return *this;
				}

				// Overload the dereference operator
				T& operator*() const
				{
					return *m_ptr;
				}

				// Overload the arrow operator
				T* operator->() const
				{
					return m_ptr;
				}

				// Release the ownership of the managed object and return a pointer to it
				T *Release()
				{
					T* tmp = m_ptr;
					m_ptr = nullptr;
					return tmp;
				}

				// Reset the unique pointer to manage a new object
				void Reset(T *ptr = nullptr)
				{
					if (m_ptr != ptr)
					{
						delete m_ptr;
						m_ptr = ptr;
					}
				}

				// Return the raw pointer without changing ownership
				T *Get() const
				{
					return m_ptr;
				}
		};
		
		/// @cond INTERNAL
		template <typename T>
		class UniquePointer<T[]>
		{
			private:
				T *m_ptr;
			
			public:
				// Constructor
				UniquePointer(T *ptr = nullptr) : m_ptr(ptr) {}

				// Destructor
				~UniquePointer()
				{
					delete[] m_ptr;
				}

				// Copy constructor is deleted to prevent copying of unique pointers
				UniquePointer(const UniquePointer&) = delete;

				// Copy assignment operator is also deleted
				UniquePointer& operator=(const UniquePointer&) = delete;

				// Move constructor
				UniquePointer(UniquePointer &&other) noexcept
				{
					m_ptr = other.m_ptr;
					other.m_ptr = nullptr;
				}

				// Move assignment operator
				UniquePointer &operator=(UniquePointer &&other) noexcept
				{
					if (this != &other)
					{
						delete[] m_ptr;
						m_ptr = other.m_ptr;
						other.m_ptr = nullptr;
					}
					return *this;
				}

				// Overload the dereference operator
				T& operator*() const
				{
					return *m_ptr;
				}

				// Overload the arrow operator
				T* operator->() const
				{
					return m_ptr;
				}

				// Release the ownership of the managed object and return a pointer to it
				T *Release()
				{
					T* tmp = m_ptr;
					m_ptr = nullptr;
					return tmp;
				}

				// Reset the unique pointer to manage a new object
				void Reset(T *ptr = nullptr)
				{
					if (m_ptr != ptr)
					{
						delete[] m_ptr;
						m_ptr = ptr;
					}
				}

				// Return the raw pointer without changing ownership
				T *Get() const
				{
					return m_ptr;
				}
		};
		/// @endcond

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
				const uint8_t *p1 = (const uint8_t*)s1;
				const uint8_t *p2 = (const uint8_t*)s2;
				for (; *p1 != '\0'; p1++, p2++)
				{
					uint8_t u1 = *p1;
					uint8_t u2 = *p2;
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
				const uint8_t *p1 = (const uint8_t*)s1;
				const uint8_t *p2 = (const uint8_t*)s2;
				for (; n--; p1++, p2++)
				{
					uint8_t u1 = *p1;
					uint8_t u2 = *p2;
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
				uint8_t *pd = (uint8_t*)d;
				const uint8_t *ps = (const uint8_t*)s;
				for (; n--; pd++, ps++)
					*pd = *ps;
			}
		}
	}
}
