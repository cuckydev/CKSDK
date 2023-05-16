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

/// @file CKSDK/Util/Fixed.h
/// @brief CKSDK fixed point utility

#pragma once

#include <CKSDK/CKSDK.h>

#include <type_traits>
#include <limits>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK fixed point namespace
	namespace Fixed
	{
		// Integer type extension for multiplication
		template <typename T>
		struct extend_integral_type {
			using type = T;
		};

		template <>
		struct extend_integral_type<char> {
			using type = short;
		};

		template <>
		struct extend_integral_type<short> {
			using type = int;
		};

		template <>
		struct extend_integral_type<int> {
			using type = long;
		};

		template <>
		struct extend_integral_type<long> {
			using type = long long;
		};

		template <>
		struct extend_integral_type<long long> {
			using type = long long; // No standard integral type larger than long long.
		};

		template <typename T>
		using extend_integral_t = typename extend_integral_type<T>::type;

		// Fixed point types
		/// @brief Fixed point class template
		/// @tparam T Fixed point type
		/// @tparam FRAC Fixed point fractional bits
		template <typename T, int FRAC>
		class Fixed
		{
			private:
				// Raw value
				T x;

			public:
				/// @brief Fixed point type
				using Type = T;
				static constexpr int Frac = FRAC;

				// Constructors
				/// @brief Default constructor
				constexpr Fixed() noexcept {}

				/// @brief Copy constructor
				/// @param _x Numerical value
				template<typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed(U _x) noexcept
				{
					if constexpr (std::is_integral_v<U>)
						x = T(_x << FRAC);
					else if constexpr (std::is_floating_point_v<U>)
						x = T(_x * (T(1) << FRAC));
				}

				/// @brief Copy constructor
				/// @tparam T2 Other fixed point type
				/// @tparam FRAC2 Other fixed point fractional bits
				/// @param rhs Other fixed point value
				template<typename T2, int FRAC2>
				constexpr Fixed(Fixed<T2, FRAC2> const &rhs) noexcept : x(*((T2*)&rhs))
				{ 
					if constexpr (FRAC2 > FRAC)
						x >>= FRAC2 - FRAC;
					else if constexpr (FRAC > FRAC2)
						x <<= FRAC - FRAC2;
				}

				/// @brief Assignment operator
				/// @tparam T2 Other fixed point type
				/// @tparam FRAC2 Other fixed point fractional bits
				/// @param rhs Other fixed point value
				template<typename T2, int FRAC2>
				Fixed<T, FRAC> &operator=(Fixed<T2, FRAC2> const &rhs)
				{
					if constexpr (FRAC2 > FRAC)
						x = *((T2*)&rhs) >> (FRAC2 - FRAC);
					else if constexpr (FRAC > FRAC2)
						x = *((T2*)&rhs) << (FRAC - FRAC2);
					else
						x = *((T2*)&rhs);
					return *this;
				}

				/// @brief Assignment operator
				/// @tparam U Other numerical type
				/// @param _x Other numerical value
				template<typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				Fixed<T, FRAC> &operator=(U _x)
				{
					if constexpr (std::is_integral_v<U>)
						x = T(_x << FRAC);
					else if constexpr (std::is_floating_point_v<U>)
						x = T(_x * (T(1) << FRAC));
					return *this;
				}

				// Functions
				/// @brief Get floored integer value
				/// @return Floored integer value
				constexpr Fixed<T, FRAC> Floor() const
				{ return Raw(x & ~((1 << FRAC) - 1)); }
				/// @brief Get ceiled integer value
				/// @return Ceiled integer value
				constexpr Fixed<T, FRAC> Ceil() const
				{ return Raw((x + ((1 << FRAC) - 1)) & ~((1 << FRAC) - 1)); }
				/// @brief Get rounded integer value
				/// @return Rounded integer value
				constexpr Fixed<T, FRAC> Round() const
				{ return Raw((x + ((1 << FRAC) >> 1)) & ~((1 << FRAC) - 1)); }
				/// @brief Get raw value
				/// @return Raw value
				constexpr T Raw() const
				{ return x; }
				/// @brief Construct from raw value
				/// @param _x Raw value
				/// @return Fixed point value
				constexpr static Fixed<T, FRAC> Raw(T _x)
				{ Fixed<T, FRAC> result; result.x = _x; return result; }

				/// @brief Get lowest representable value
				/// @return Lowest representable value
				static constexpr Fixed<T, FRAC> Min()
				{ return Raw(std::numeric_limits<T>::min()); }
				/// @brief Get highest representable value
				/// @return Highest representable value
				static constexpr Fixed<T, FRAC> Max()
				{ return Raw(std::numeric_limits<T>::max()); }

				// Operators
				// Fixed + Fixed
				constexpr Fixed<T, FRAC> operator+(const Fixed<T, FRAC> &_x) const
				{ return Raw(this->x + _x.x); }
				// Fixed += Fixed
				void operator+=(const Fixed<T, FRAC> &_x)
				{ *this = *this + _x; }

				// Fixed + number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> operator+(const U &_x) const
				{ return Raw(this->x + Fixed<T, FRAC>(_x)); }
				// Fixed += number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				void operator+=(const U &_x)
				{ *this = *this + _x; }
				
				// Fixed - Fixed
				constexpr Fixed<T, FRAC> operator-(const Fixed<T, FRAC> &_x) const
				{ return Raw(this->x - _x.x); }
				// Fixed -= Fixed
				void operator-=(const Fixed<T, FRAC> &_x)
				{ *this = *this - _x; }

				// Fixed - number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> operator-(const U &_x) const
				{ return Raw(this->x - Fixed<T, FRAC>(_x)); }
				// Fixed -= number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				void operator-=(const U &_x)
				{ *this = *this - _x; }
				
				// +Fixed
				constexpr Fixed<T, FRAC> operator+() const
				{ return *this; }
				// -Fixed
				constexpr Fixed<T, FRAC> operator-() const
				{ return Raw(-this->x); }

				// Fixed * Fixed
				constexpr Fixed<T, FRAC> operator*(const Fixed<T, FRAC> &_x) const
				{ return Raw(((extend_integral_t<T>)this->x * _x.x) >> FRAC); }
				// Fixed *= Fixed
				void operator*=(const Fixed<T, FRAC> &_x)
				{ *this = *this * _x; }

				// Fixed * number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> operator*(U _x) const
				{
					if constexpr (std::is_integral_v<U>)
						return Raw(this->x * _x);
					else if constexpr (std::is_floating_point_v<U>)
						return *this * Fixed<T, FRAC>(_x);
				}
				// Fixed *= number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				void operator*=(U _x)
				{ *this = *this * _x; }

				// Fixed / Fixed
				constexpr Fixed<T, FRAC> operator/(const Fixed<T, FRAC> &_x) const
				{ return Raw(((extend_integral_t<T>)this->x << FRAC) / _x.x); }
				// Fixed /= Fixed
				void operator/=(const Fixed<T, FRAC> &_x)
				{ *this = *this * _x; }

				// Fixed / number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> operator/(U _x) const
				{
					if constexpr (std::is_integral_v<U>)
						return Raw(this->x / _x);
					else if constexpr (std::is_floating_point_v<U>)
						return *this / Fixed<T, FRAC>(_x);
				}
				// Fixed /= number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				void operator/=(U _x)
				{ *this = *this / _x; }

				// Fixed < Fixed
				bool operator<(const Fixed<T, FRAC> &_x) const
				{ return x < _x.x; }
				// Fixed < number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				bool operator<(U _x) const
				{ return x < Fixed<T, FRAC>(_x).x; }

				// Fixed > Fixed
				bool operator>(const Fixed<T, FRAC> &_x) const
				{ return x > _x.x; }
				// Fixed > number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				bool operator>(U _x) const
				{ return x > Fixed<T, FRAC>(_x).x; }

				// Fixed <= Fixed
				bool operator<=(const Fixed<T, FRAC> &_x) const
				{ return x <= _x.x; }
				// Fixed <= number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				bool operator<=(U _x) const
				{ return x <= Fixed<T, FRAC>(_x).x; }

				// Fixed >= Fixed
				bool operator>=(const Fixed<T, FRAC> &_x) const
				{ return x >= _x.x; }
				// Fixed >= number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				bool operator>=(U _x) const
				{ return x >= Fixed<T, FRAC>(_x).x; }

				// Fixed == Fixed
				bool operator==(const Fixed<T, FRAC> &_x) const
				{ return x == _x.x; }
				// Fixed == number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				bool operator==(U _x) const
				{ return x == Fixed<T, FRAC>(_x).x; }

				// Fixed != Fixed
				bool operator!=(const Fixed<T, FRAC> &_x) const
				{ return x != _x.x; }
				// Fixed != number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				bool operator!=(U _x) const
				{ return x != Fixed<T, FRAC>(_x).x; }

				// (number)Fixed
				template <typename U, typename = typename std::enable_if_t<std::is_integral_v<U>, U>>
				operator U() const
				{ return (U)(x >> FRAC); }
		};
	}
}
