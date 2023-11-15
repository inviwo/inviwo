/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2023 Inviwo Foundation
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

// Scalar to Scalar conversion
template <typename To = double, typename From,
          typename std::enable_if<util::rank<From>::value == 0 && util::rank<To>::value == 0,
                                  int>::type = 0>
inline To glm_convert(From x) {
    return static_cast<To>(x);
}

// Scalar to Vector conversion
template <class To, typename From,
          typename std::enable_if<util::rank<From>::value == 0 && util::rank<To>::value == 1,
                                  int>::type = 0>
inline To glm_convert(From x) {
    To res(0);
    res[0] = static_cast<typename To::value_type>(x);
    return res;
}

// Vector to Scalar conversion
template <typename To = double, class From,
          typename std::enable_if<util::rank<From>::value == 1 && util::rank<To>::value == 0,
                                  int>::type = 0>
inline To glm_convert(From x) {
    return static_cast<To>(x[0]);
}

// Vector to Vector conversion
template <class To, class From,
          typename std::enable_if<util::rank<From>::value == 1 && util::rank<To>::value == 1,
                                  int>::type = 0>
inline To glm_convert(From x) {
    To res(static_cast<typename To::value_type>(0));
    size_t max = std::min(util::extent<To, 0>::value, util::extent<From, 0>::value);
    for (size_t i = 0; i < max; ++i) res[i] = static_cast<typename To::value_type>(x[i]);
    return res;
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

// is_scalar_conv
template <typename To, typename From>
struct is_scalar_conv {
    static const bool value = !std::is_same<To, From>::value && util::rank<To>::value == 0 &&
                              util::rank<From>::value == 0;
};

// is_unsigned_int
template <typename T>
struct is_unsigned_int {
    static const bool value = std::is_unsigned<T>::value && std::is_integral<T>::value;
};

// is_signed_int
template <typename T>
struct is_signed_int {
    static const bool value = std::is_signed<T>::value && std::is_integral<T>::value;
};

template <typename T, typename To = T>
To range() {
    return (static_cast<To>(std::numeric_limits<T>::max()) -
            static_cast<To>(std::numeric_limits<T>::lowest()));
}

template <typename To, typename From>
struct upscale {
    static const bool value = std::numeric_limits<To>::digits > std::numeric_limits<From>::digits;
};

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
    using type = typename std::make_signed<
        typename growImpl<typename std::make_unsigned<T>::type, false>::type>::type;
};

template <typename T>
struct grow {
    using type = typename growImpl<T, std::is_signed<T>::value>::type;
};
template <typename T>
using grow_t = typename grow<T>::type;

/**
 * The size of a integer types domain is 2^digits - 1
 * The ratio between two types is then (2^d1 - 1) / (2^d2 - 1)
 * which for power of 2 number of digits can be written as:
 *
 * (-1 + x)(1+x^1)(1+x^2)(1+x^4)(1+x^8)...(1+x^d1/2)
 * --------------------------------------------------
 * (-1 + x)(1+x^1)(1+x^2)(1+x^4)(1+x^8)...(1+x^d2/2)
 *
 * assuming that d1 > d2 can be simplified to:
 *
 * (1+x^d2)...(1+x^d1/2)
 */

/* Recursive implementation
 *             / (2^D1+1) * f(D1*2, D2) if D1 != D2
 * f(D1,D2) = {
 *             \ 1                      if D1 == D2
 */
template <typename T, size_t D1, size_t D2>
struct rangeRatioImpl {
    static const T value = ((T{1} << D2) + T{1}) * rangeRatioImpl<T, D1, D2 * 2>::value;
};

// termination.
template <typename T, size_t D>
struct rangeRatioImpl<T, D, D> {
    static const T value = T{1};
};
/*
 * Calculate the ratio between to Integer domain sizes.
 * The number of digits in "To" should always be the larger
 * or equal to the number digits in "From".
 */
template <typename To, typename From>
struct rangeRatio {
    using uTo = typename std::make_unsigned<To>::type;
    using uFrom = typename std::make_unsigned<From>::type;
    static_assert(std::numeric_limits<To>::digits >= std::numeric_limits<From>::digits,
                  "Invalid type order: To is smaller then From");
    static const To value =
        static_cast<To>(rangeRatioImpl<uTo, std::numeric_limits<uTo>::digits,
                                       std::numeric_limits<uFrom>::digits>::value);
};

}  // namespace detail

// 0. To == From
template <typename To = double, typename From,
          typename std::enable_if<std::is_same<To, From>::value, int>::type = 0>
To glm_convert_normalized(From x) {
    return x;
}

// 1. Floating point to floating point, only cast
template <typename To = double, typename From,
          typename std::enable_if<detail::is_scalar_conv<To, From>::value &&
                                      util::is_floating_point<To>::value &&
                                      util::is_floating_point<From>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(x);
}

// 2. Floating point to Unsigned Integer
template <typename To = double, typename From,
          typename std::enable_if<detail::is_scalar_conv<To, From>::value &&
                                      detail::is_unsigned_int<To>::value &&
                                      util::is_floating_point<From>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(x * std::numeric_limits<To>::max());
}

// 3. Floating point to Signed Integer
template <typename To = double, typename From,
          typename std::enable_if<detail::is_scalar_conv<To, From>::value &&
                                      detail::is_signed_int<To>::value &&
                                      util::is_floating_point<From>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(x * detail::range<To, From>() +
                           static_cast<From>(std::numeric_limits<To>::lowest()));
}

