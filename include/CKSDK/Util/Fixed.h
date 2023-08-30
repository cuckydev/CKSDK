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
		/// @cond INTERNAL
		// Integer type extension for multiplication
		template <typename T, bool DIV = false>
		struct extend_integral_type { using type = T; };

		template <>
		struct extend_integral_type<char> { using type = short; };

		template <>
		struct extend_integral_type<unsigned char> { using type = unsigned short; };

		template <>
		struct extend_integral_type<short> { using type = int; };

		template <>
		struct extend_integral_type<unsigned short> { using type = unsigned int; };

		template <>
		struct extend_integral_type<int> { using type = long long; };

		template <>
		struct extend_integral_type<unsigned int> { using type = unsigned long long; };

		template <>
		struct extend_integral_type<long, false> { using type = long long; };
		template <>
		struct extend_integral_type<long, true> { using type = long; };

		template <>
		struct extend_integral_type<unsigned long, false> { using type = unsigned long long; };
		template <>
		struct extend_integral_type<unsigned long, true> { using type = unsigned long; };

		// No standard integral type larger than long long.
		template <>
		struct extend_integral_type<long long, false> { using type = long long; };
		template <>
		struct extend_integral_type<long long, true> { using type = long; };

		template <>
		struct extend_integral_type<unsigned long long, false> { using type = unsigned long long; };
		template <>
		struct extend_integral_type<unsigned long long, true> { using type = unsigned long; };

		template <typename T, bool DIV = false>
		using extend_integral_t = typename extend_integral_type<T, DIV>::type;

		// Integer type extension for two types, pick the larger one while keeping signedness
		template <typename T1, typename T2, bool DIV = false>
		struct extend_integral_2
		{
			using larger_type = std::conditional_t<(sizeof(T1) > sizeof(T2)), T1, T2>;
			using ext_larger_type = extend_integral_t<larger_type, DIV>;
			using type = std::conditional_t<std::is_signed_v<T1>, std::make_signed_t<ext_larger_type>, std::make_unsigned_t<ext_larger_type>>;
		};

		template <typename T1, typename T2, bool DIV = false>
		using extend_integral_2_t = typename extend_integral_2<T1, T2, DIV>::type;
		/// @endcond

		// Fixed point types
		/// @brief Fixed point class template
		/// @tparam T Fixed point type
		/// @tparam FRAC Fixed point fractional bits
		template <typename T, int FRAC>
		class Fixed
		{
			private:
				// Friend classes
				template <typename T2, int FRAC2>
				friend class Fixed;

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
				constexpr Fixed(Fixed<T2, FRAC2> const &rhs) noexcept : x(*((T2 *)&rhs))
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
						x = *((T2 *)&rhs) >> (FRAC2 - FRAC);
					else if constexpr (FRAC > FRAC2)
						x = *((T2 *)&rhs) << (FRAC - FRAC2);
					else
						x = *((T2 *)&rhs);
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
				{
					return Raw(x & ~((1 << FRAC) - 1));
				}
				/// @brief Get ceiled integer value
				/// @return Ceiled integer value
				constexpr Fixed<T, FRAC> Ceil() const
				{
					return Raw((x + ((1 << FRAC) - 1)) & ~((1 << FRAC) - 1));
				}
				/// @brief Get rounded integer value
				/// @return Rounded integer value
				constexpr Fixed<T, FRAC> Round() const
				{
					return Raw((x + ((1 << FRAC) >> 1)) & ~((1 << FRAC) - 1));
				}
				/// @brief Get raw value
				/// @return Raw value
				constexpr T Raw() const
				{
					return x;
				}
				/// @brief Construct from raw value
				/// @param _x Raw value
				/// @return Fixed point value
				constexpr static Fixed<T, FRAC> Raw(T _x)
				{
					Fixed<T, FRAC> result; result.x = _x; return result;
				}

				/// @brief Get lowest representable value
				/// @return Lowest representable value
				static constexpr Fixed<T, FRAC> Min()
				{
					return Raw(std::numeric_limits<T>::min());
				}
				/// @brief Get highest representable value
				/// @return Highest representable value
				static constexpr Fixed<T, FRAC> Max()
				{
					return Raw(std::numeric_limits<T>::max());
				}

				// Operators
				// Fixed += Fixed
				constexpr Fixed<T, FRAC> &operator+=(const Fixed<T, FRAC> &_x)
				{
					this->x += _x.x;
					return *this;
				}
				// Fixed + Fixed
				constexpr Fixed<T, FRAC> operator+(const Fixed<T, FRAC> &_x) const
				{
					return Fixed<T, FRAC>(*this) += _x;
				}

				// Fixed += number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> &operator+=(const U &_x)
				{
					this->x += Fixed<T, FRAC>(_x).x;
					return *this;
				}
				// Fixed + number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> operator+(const U &_x) const
				{
					return Fixed<T, FRAC>(*this) += _x;
				}
				// number + Fixed
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				friend constexpr Fixed<T, FRAC> operator+(const U &_x, const Fixed<T, FRAC> &_y)
				{
					return Fixed<T, FRAC>(_x) += _y;
				}

				// Fixed -= Fixed
				constexpr Fixed<T, FRAC> &operator-=(const Fixed<T, FRAC> &_x)
				{
					this->x -= _x.x;
					return *this;

				}
				// Fixed - Fixed
				constexpr Fixed<T, FRAC> operator-(const Fixed<T, FRAC> &_x) const
				{
					return Fixed<T, FRAC>(*this) -= _x;
				}

				// Fixed -= number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> &operator-=(const U &_x)
				{
					this->x -= Fixed<T, FRAC>(_x).x;
					return *this;
				}
				// Fixed - number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> operator-(const U &_x) const
				{
					return Fixed<T, FRAC>(*this) -= _x;
				}
				// number - Fixed
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				friend constexpr Fixed<T, FRAC> operator-(const U &_x, const Fixed<T, FRAC> &_y)
				{
					return Fixed<T, FRAC>(_x) -= _y;
				}

				// +Fixed
				constexpr Fixed<T, FRAC> operator+() const
				{
					return *this;
				}
				// -Fixed
				constexpr Fixed<T, FRAC> operator-() const
				{
					return Raw(-this->x);
				}

				// Fixed *= Fixed
				template <typename T2, int FRAC2>
				constexpr Fixed<T, FRAC> &operator*=(const Fixed<T2, FRAC2> &_x)
				{
					this->x = ((extend_integral_2_t<T, T2>)this->x * _x.x) >> FRAC2;
					return *this;
				}
				// Fixed * Fixed
				template <typename T2, int FRAC2>
				constexpr Fixed<T, FRAC> operator*(const Fixed<T2, FRAC2> &_x) const
				{
					return Fixed<T, FRAC>(*this) *= _x;
				}

				// Fixed *= number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> &operator*=(U _x)
				{
					if constexpr (std::is_integral_v<U>)
					{
						this->x *= _x;
						return *this;
					}
					else if constexpr (std::is_floating_point_v<U>)
					{
						*this *= Fixed<T, FRAC>(_x);
						return *this;
					}
				}
				// Fixed * number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> operator*(U _x) const
				{
					return Fixed<T, FRAC>(*this) *= _x;
				}
				// number * Fixed
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				friend constexpr Fixed<T, FRAC> operator*(U _x, const Fixed<T, FRAC> &_y)
				{
					return Fixed<T, FRAC>(_x) *= _y;
				}

				// Fixed /= Fixed
				template <typename T2, int FRAC2>
				constexpr Fixed<T, FRAC> &operator/=(const Fixed<T2, FRAC2> &_x)
				{
					this->x = ((extend_integral_2_t<T, T2, true>)this->x << FRAC2) / _x.x;
					return *this;
				}
				// Fixed / Fixed
				template <typename T2, int FRAC2>
				constexpr Fixed<T, FRAC> operator/(const Fixed<T2, FRAC2> &_x) const
				{
					return Fixed<T, FRAC>(*this) /= _x;
				}

				// Fixed /= number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> &operator/=(U _x)
				{
					if constexpr (std::is_integral_v<U>)
					{
						this->x /= _x;
						return *this;
					}
					else if constexpr (std::is_floating_point_v<U>)
					{
						*this /= Fixed<T, FRAC>(_x);
						return *this;
					}
				}
				// Fixed / number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr Fixed<T, FRAC> operator/(U _x) const
				{
					return Fixed<T, FRAC>(*this) /= _x;
				}
				// number / Fixed
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				friend constexpr Fixed<T, FRAC> operator/(U _x, const Fixed<T, FRAC> &_y)
				{
					return Fixed<T, FRAC>(_x) /= _y;
				}

				// Fixed < Fixed
				constexpr bool operator<(const Fixed<T, FRAC> &_x) const
				{
					return x < _x.x;
				}
				// Fixed < number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr bool operator<(U _x) const
				{
					return x < Fixed<T, FRAC>(_x).x;
				}
				// number < Fixed
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				friend constexpr bool operator<(U _x, const Fixed<T, FRAC> &_y)
				{
					return Fixed<T, FRAC>(_x).x < _y.x;
				}

				// Fixed > Fixed
				constexpr bool operator>(const Fixed<T, FRAC> &_x) const
				{
					return x > _x.x;
				}
				// Fixed > number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr bool operator>(U _x) const
				{
					return x > Fixed<T, FRAC>(_x).x;
				}
				// number > Fixed
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				friend constexpr bool operator>(U _x, const Fixed<T, FRAC> &_y)
				{
					return Fixed<T, FRAC>(_x).x > _y.x;
				}

				// Fixed <= Fixed
				constexpr bool operator<=(const Fixed<T, FRAC> &_x) const
				{
					return x <= _x.x;
				}
				// Fixed <= number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr bool operator<=(U _x) const
				{
					return x <= Fixed<T, FRAC>(_x).x;
				}
				// number <= Fixed
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				friend constexpr bool operator<=(U _x, const Fixed<T, FRAC> &_y)
				{
					return Fixed<T, FRAC>(_x).x <= _y.x;
				}

				// Fixed >= Fixed
				constexpr bool operator>=(const Fixed<T, FRAC> &_x) const
				{
					return x >= _x.x;
				}
				// Fixed >= number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr bool operator>=(U _x) const
				{
					return x >= Fixed<T, FRAC>(_x).x;
				}
				// number >= Fixed
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				friend constexpr bool operator>=(U _x, const Fixed<T, FRAC> &_y)
				{
					return Fixed<T, FRAC>(_x).x >= _y.x;
				}

				// Fixed == Fixed
				constexpr bool operator==(const Fixed<T, FRAC> &_x) const
				{
					return x == _x.x;
				}
				// Fixed == number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr bool operator==(U _x) const
				{
					return x == Fixed<T, FRAC>(_x).x;
				}
				// number == Fixed
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				friend constexpr bool operator==(U _x, const Fixed<T, FRAC> &_y)
				{
					return Fixed<T, FRAC>(_x).x == _y.x;
				}

				// Fixed != Fixed
				constexpr bool operator!=(const Fixed<T, FRAC> &_x) const
				{
					return x != _x.x;
				}
				// Fixed != number
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				constexpr bool operator!=(U _x) const
				{
					return x != Fixed<T, FRAC>(_x).x;
				}
				// number != Fixed
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				friend constexpr bool operator!=(U _x, const Fixed<T, FRAC> &_y)
				{
					return Fixed<T, FRAC>(_x).x != _y.x;
				}

				// (number)Fixed
				template <typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
				operator U() const
				{
					if constexpr (std::is_integral_v<U>)
						return U(x >> FRAC);
					else if constexpr (std::is_floating_point_v<U>)
						return x / U(1 << FRAC);
				}
		};
	}
}
