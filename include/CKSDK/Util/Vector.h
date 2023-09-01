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

/// @file CKSDK/Util/Vector.h
/// @brief CKSDK vector container

#pragma once

#include <CKSDK/CKSDK.h>

#include <CKSDK/Mem.h>

#include <iterator>
#include <type_traits>
#include <initializer_list>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief Vector container
	/// @tparam T Type of vector
	template <typename T>
	class Vector
	{
		private:
			// Require that T is copy assignable and copy constructible
			static_assert(std::is_copy_assignable_v<T>, "Vector type must be copy assignable");
			static_assert(std::is_copy_constructible_v<T>, "Vector type must be copy constructible");

			// Vector data
			T *m_data;
			size_t m_size;

			size_t m_capacity;

			// Get ideal capacity for size
			static size_t GetIdealCapacity(size_t size)
			{
				// Get ideal capacity
				size_t capacity = 256 / sizeof(T); // 256 bytes is a good minimum
				if (capacity == 0) capacity = 1;
				while (capacity < size)
					capacity <<= 1;
				return capacity;
			}

		public:
			// Iterator types
			using iterator = T*;
			using const_iterator = const T*;
			using reverse_iterator = std::reverse_iterator<iterator>;
			using const_reverse_iterator = std::reverse_iterator<const_iterator>;

			// Constructors
			/// @brief Default constructor
			Vector()
			{
				m_data = nullptr;
				m_size = 0;
				m_capacity = 0;
			}

			/// @brief Constructor with initial size and value
			/// @param size Initial size
			/// @param value Initial value
			Vector(size_t size, const T &value)
			{
				m_capacity = GetIdealCapacity(size);
				m_data = (T*)CKSDK::Mem::Alloc(m_capacity * sizeof(T));
				m_size = size;

				for (size_t i = 0; i < size; i++)
					new (&m_data[i]) T(value);
			}

			/// @brief Constructor with initial size
			/// @param size Initial size
			explicit Vector(size_t size)
			{
				m_capacity = GetIdealCapacity(size);
				m_data = (T*)CKSDK::Mem::Alloc(m_capacity * sizeof(T));
				m_size = size;

				for (size_t i = 0; i < size; i++)
					new (&m_data[i]) T();
			}

			/// @brief Copy constructor
			/// @param other Vector to copy
			Vector(const Vector<T> &other) = default;

			/// @brief Copy assignment operator
			/// @param other Vector to copy
			/// @return Reference to this vector
			Vector<T> &operator=(const Vector<T> &other)
			{
				// Free old data
				if (m_data != nullptr)
				{
					for (size_t i = 0; i < m_size; i++)
						m_data[i].~T();
					CKSDK::Mem::Free(m_data);
				}

				// Copy new data
				m_data = (T *)CKSDK::Mem::Alloc(other.m_capacity * sizeof(T));
				m_size = other.m_size;
				m_capacity = other.m_capacity;

				for (size_t i = 0; i < other.m_size; i++)
					new (&m_data[i]) T(other.m_data[i]);
			}

			/// @brief Move constructor
			/// @param other Vector to move
			Vector(Vector<T> &&other)
			{
				m_data = other.m_data;
				m_size = other.m_size;
				m_capacity = other.m_capacity;

				other.m_data = nullptr;
				other.m_size = 0;
				other.m_capacity = 0;
			}

			/// @brief Destructor
			~Vector()
			{
				// Free data
				if (m_data != nullptr)
				{
					for (size_t i = 0; i < m_size; i++)
						m_data[i].~T();
					CKSDK::Mem::Free(m_data);
				}
			}

			/// @brief Array subscript operator
			/// @param index Index to access
			T &operator[](int index)
			{
				return m_data[index];
			}

			/// @brief Array subscript operator
			/// @param index Index to access
			const T &operator[](int index) const
			{
				return m_data[index];
			}

			/// @brief Get element at index
			/// @param index Index to access
			/// @return Reference to element
			T &At(int index)
			{
				if (index < 0 || index >= m_size) CKSDK::ExScreen::Abort("Vector::At index out of range");
				return m_data[index];
			}

			/// @brief Get element at index
			/// @param index Index to access
			/// @return Reference to element
			const T &At(int index) const
			{
				if (index < 0 || index >= m_size) CKSDK::ExScreen::Abort("Vector::At index out of range");
				return m_data[index];
			}

			/// @brief Get iterator to beginning
			/// @return Iterator to beginning
			iterator begin()
			{
				return iterator(m_data);
			}

			/// @brief Get const iterator to beginning
			/// @return Const iterator to beginning
			const_iterator begin() const
			{
				return const_iterator(m_data);
			}

			/// @brief Get const iterator to beginning
			/// @return Const iterator to beginning
			const_iterator cbegin() const
			{
				return const_iterator(m_data);
			}

			/// @brief Get iterator to end
			/// @return Iterator to end
			iterator end()
			{
				return iterator(m_data + m_size);
			}

			/// @brief Get const iterator to end
			/// @return Const iterator to end
			const_iterator end() const
			{
				return const_iterator(m_data + m_size);
			}

			/// @brief Get const iterator to end
			/// @return Const iterator to end
			const_iterator cend() const
			{
				return const_iterator(m_data + m_size);
			}

			/// @brief Get reverse iterator to beginning
			/// @return Reverse iterator to beginning
			reverse_iterator rbegin()
			{
				return reverse_iterator(m_data + m_size - 1);
			}

			/// @brief Get const reverse iterator to beginning
			/// @return Const reverse iterator to beginning
			const_reverse_iterator rbegin() const
			{
				return const_reverse_iterator(m_data + m_size - 1);
			}

			/// @brief Get const reverse iterator to beginning
			/// @return Const reverse iterator to beginning
			const_reverse_iterator crbegin() const
			{
				return const_reverse_iterator(m_data + m_size - 1);
			}

			/// @brief Get reverse iterator to end
			/// @return Reverse iterator to end
			reverse_iterator rend()
			{
				return reverse_iterator(m_data - 1);
			}

			/// @brief Get const reverse iterator to end
			/// @return Const reverse iterator to end
			const_reverse_iterator rend() const
			{
				return const_reverse_iterator(m_data - 1);
			}

			/// @brief Get const reverse iterator to end
			/// @return Const reverse iterator to end
			const_reverse_iterator crend() const
			{
				return const_reverse_iterator(m_data - 1);
			}

			/// @brief Check if vector is empty
			/// @return True if vector is empty
			bool Empty() const
			{
				return m_size == 0;
			}

			/// @brief Get pointer to data
			/// @return Pointer to data
			T *Data()
			{
				return m_data;
			}

			/// @brief Get pointer to data
			/// @return Pointer to data
			const T *Data() const
			{
				return m_data;
			}

			/// @brief Access the first element
			/// @return Reference to first element
			/// @note Vector must not be empty
			T &Front()
			{
				return m_data[0];
			}

			/// @brief Access the first element
			/// @return Reference to first element
			/// @note Vector must not be empty
			const T &Front() const
			{
				return m_data[0];
			}

			/// @brief Access the last element
			/// @return Reference to last element
			/// @note Vector must not be empty
			T &Back()
			{
				return m_data[m_size - 1];
			}

			/// @brief Access the last element
			/// @return Reference to last element
			/// @note Vector must not be empty
			const T &Back() const
			{
				return m_data[m_size - 1];
			}

			/// @brief Get size of vector
			/// @return Size of vector
			size_t Size() const
			{
				return m_size;
			}

			/// @brief Get capacity of vector
			/// @return Capacity of vector
			size_t Capacity() const
			{
				return m_capacity;
			}

			/// @brief Reserve vector capacity
			/// @param capacity Capacity to reserve
			void Reserve(size_t new_cap)
			{
				new_cap = GetIdealCapacity(new_cap);
				if (new_cap > m_capacity)
				{
					// Reallocate data
					m_data = (T*)CKSDK::Mem::Realloc(m_data, new_cap * sizeof(T));
					m_capacity = new_cap;
				}
			}

			/// @brief Resize vector
			/// @param size New size
			void Resize(size_t count)
			{
				// Reserve new capacity
				Reserve(count);

				// Destroy or construct elements
				if (count < m_size)
				{
					for (size_t i = count; i < m_size; i++)
						m_data[i].~T();
					m_size = count;
				}
				else if (count > m_size)
				{
					for (size_t i = m_size; i < count; i++)
						new (&m_data[i]) T();
					m_size = count;
				}
			}

			/// @brief Resize vector
			/// @param size New size
			/// @param value Value to fill new elements with
			void Resize(size_t count, const T &value)
			{
				// Reserve new capacity
				Reserve(count);

				// Destroy or construct elements
				if (count < m_size)
				{
					for (size_t i = count; i < m_size; i++)
						m_data[i].~T();
					m_size = count;
				}
				else if (count > m_size)
				{
					for (size_t i = m_size; i < count; i++)
						new (&m_data[i]) T(value);
					m_size = count;
				}
			}

			/// @brief Clear vector
			void Clear()
			{
				// Destroy elements
				for (size_t i = 0; i < m_size; i++)
					m_data[i].~T();
				m_size = 0;
			}

			/// @brief Shrink vector to fit
			void ShrinkToFit()
			{
				// Reallocate data
				m_capacity = GetIdealCapacity(m_size);
				m_data = (T*)CKSDK::Mem::Realloc(m_data, m_capacity * sizeof(T));
			}

			/// @brief Insert element
			/// @param pos Position to insert at
			/// @param value Value to insert
			/// @return Iterator to position of first inserted element
			iterator Insert(const_iterator pos, const T &value)
			{
				// Reserve new capacity
				Reserve(++m_size);

				// Shift elements forward
				size_t index = pos - m_data;
				for (size_t i = m_size; i >= index + 1; i--)
					m_data[i] = m_data[i - 1];
				m_data[index] = value;

				return m_data + index;
			}

			/// @brief Insert element
			/// @param pos Position to insert at
			/// @param value Value to insert
			/// @return Iterator to position of first inserted element
			iterator Insert(const_iterator pos, T &&value)
			{
				// Reserve new capacity
				Reserve(++m_size);

				// Shift elements forward
				size_t index = pos - m_data;
				for (size_t i = m_size - 1; i >= index + 1; i--)
					m_data[i] = m_data[i - 1];
				m_data[index] = std::move(value);

				return m_data + index;
			}

			/// @brief Insert elements
			/// @param pos Position to insert at
			/// @param count Number of elements to insert
			/// @param value Value to insert
			/// @return Iterator to position of first inserted element
			iterator Insert(const_iterator pos, size_t count, const T &value)
			{
				// If no elements to insert, return
				if (count == 0)
					return pos;

				// Reserve new capacity
				Reserve(m_size += count);

				// Shift elements forward
				size_t index = pos - m_data;
				for (size_t i = m_size - 1; i >= index + count; i--)
					m_data[i] = m_data[i - count];
				for (size_t i = index; i < index + count; i++)
					m_data[i] = value;

				return m_data + index;
			}

			/// @brief Insert elements
			/// @param pos Position to insert at
			/// @param first Iterator to first element to insert
			/// @param last Iterator to last element to insert
			/// @return Iterator to position of first inserted element
			template <typename InputIt>
			iterator Insert(const_iterator pos, InputIt first, InputIt last)
			{
				// If no elements to insert, return
				if (first == last)
					return pos;

				// Reserve new capacity
				size_t count = last - first;
				Reserve(m_size += count);

				// Shift elements forward
				size_t index = pos - m_data;
				for (size_t i = m_size - 1; i >= index + count; i--)
					m_data[i] = m_data[i - count];
				for (size_t i = index; i < index + count; i++)
					m_data[i] = *first++;

				return m_data + index;
			}

			/// @brief Insert elements
			/// @param pos Position to insert at
			/// @param ilist List to insert
			/// @return Iterator to position of first inserted element
			iterator Insert(const_iterator pos, std::initializer_list<T> ilist)
			{
				// If no elements to insert, return
				if (ilist.size() == 0)
					return pos;

				// Reserve new capacity
				size_t count = ilist.size();
				Reserve(m_size += count);

				// Shift elements forward
				size_t index = pos - m_data;
				for (size_t i = m_size - 1; i >= index + count; i--)
					m_data[i] = m_data[i - count];
				for (size_t i = index; i < index + count; i++)
					m_data[i] = ilist.begin()[i - index];

				return m_data + index;
			}

			/// @brief Emplace element
			/// @param pos Position to insert at
			/// @param args Arguments to construct element with
			/// @return Iterator to inserted element
			template <typename... Args>
			iterator Emplace(const_iterator pos, Args&&... args)
			{
				// Reserve new capacity
				Reserve(++m_size);

				// Shift elements forward
				size_t index = pos - m_data;
				for (size_t i = m_size - 1; i >= index + 1; i--)
					m_data[i] = m_data[i - 1];
				new (&m_data[index]) T(std::forward<Args>(args)...);

				return m_data + index;
			}

			/// @brief Erase element
			/// @param pos Position to erase at
			/// @return Iterator to element after last erased element
			iterator Erase(const_iterator pos)
			{
				// Check if pos is at end
				if (pos == end())
					return end();
				
				// Shift elements backward
				size_t index = pos - m_data;
				for (size_t i = index; i < m_size - 1; i++)
					m_data[i] = m_data[i + 1];
				m_size--;

				return m_data + index;
			}

			/// @brief Erase elements
			/// @param first Iterator to first element to erase
			/// @param last Iterator to last element to erase
			/// @return Iterator to element after last erased element
			iterator Erase(iterator first, iterator last)
			{
				// If no elements to erase, return
				if (first == last)
					return last;

				// Shift elements backward
				size_t index = first - m_data;
				size_t count = last - first;
				for (size_t i = index; i < m_size - count; i++)
					m_data[i] = m_data[i + count];
				m_size -= count;

				return m_data + index;
			}

			/// @brief Erase elements
			/// @param first Iterator to first element to erase
			/// @param last Iterator to last element to erase
			/// @return Iterator to element after last erased element
			iterator Erase(const_iterator first, const_iterator last)
			{
				// If no elements to erase, return
				if (first == last)
					return last;

				// Shift elements backward
				size_t index = first - m_data;
				size_t count = last - first;
				for (size_t i = index; i < m_size - count; i++)
					m_data[i] = m_data[i + count];
				m_size -= count;

				return m_data + index;
			}

			/// @brief Push value to back
			/// @param value Value to push
			void PushBack(const T &value)
			{
				// Reserve new capacity
				Reserve(++m_size);

				// Push value to back
				m_data[m_size - 1] = value;
			}

			/// @brief Push value to back
			/// @param value Value to push
			void PushBack(T &&value)
			{
				// Reserve new capacity
				Reserve(++m_size);

				// Push value to back
				m_data[m_size - 1] = std::move(value);
			}

			/// @brief Emplace value to back
			/// @param args Arguments to construct value with
			template <typename... Args>
			void EmplaceBack(Args&&... args)
			{
				// Reserve new capacity
				Reserve(++m_size);

				// Emplace value to back
				new (&m_data[m_size - 1]) T(std::forward<Args>(args)...);
			}

			/// @brief Pop value from back
			void PopBack()
			{
				// Check if empty
				if (m_size == 0)
					return;

				// Pop value from back
				m_size--;
			}
	};
}