// 4. Unsigned Integer to Unsigned Integer
template <typename To = double, typename From,
          typename std::enable_if<
              detail::is_scalar_conv<To, From>::value && detail::is_unsigned_int<To>::value &&
                  detail::is_unsigned_int<From>::value && detail::upscale<To, From>::value,
              int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(x) * detail::rangeRatio<To, From>::value;
}
template <typename To = double, typename From,
          typename std::enable_if<
              detail::is_scalar_conv<To, From>::value && detail::is_unsigned_int<To>::value &&
                  detail::is_unsigned_int<From>::value && !detail::upscale<To, From>::value,
              int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(x / detail::rangeRatio<From, To>::value);
}

// 5 Unsigned Integer to Signed Integer
template <typename To = double, typename From,
          typename std::enable_if<
              detail::is_scalar_conv<To, From>::value && detail::is_signed_int<To>::value &&
                  detail::is_unsigned_int<From>::value && detail::upscale<To, From>::value,
              int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(x) * detail::rangeRatio<To, From>::value +
           std::numeric_limits<To>::lowest();
}
template <typename To = double, typename From,
          typename std::enable_if<
              detail::is_scalar_conv<To, From>::value && detail::is_signed_int<To>::value &&
                  detail::is_unsigned_int<From>::value && !detail::upscale<To, From>::value,
              int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(static_cast<typename std::make_signed<From>::type>(
               x / detail::rangeRatio<From, To>::value)) +
           std::numeric_limits<To>::lowest();
}

// 6 Unsigned Integer to Float
template <typename To = double, typename From,
          typename std::enable_if<detail::is_scalar_conv<To, From>::value &&
                                      util::is_floating_point<To>::value &&
                                      detail::is_unsigned_int<From>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(x) / static_cast<To>(std::numeric_limits<From>::max());
}

// 7. Signed Integer to  Signed Integer
template <typename To = double, typename From,
          typename std::enable_if<
              detail::is_scalar_conv<To, From>::value && detail::is_signed_int<To>::value &&
                  detail::is_signed_int<From>::value && detail::upscale<To, From>::value,
              int>::type = 0>
To glm_convert_normalized(From x) {
    return (static_cast<To>(x) - std::numeric_limits<From>::lowest()) *
               detail::rangeRatio<To, From>::value +
           std::numeric_limits<To>::lowest();
}
template <typename To = double, typename From,
          typename std::enable_if<
              detail::is_scalar_conv<To, From>::value && detail::is_signed_int<To>::value &&
                  detail::is_signed_int<From>::value && !detail::upscale<To, From>::value,
              int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(
        (static_cast<detail::grow_t<From>>(x) - std::numeric_limits<From>::lowest()) /
            detail::rangeRatio<From, To>::value +
        std::numeric_limits<To>::lowest());
}

// 8 Signed Integer to Unsigned Integer
template <typename To = double, typename From,
          typename std::enable_if<
              detail::is_scalar_conv<To, From>::value && detail::is_unsigned_int<To>::value &&
                  detail::is_signed_int<From>::value && detail::upscale<To, From>::value,
              int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(static_cast<typename std::make_signed<To>::type>(x) -
                           std::numeric_limits<From>::lowest()) *
           detail::rangeRatio<To, From>::value;
}
template <typename To = double, typename From,
          typename std::enable_if<
              detail::is_scalar_conv<To, From>::value && detail::is_unsigned_int<To>::value &&
                  detail::is_signed_int<From>::value && !detail::upscale<To, From>::value,
              int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(static_cast<typename std::make_unsigned<From>::type>(
                               x - std::numeric_limits<From>::lowest()) /
                           detail::rangeRatio<From, To>::value);
}

// 9 Signed Integer to Float
template <typename To = double, typename From,
          typename std::enable_if<detail::is_scalar_conv<To, From>::value &&
                                      util::is_floating_point<To>::value &&
                                      detail::is_signed_int<From>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    return (static_cast<To>(x) - static_cast<To>(std::numeric_limits<From>::lowest())) /
           detail::range<From, To>();
}

// 10 Scalar to vector
template <class To, typename From,
          typename std::enable_if<!std::is_same<To, From>::value && util::rank<From>::value == 0 &&
                                      util::rank<To>::value == 1,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    To res(0);
    res[0] = glm_convert_normalized<typename To::value_type>(x);
    return res;
}

// 11 Vector to Scalar
template <class To, typename From,
          typename std::enable_if<!std::is_same<To, From>::value && util::rank<From>::value == 1 &&
                                      util::rank<To>::value == 0,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    return glm_convert_normalized<To>(x[0]);
}

// 12 Vector to Vector
template <class To, typename From,
          typename std::enable_if<!std::is_same<To, From>::value && util::rank<From>::value == 1 &&
                                      util::rank<To>::value == 1,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    typedef typename To::value_type T;
    To res(static_cast<T>(0));
    size_t max = std::min(util::extent<To, 0>::value, util::extent<From, 0>::value);
    for (size_t i = 0; i < max; ++i) res[i] = glm_convert_normalized<T>(x[i]);
    return res;
}

#include <warn/pop>

}  // namespace inviwo::util
