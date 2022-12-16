/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/
#pragma once

#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmutils.h>

namespace inviwo::util {

// Type conversion

// disable conversion warning
#include <warn/push>
#include <warn/ignore/conversion>
#include <warn/ignore/conversion-loss>

// Standard conversion simple casts
// Just using standard casts. When targets has more dimensions then the source
// pad with zero. When there are more dimensions in source just discard the extra ones.
template <typename To = double, typename From>
inline constexpr To glm_convert(From x) {
    if constexpr (util::rank<From>::value == 0 && util::rank<To>::value == 0) {
        // Scalar to Scalar conversion
        return static_cast<To>(x);
    } else if constexpr (util::rank<From>::value == 0 && util::rank<To>::value == 1) {
        // Scalar to Vector conversion
        To res(typename To::value_type(0));
        res[0] = static_cast<typename To::value_type>(x);
        return res;
    } else if constexpr (util::rank<From>::value == 1 && util::rank<To>::value == 0) {
        // Vector to Scalar conversion
        return static_cast<To>(x[0]);
    } else if constexpr (util::rank<From>::value == 1 && util::rank<To>::value == 1) {
        // Vector to Vector conversion
        To res(static_cast<typename To::value_type>(0));
        size_t max = std::min(util::extent<To, 0>::value, util::extent<From, 0>::value);
        for (size_t i = 0; i < max; ++i) res[i] = static_cast<typename To::value_type>(x[i]);
        return res;
    }
}

/* Normalized conversions.
 * When going from floating point to integer, assume that floats are in the range (0,1)
 *
 *      Source (S)          Destination (D)     Action
 * ===============================================================================================
 * 0    X                   X                   x
 * 1    Float               Float               castD(x)
 * 2    Float               Unsigned Integer    castD(x * D::max)
 * 3    Float               Signed Integer      castD(x * D::range + D::low)

 * 4    Unsigned Integer    Unsigned Integer    castD(x / S::max * D::max)
 (* 5    Unsigned Integer    Signed Integer      castD(x / S::max * D::range + D::low))
 * 8    Unsigned Integer    Float               castD(x / S::max)

 * 7    Signed Integer      Signed Integer      castD((x - S::low) / S::range * D::range + D::low)
 (* 8    Signed Integer      Unsigned Integer    castD((x - S::low) / S::range * D::range))
 * 9    Signed Integer      Float               castD((x - S::low) / S::range)

 * 10   Scalar              Vector              {conv(x), 0,...}
 * 11   Vector              Scalar              conv(x[0])
 * 12   Vector              Vector              {conv(x[0]), conv(x[1]),...}
 */

namespace detail {

template <typename To, typename From>
constexpr bool is_scalar_conv =
    !std::is_same<To, From>::value && util::rank_v<To> == 0 && util::rank_v<From> == 0;

template <typename T>
constexpr bool is_unsigned_int = std::is_unsigned_v<T>&& std::is_integral_v<T>;

template <typename T>
constexpr bool is_signed_int = std::is_signed_v<T>&& std::is_integral_v<T>;

template <typename T, typename To = T>
constexpr To range = static_cast<To>(std::numeric_limits<T>::max()) -
                     static_cast<To>(std::numeric_limits<T>::lowest());

// If To is bigger then From then scale > 0 if To == From then scale == 0 else < 0
template <typename From, typename To>
constexpr int scale = std::numeric_limits<std::make_unsigned_t<To>>::digits -
                      std::numeric_limits<std::make_unsigned_t<From>>::digits;

template <typename T, bool Signed>
struct growImpl {};

template <>
struct growImpl<unsigned char, false> {
    using type = unsigned short;
};

template <>
struct growImpl<unsigned short, false> {
    using type = unsigned int;
};

template <>
struct growImpl<unsigned int, false> {
    using type = unsigned long long;
};

template <typename T>
struct growImpl<T, true> {
    using type =
        typename std::make_signed<typename growImpl<std::make_unsigned_t<T>, false>::type>::type;
};

template <typename T>
struct grow {
    using type = typename growImpl<T, std::is_signed<T>::value>::type;
};
template <typename T>
using grow_t = typename grow<T>::type;

template <typename T>
constexpr auto bigger() {
    if constexpr (std::is_unsigned_v<T>) {
        return std::make_unsigned_t<decltype(bigger<std::make_signed_t<T>>())>{};
    } else if constexpr (std::is_same_v<char, T>) {
        return short(0);
    } else if constexpr (std::is_same_v<short, T>) {
        return int(0);
    } else if constexpr (std::is_same_v<int, T>) {
        long long v{0};
        return v;
    } else if constexpr (std::is_same_v<long, T>) {
        if constexpr (sizeof(long) == sizeof(long long)) {
            return double{};
        } else {
            long long v{0};
            return v;
        }
    } else if constexpr (std::is_same_v<long long, T>) {
        return double{};
    }
}
template <typename T>
using bigger_t = decltype(bigger<T>());

/*
 * Calculate the ratio between to Integer domain sizes.
 * The number of digits in "To" should always be the larger
 * or equal to the number digits in "From".
 */
template <typename To, typename From>
constexpr auto rangeRatio = []() {
    using uTo = std::make_unsigned_t<To>;
    using uFrom = std::make_unsigned_t<From>;
    static_assert(std::numeric_limits<To>::digits >= std::numeric_limits<From>::digits,
                  "Invalid type order: To is smaller then From");
    return static_cast<std::make_signed_t<To>>(std::numeric_limits<uTo>::max() /
                                               static_cast<uTo>(std::numeric_limits<uFrom>::max()));
}();

}  // namespace detail

template <typename To = double, typename From>
constexpr To glm_convert_normalized(From x) {
    if constexpr (std::is_same_v<To, From>) {
        // To == From
        return x;
    } else if constexpr (detail::is_scalar_conv<To, From>) {

        if constexpr (util::is_floating_point_v<To> && util::is_floating_point_v<From>) {
            // 1. Floating point to floating point, only cast
            return static_cast<To>(x);

        } else if constexpr (detail::is_unsigned_int<To> && util::is_floating_point_v<From>) {
            // 2. Floating point to Unsigned Integer
            return static_cast<To>(x * std::numeric_limits<To>::max());

        } else if constexpr (detail::is_signed_int<To> && util::is_floating_point_v<From>) {
            // 3. Floating point to Signed Integer
            return static_cast<To>(x * detail::range<To, double> +
                                   static_cast<From>(std::numeric_limits<To>::lowest()));

        } else if constexpr (detail::is_unsigned_int<To> && detail::is_unsigned_int<From>) {
            // 4. Unsigned Integer to Unsigned Integer
            if constexpr (detail::scale<From, To> > 0) {
                return static_cast<To>(x) * detail::rangeRatio<To, From>;
            } else {
                return static_cast<To>(x / detail::rangeRatio<From, To>);
            }

        } else if constexpr (detail::is_signed_int<To> && detail::is_unsigned_int<From>) {
            // 5 Unsigned Integer to Signed Integer
            if constexpr (detail::scale<From, To> > 0) {
                return static_cast<To>(static_cast<detail::bigger_t<To>>(x) *
                                           detail::rangeRatio<To, From> +
                                       std::numeric_limits<To>::lowest());
            } else if (detail::scale<From, To> < 0) {
                return static_cast<To>(static_cast<std::make_signed_t<From>>(
                           x / detail::rangeRatio<From, To>)) +
                       std::numeric_limits<To>::lowest();
            } else {
                return static_cast<To>(
                    static_cast<std::make_signed_t<From>>(x + std::numeric_limits<To>::lowest()));
            }

        } else if constexpr (util::is_floating_point_v<To> && detail::is_unsigned_int<From>) {
            // 6 Unsigned Integer to Float
            return static_cast<To>(x) / static_cast<To>(std::numeric_limits<From>::max());

        } else if constexpr (detail::is_signed_int<To> && detail::is_signed_int<From>) {
            // 7. Signed Integer to Signed Integer
            if constexpr (detail::scale<From, To> > 0) {
                return static_cast<To>(
                    (static_cast<detail::bigger_t<To>>(x) - std::numeric_limits<From>::lowest()) *
                        detail::rangeRatio<To, From> +
                    std::numeric_limits<To>::lowest());
            } else {
                return static_cast<To>(
                    (static_cast<detail::grow_t<From>>(x) - std::numeric_limits<From>::lowest()) /
                        detail::rangeRatio<From, To> +
                    std::numeric_limits<To>::lowest());
            }

        } else if constexpr (detail::is_unsigned_int<To> && detail::is_signed_int<From>) {
            // 8 Signed Integer to Unsigned Integer
            if constexpr (detail::scale<From, To> > 0) {
                return static_cast<To>(static_cast<std::make_signed_t<To>>(x) -
                                       std::numeric_limits<From>::lowest()) *
                       detail::rangeRatio<To, From>;
            } else if constexpr (detail::scale<From, To> < 0) {
                return static_cast<To>(
                    (static_cast<detail::bigger_t<From>>(x) - std::numeric_limits<From>::lowest()) /
                    detail::rangeRatio<From, To>);
            } else {
                if (x < 0) {
                    return static_cast<To>(static_cast<std::make_signed_t<To>>(x) -
                                           std::numeric_limits<From>::lowest());
                } else {
                    return static_cast<To>(x) - std::numeric_limits<From>::lowest();
                }
            }
        } else if constexpr (util::is_floating_point_v<To> && detail::is_signed_int<From>) {
            // 9 Signed Integer to Float
            return static_cast<To>((static_cast<double>(x) -
                                    static_cast<double>(std::numeric_limits<From>::lowest())) /
                                   detail::range<From, double>);
        }

    } else if constexpr (!std::is_same_v<To, From> && util::rank_v<From> == 0 &&
                         util::rank_v<To> == 1) {
        // 10 Scalar to vector
        To res(0);
        res[0] = glm_convert_normalized<typename To::value_type>(x);
        return res;

    } else if constexpr (!std::is_same_v<To, From> && util::rank_v<From> == 1 &&
                         util::rank_v<To> == 0) {
        // 11 Vector to Scalar
        return glm_convert_normalized<To>(x[0]);

    } else if constexpr (!std::is_same_v<To, From> && util::rank_v<From> == 1 &&
                         util::rank_v<To> == 1) {
        // 12 Vector to Vector
        typedef typename To::value_type T;
        To res(static_cast<T>(0));
        size_t max = std::min(util::extent<To, 0>::value, util::extent<From, 0>::value);
        for (size_t i = 0; i < max; ++i) res[i] = glm_convert_normalized<T>(x[i]);
        return res;
    }
}  // namespace inviwo::util

#include <warn/pop>

}  // namespace inviwo::util
