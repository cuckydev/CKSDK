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
		// Fixed point types
		/// @brief Fixed point class template
		/// @tparam T Fixed point type
		/// @tparam FRAC Fixed point fractional bits
		template <typename T, int FRAC>
		class Fixed
		{
			private:
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
				{ Fixed<T, FRAC> result; result.x = x & ~((1 << FRAC) - 1); return result; }
				/// @brief Get ceiled integer value
				/// @return Ceiled integer value
				constexpr Fixed<T, FRAC> Ceil() const
				{ Fixed<T, FRAC> result; result.x = (x + ((1 << FRAC) - 1)) & ~((1 << FRAC) - 1); return result; }
				/// @brief Get rounded integer value
				/// @return Rounded integer value
				constexpr Fixed<T, FRAC> Round() const
				{ Fixed<T, FRAC> result; result.x = (x + ((1 << FRAC) >> 1)) & ~((1 << FRAC) - 1); return result; }
				/// @brief Get raw value
				/// @return Raw value
				constexpr T Raw() const
				{ return x; }

				/// @brief Get lowest representable value
				/// @return Lowest representable value
				static constexpr Fixed<T, FRAC> Min()
				{ Fixed<T, FRAC> result; result.x = std::numeric_limits<T>::min(); return result; }
				/// @brief Get highest representable value
				/// @return Highest representable value
				static constexpr Fixed<T, FRAC> Max()
				{ Fixed<T, FRAC> result; result.x = std::numeric_limits<T>::max(); return result; }

				// Operators
				// Fixed + Fixed
				constexpr Fixed<T, FRAC> operator+(const Fixed<T, FRAC> &_x) const
				{ Fixed<T, FRAC> result; result.x = this->x + _x.x; return result; }
				// Fixed += Fixed
				void operator+=(const Fixed<T, FRAC> &_x)
				{ *this = *this + _x; }

				// Fixed + number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> operator+(const U &_x) const
				{ Fixed<T, FRAC> result; result.x = this->x + (T(_x) << FRAC); return result; }
				// Fixed += number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				void operator+=(const U &_x)
				{ *this = *this + _x; }
				
				// Fixed - Fixed
				constexpr Fixed<T, FRAC> operator-(const Fixed<T, FRAC> &_x) const
				{ Fixed<T, FRAC> result; result.x = this->x - _x.x; return result; }
				// Fixed -= Fixed
				void operator-=(const Fixed<T, FRAC> &_x)
				{ *this = *this - _x; }

				// Fixed - number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> operator-(const U &_x) const
				{ Fixed<T, FRAC> result; result.x = this->x - (T(_x) << FRAC); return result; }
				// Fixed -= number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				void operator-=(const U &_x)
				{ *this = *this - _x; }
				
				// +Fixed
				constexpr Fixed<T, FRAC> operator+() const
				{ return *this; }
				// -Fixed
				constexpr Fixed<T, FRAC> operator-() const
				{ Fixed<T, FRAC> result; result.x = -this->x; return result; }

				// Fixed * Fixed
				constexpr Fixed<T, FRAC> operator*(const Fixed<T, FRAC> &_x) const
				{ Fixed<T, FRAC> result; result.x = (this->x * _x.x) >> FRAC; return result; }
				// Fixed *= Fixed
				Fixed<T, FRAC> operator*=(const Fixed<T, FRAC> &_x)
				{ *this = *this * _x; }

				// Fixed * number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> operator*(U _x) const
				{ Fixed<T, FRAC> result; result.x = this->x * _x; return result; }
				// Fixed *= number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				Fixed<T, FRAC> operator*=(U _x)
				{ *this = *this * _x; }

				// Fixed / Fixed
				constexpr Fixed<T, FRAC> operator/(const Fixed<T, FRAC> &_x) const
				{ Fixed<T, FRAC> result; result.x = (this->x << FRAC) / _x.x; return result; }
				// Fixed /= Fixed
				Fixed<T, FRAC> operator/=(const Fixed<T, FRAC> &_x)
				{ *this = *this * _x; }

				// Fixed / number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> operator/(U _x) const
				{
					Fixed<T, FRAC> result; result.x = this->x / _x; return result;
				}
				// Fixed /= number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				Fixed<T, FRAC> operator/=(U _x)
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
