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
		template <typename T, T FRAC>
		class Fixed
		{
			private:
				T x;

			public:
				/// @brief Fixed point type
				using type = T;

				// Constructors
				/// @brief Default constructor
				constexpr Fixed() {}
				/// @brief Copy constructor
				/// @param _x Integer value
				constexpr Fixed(T _x) : x(_x << FRAC) {}
				/// @brief Copy constructor
				/// @param _x Float value
				/// @note This should always compile without using floats
				constexpr Fixed(float _x) : x(T(_x * (T(1) << FRAC))) {}
				/// @brief Copy constructor
				/// @param _x Double value
				/// @note This should always compile without using floats
				constexpr Fixed(double _x) : x(T(_x * (T(1) << FRAC))) {}
				/// @brief Copy constructor
				/// @param _x Long double value
				/// @note This should always compile without using floats
				constexpr Fixed(long double _x) : x(T(_x * (T(1) << FRAC))) {}

				/// @brief Copy constructor
				/// @tparam T2 Other fixed point type
				/// @tparam FRAC2 Other fixed point fractional bits
				/// @param rhs Other fixed point value
				template<typename T2, T2 FRAC2>
				constexpr Fixed(Fixed<T2, FRAC2> const &rhs) : x(*((T2*)&rhs))
				{ 
					if (FRAC2 > FRAC > 0)
						x >>= FRAC2 - FRAC;
					if (FRAC > FRAC2 > 0)
						x <<= FRAC - FRAC2;
				}

				// Functions
				/// @brief Get floored integer value
				/// @return Floored integer value
				Fixed<T, FRAC> floor() const
				{ Fixed<T, FRAC> result; result.x = x & ~((1 << FRAC) - 1); return result; }
				/// @brief Get ceiled integer value
				/// @return Ceiled integer value
				Fixed<T, FRAC> ceil() const
				{ Fixed<T, FRAC> result; result.x = (x + ((1 << FRAC) - 1)) & ~((1 << FRAC) - 1); return result; }
				/// @brief Get rounded integer value
				/// @return Rounded integer value
				Fixed<T, FRAC> round() const
				{ Fixed<T, FRAC> result; result.x = (x + ((1 << FRAC) >> 1)) & ~((1 << FRAC) - 1); return result; }

				// Operators
				Fixed<T, FRAC> operator+(const Fixed<T, FRAC> &_x) const
				{ Fixed<T, FRAC> result; result.x = this->x + _x.x; return result; }
				void operator+=(const Fixed<T, FRAC> &_x)
				{ *this = *this + _x; }
				
				Fixed<T, FRAC> operator-(const Fixed<T, FRAC> &_x) const
				{ Fixed<T, FRAC> result; result.x = this->x - _x.x; return result; }
				void operator-=(const Fixed<T, FRAC> &_x)
				{ *this = *this - _x; }
				
				Fixed<T, FRAC> operator+() const
				{ return *this; }
				Fixed<T, FRAC> operator-() const
				{ Fixed<T, FRAC> result; result.x = -this->x; return result; }

				Fixed<T, FRAC> operator*(const Fixed<T, FRAC> &_x) const
				{ Fixed<T, FRAC> result; result.x = (this->x * _x.x) >> FRAC; return result; }
				Fixed<T, FRAC> operator*=(const Fixed<T, FRAC> &_x)
				{ *this = *this * _x; }

				Fixed<T, FRAC> operator*(T _x) const
				{ Fixed<T, FRAC> result; result.x = this->x * _x; return result; }
				Fixed<T, FRAC> operator*=(T _x)
				{ *this = *this * _x; }

				Fixed<T, FRAC> operator/(const Fixed<T, FRAC> &_x) const
				{ Fixed<T, FRAC> result; result.x = (this->x << FRAC) / _x.x; return result; }
				Fixed<T, FRAC> operator/=(const Fixed<T, FRAC> &_x)
				{ *this = *this * _x; }

				Fixed<T, FRAC> operator/(T _x) const
				{ Fixed<T, FRAC> result; result.x = this->x / _x; return result; }
				Fixed<T, FRAC> operator/=(T &_x)
				{ *this = *this * _x; }

				bool operator<(const Fixed<T, FRAC> &_x) const
				{ return x < _x.x; }
				bool operator>(const Fixed<T, FRAC> &_x) const
				{ return x > _x.x; }
				bool operator<=(const Fixed<T, FRAC> &_x) const
				{ return x <= _x.x; }
				bool operator>=(const Fixed<T, FRAC> &_x) const
				{ return x >= _x.x; }
				bool operator==(const Fixed<T, FRAC> &_x) const
				{ return x == _x.x; }
				bool operator!=(const Fixed<T, FRAC> &_x) const
				{ return x != _x.x; }

				operator T() const
				{ return x >> FRAC; }
		};
	}
}
