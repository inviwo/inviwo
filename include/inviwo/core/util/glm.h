/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#ifndef IVW_GLM_H
#define IVW_GLM_H

#include <inviwo/core/common/inviwocoredefine.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <warn/ignore/sign-compare>
#include <warn/ignore/conversion>
#include <warn/ignore/misleading-indentation>
#include <warn/ignore/mismatched-tags>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/std_based_type.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/common.hpp>
#include <glm/detail/type_vec.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/hash.hpp>

#include <half/half.hpp>

#include <warn/pop>

#include <limits>
#include <type_traits>

namespace inviwo {

typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::dvec2 dvec2;
typedef glm::dvec3 dvec3;
typedef glm::dvec4 dvec4;
typedef glm::bvec2 bvec2;
typedef glm::bvec3 bvec3;
typedef glm::bvec4 bvec4;
typedef glm::uvec2 uvec2;
typedef glm::uvec3 uvec3;
typedef glm::uvec4 uvec4;
typedef glm::mat2 mat2;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::dmat2 dmat2;
typedef glm::dmat3 dmat3;
typedef glm::dmat4 dmat4;
typedef glm::quat quat;

typedef glm::size2_t size2_t;
typedef glm::size3_t size3_t;
typedef glm::size4_t size4_t;

typedef glm::tvec2<glm::uint64> u64vec2;
typedef glm::tvec3<glm::uint64> u64vec3;
typedef glm::tvec4<glm::uint64> u64vec4;

namespace util {

template <typename T>
struct is_floating_point : public std::is_floating_point<T> {};

template <>
struct is_floating_point<half_float::half> : std::true_type {};

template <typename T>
bool isfinite(const T& v) {
    return std::isfinite(v);
}

template <glm::length_t L, typename T, glm::qualifier Q>
glm::vec<L, bool, Q> isfinite(const glm::vec<L, T, Q>& x) {
    return glm::isfinite(x);
}

template <>
IVW_CORE_API bool isfinite(const half_float::half& v);

template <typename T>
bool isnan(const T& v) {
    return std::isnan(v);
}

template <glm::length_t L, typename T, glm::qualifier Q>
glm::vec<L, bool, Q> isnan(const glm::vec<L, T, Q>& x) {
    return glm::isnan(x);
}

template <>
IVW_CORE_API bool isnan(const half_float::half& v);

template <typename T>
struct rank : public std::rank<T> {};

template <typename T>
struct rank<const T> : public rank<T> {};

template <>
struct rank<half_float::half> : public std::integral_constant<std::size_t, 0> {};

template <glm::length_t L, typename T, glm::qualifier Q>
struct rank<glm::vec<L, T, Q>> : public std::integral_constant<std::size_t, 1> {};

template <typename T, glm::qualifier Q>
struct rank<glm::tquat<T, Q>> : public std::integral_constant<std::size_t, 1> {};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct rank<glm::mat<C, R, T, Q>> : public std::integral_constant<std::size_t, 2> {};

template <typename T, unsigned N = 0>
struct extent : public std::extent<T, N> {};

template <typename T, unsigned N>
struct extent<const T, N> : public extent<T, N> {};

template <glm::length_t L, typename T, glm::qualifier Q>
struct extent<glm::vec<L, T, Q>, 0> : public std::integral_constant<std::size_t, L> {};

template <typename T, glm::qualifier Q>
struct extent<glm::tquat<T, Q>, 0> : public std::integral_constant<std::size_t, 4> {};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct extent<glm::mat<C, R, T, Q>, 0> : public std::integral_constant<std::size_t, C> {};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct extent<glm::mat<C, R, T, Q>, 1> : public std::integral_constant<std::size_t, R> {};

namespace detail {
template <typename T, int N>
struct flat_extent_impl
    : public std::integral_constant<std::size_t, util::extent<T, N - 1>::value *
                                                     flat_extent_impl<T, N - 1>::value> {};

template <typename T>
struct flat_extent_impl<T, 0> : public std::integral_constant<std::size_t, 1> {};
}  // namespace detail

template <typename T>
struct flat_extent : detail::flat_extent_impl<T, util::rank<T>::value> {};

template <class U, class T, class BinaryOperation>
U accumulate(T x, U init, BinaryOperation op) {
    init = op(init, x);
    return init;
}

template <glm::length_t L, class U, glm::qualifier Q,
          template <glm::length_t, typename, glm::qualifier> class vecType, class BinaryOperation>
typename std::enable_if<util::rank<vecType<L, U, Q>>::value == 1, U>::type accumulate(
    vecType<L, U, Q> const& x, U init, BinaryOperation op) {
    for (glm::length_t i = 0; i < L; ++i) init = op(init, x[i]);
    return init;
}

template <glm::length_t C, glm::length_t R, class U, glm::qualifier Q,
          template <glm::length_t, glm::length_t, typename, glm::qualifier> class vecType,
          class BinaryOperation>
typename std::enable_if<util::rank<vecType<C, R, U, Q>>::value == 2, U>::type accumulate(
    vecType<C, R, U, Q> const& x, U init, BinaryOperation op) {
    for (size_t i = 0; i < C; ++i)
        for (size_t j = 0; j < R; ++j) init = op(init, x[i][j]);

    return init;
}

template <typename T = double, glm::length_t C = 1, glm::length_t R = 1,
          glm::qualifier Q = glm::defaultp>
struct glmtype {
    using type = glm::mat<C, R, T, Q>;
};

template <typename T, glm::qualifier P>
struct glmtype<T, 1, 1, P> {
    typedef T type;
};

template <typename T, glm::length_t L, glm::qualifier P>
struct glmtype<T, L, 1, P> {
    using type = glm::vec<L, T, P>;
};

/**
 * Extract the value_type from a scalar or glm type
 */
template <typename T>
struct value_type {
    using type = T;
};
template <glm::length_t L, typename T, glm::qualifier Q>
struct value_type<glm::vec<L, T, Q>> {
    using type = typename glm::vec<L, T, Q>::value_type;
};
template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct value_type<glm::mat<C, R, T, Q>> {
    using type = typename glm::mat<C, R, T, Q>::value_type;
};
template <typename T, glm::qualifier Q>
struct value_type<glm::tquat<T, Q>> {
    using type = typename glm::tquat<T, Q>::value_type;
};


/**
 * Construct a type with the same extents as the given type but with a different value type
 */
template <typename T, typename U>
struct same_extent {
    using type = U;
};
template <glm::length_t L, typename T, glm::qualifier Q, typename U>
struct same_extent<glm::vec<L, T, Q>, U> {
    using type = glm::vec<L, U, Q>;
};
template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q, typename U>
struct same_extent<glm::mat<C, R, T, Q>, U> {
    using type = glm::mat<C, R, U, Q>;
};
template <typename T, glm::qualifier Q, typename U>
struct same_extent<glm::tquat<T, Q>, U> {
    using type = glm::tquat<U, Q>;
};

// Type conversion

// disable conversion warning
#include <warn/push>
#include <warn/ignore/conversion>

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

// GLM element access wrapper functions. Useful in template functions with scalar and vec types

// vector like access
template <typename T, typename std::enable_if<util::rank<T>::value == 0, int>::type = 0>
constexpr auto glmcomp(T& elem, size_t) -> T& {
    return elem;
}
template <typename T, typename std::enable_if<util::rank<T>::value == 1, int>::type = 0>
constexpr auto glmcomp(T& elem, size_t i) ->
    typename std::conditional<std::is_const<T>::value, const typename T::value_type&,
                              typename T::value_type&>::type {
    return elem[i];
}
template <typename T, typename std::enable_if<util::rank<T>::value == 2, int>::type = 0>
constexpr auto glmcomp(T& elem, size_t i) ->
    typename std::conditional<std::is_const<T>::value, const typename T::value_type&,
                              typename T::value_type&>::type {
    return elem[i / util::extent<T, 0>::value][i % util::extent<T, 1>::value];
}
template <typename T, typename std::enable_if<util::rank<T>::value == 2, int>::type = 0>
constexpr auto glmcomp(const T& elem, size_t i) -> const typename T::value_type& {
    return elem[i / util::extent<T, 0>::value][i % util::extent<T, 1>::value];
}

// matrix like access
template <typename T, typename std::enable_if<util::rank<T>::value == 0, int>::type = 0>
constexpr auto glmcomp(T& elem, size_t i, size_t j) -> T& {
    return elem;
}
template <typename T, typename std::enable_if<util::rank<T>::value == 1, int>::type = 0>
constexpr auto glmcomp(T& elem, size_t i, size_t j) ->
    typename std::conditional<std::is_const<T>::value, const typename T::value_type&,
                              typename T::value_type&>::type {
    return elem[i];
}
template <typename T, typename std::enable_if<util::rank<T>::value == 2, int>::type = 0>
constexpr auto glmcomp(T& elem, size_t i, size_t j) ->
    typename std::conditional<std::is_const<T>::value, const typename T::value_type&,
                              typename T::value_type&>::type {
    return elem[i][j];
}
template <typename T, typename std::enable_if<util::rank<T>::value == 2, int>::type = 0>
constexpr auto glmcomp(const T& elem, size_t i, size_t j) -> const typename T::value_type& {
    return elem[i][j];
}

}  // namespace util

template <unsigned int Dim, typename Type>
using Matrix = typename util::glmtype<Type, Dim, Dim>::type;

template <unsigned int Dim, typename Type>
using Vector = typename util::glmtype<Type, Dim, 1>::type;

template <unsigned int N, typename T>
Matrix<N, T> MatrixInvert(const Matrix<N, T>& m) {
    return glm::inverse(m);
}
template <typename T>
Matrix<4, T> MatrixInvert(const glm::tmat4x4<T, glm::defaultp>& m) {
    return glm::inverse(m);
}
template <typename T>
Matrix<3, T> MatrixInvert(const glm::tmat3x3<T, glm::defaultp>& m) {
    return glm::inverse(m);
}
template <typename T>
Matrix<2, T> MatrixInvert(const glm::tmat2x2<T, glm::defaultp>& m) {
    return glm::inverse(m);
}

namespace util {

// Type1 and and Type2 models glm vec 2 types.
template <typename Type1, typename Type2>
Type1 invertY(Type1 vec, Type2 dim) {
    using T = typename Type1::value_type;
    return Type1(vec.x, static_cast<T>(dim.y) - 1 - vec.y);
}

template <typename T>
inline bool all(const T& t) {
    return glm::all(t);
}

template <>
inline bool all(const bool& t) {
    return t;
}

template <typename T>
inline bool any(const T& t) {
    return glm::any(t);
}

template <>
inline bool any(const bool& t) {
    return t;
}

}  // namespace util

}  // namespace inviwo

namespace glm {

#define VECTORIZE2_MAT(func)                                                              \
    template <typename T, precision P>                                                    \
    GLM_FUNC_QUALIFIER tmat2x2<T, P> func(tmat2x2<T, P> const& x) {                       \
        return tmat2x2<T, P>(func(x[0][0]), func(x[1][0]), func(x[0][1]), func(x[1][1])); \
    }

#define VECTORIZE3_MAT(func)                                                             \
    template <typename T, precision P>                                                   \
    GLM_FUNC_QUALIFIER tmat3x3<T, P> func(tmat3x3<T, P> const& x) {                      \
        return tmat3x3<T, P>(func(x[0][0]), func(x[1][0]), func(x[2][0]), func(x[0][1]), \
                             func(x[1][1]), func(x[2][1]), func(x[0][2]), func(x[1][2]), \
                             func(x[2][2]));                                             \
    }

#define VECTORIZE4_MAT(func)                                                              \
    template <typename T, precision P>                                                    \
    GLM_FUNC_QUALIFIER tmat4x4<T, P> func(tmat4x4<T, P> const& x) {                       \
        return tmat4x4<T, P>(func(x[0][0]), func(x[1][0]), func(x[2][0]), func(x[3][0]),  \
                             func(x[0][1]), func(x[1][1]), func(x[2][1]), func(x[3][1]),  \
                             func(x[0][2]), func(x[1][2]), func(x[2][2]), func(x[3][2]),  \
                             func(x[0][3]), func(x[1][3]), func(x[2][3]), func(x[3][3])); \
    }

#define VECTORIZE_MAT(func) \
    VECTORIZE2_MAT(func)    \
    VECTORIZE3_MAT(func)    \
    VECTORIZE4_MAT(func)

#define VECTORIZE2_MAT_SCA(func)                                                   \
    template <typename T, precision P>                                             \
    GLM_FUNC_QUALIFIER tmat2x2<T, P> func(tmat2x2<T, P> const& x, T const& y) {    \
        return tmat2x2<T, P>(func(x[0][0], y), func(x[1][0], y), func(x[0][1], y), \
                             func(x[1][1], y));                                    \
    }

#define VECTORIZE3_MAT_SCA(func)                                                    \
    template <typename T, precision P>                                              \
    GLM_FUNC_QUALIFIER tmat3x3<T, P> func(tmat3x3<T, P> const& x, T const& y) {     \
        return tmat3x3<T, P>(func(x[0][0], y), func(x[1][0], y), func(x[2][0], y),  \
                             func(x[0][1], y), func(x[1][1], y), func(x[2][1], y),  \
                             func(x[0][2], y), func(x[1][2], y), func(x[2][2], y)); \
    }

#define VECTORIZE4_MAT_SCA(func)                                                     \
    template <typename T, precision P>                                               \
    GLM_FUNC_QUALIFIER tmat4x4<T, P> func(tmat4x4<T, P> const& x, T const& y) {      \
        return tmat4x4<T, P>(                                                        \
            func(x[0][0], y), func(x[1][0], y), func(x[2][0], y), func(x[3][0], y),  \
            func(x[0][1], y), func(x[1][1], y), func(x[2][1], y), func(x[3][1], y),  \
            func(x[0][2], y), func(x[1][2], y), func(x[2][2], y), func(x[3][2], y),  \
            func(x[0][3], y), func(x[1][3], y), func(x[2][3], y), func(x[3][3], y)); \
    }

#define VECTORIZE_MAT_SCA(func) \
    VECTORIZE2_MAT_SCA(func)    \
    VECTORIZE3_MAT_SCA(func)    \
    VECTORIZE4_MAT_SCA(func)

#define VECTORIZE2_MAT_MAT(func)                                                            \
    template <typename T, precision P>                                                      \
    GLM_FUNC_QUALIFIER tmat2x2<T, P> func(tmat2x2<T, P> const& x, tmat2x2<T, P> const& y) { \
        return tmat2x2<T, P>(func(x[0][0], y[0][0]), func(x[1][0], y[1][0]),                \
                             func(x[0][1], y[0][1]), func(x[1][1], y[1][1]));               \
    }

#define VECTORIZE3_MAT_MAT(func)                                                            \
    template <typename T, precision P>                                                      \
    GLM_FUNC_QUALIFIER tmat3x3<T, P> func(tmat3x3<T, P> const& x, tmat3x3<T, P> const& y) { \
        return tmat3x3<T, P>(                                                               \
            func(x[0][0], y[0][0]), func(x[1][0], y[1][0]), func(x[2][0], y[2][0]),         \
            func(x[0][1], y[0][1]), func(x[1][1], y[1][1]), func(x[2][1], y[2][1]),         \
            func(x[0][2], y[0][2]), func(x[1][2], y[1][2]), func(x[2][2], y[2][2]));        \
    }

#define VECTORIZE4_MAT_MAT(func)                                                            \
    template <typename T, precision P>                                                      \
    GLM_FUNC_QUALIFIER tmat4x4<T, P> func(tmat4x4<T, P> const& x, tmat4x4<T, P> const& y) { \
        return tmat4x4<T, P>(                                                               \
            func(x[0][0], y[0][0]), func(x[1][0], y[1][0]), func(x[2][0], y[2][0]),         \
            func(x[3][0], y[3][0]), func(x[0][1], y[0][1]), func(x[1][1], y[1][1]),         \
            func(x[2][1], y[2][1]), func(x[3][1], y[3][1]), func(x[0][2], y[0][2]),         \
            func(x[1][2], y[1][2]), func(x[2][2], y[2][2]), func(x[3][2], y[3][2]),         \
            func(x[0][3], y[0][3]), func(x[1][3], y[1][3]), func(x[2][3], y[2][3]),         \
            func(x[3][3], y[3][3]));                                                        \
    }

template <typename T, precision P>
GLM_FUNC_QUALIFIER tquat<T, P> clamp(tquat<T, P> const& q, T a, T b) {
    return {glm::clamp(q[0], a, b), glm::clamp(q[1], a, b), glm::clamp(q[2], a, b),
            glm::clamp(q[3], a, b)};
}

template <typename T, precision P>
GLM_FUNC_QUALIFIER tquat<T, P> clamp(tquat<T, P> const& q, const tquat<T, P>& a,
                                     const tquat<T, P>& b) {
    return {glm::clamp(q[0], a[0], b[0]), glm::clamp(q[1], a[1], b[1]),
            glm::clamp(q[2], a[2], b[2]), glm::clamp(q[3], a[3], b[3])};
}

template <typename T, precision P>
GLM_FUNC_QUALIFIER tquat<T, P> min(const tquat<T, P>& a, const tquat<T, P>& b) {
    return {glm::min(a[0], b[0]), glm::min(a[1], b[1]), glm::min(a[2], b[2]), glm::min(a[3], b[3])};
}

template <typename T, precision P>
GLM_FUNC_QUALIFIER tquat<T, P> max(const tquat<T, P>& a, const tquat<T, P>& b) {
    return {glm::max(a[0], b[0]), glm::max(a[1], b[1]), glm::max(a[2], b[2]), glm::max(a[3], b[3])};
}

#define VECTORIZE_MAT_MAT(func) \
    VECTORIZE2_MAT_MAT(func)    \
    VECTORIZE3_MAT_MAT(func)    \
    VECTORIZE4_MAT_MAT(func)

VECTORIZE_MAT(abs)
VECTORIZE_MAT(sign)
VECTORIZE_MAT(floor)
VECTORIZE_MAT(trunc)
VECTORIZE_MAT(round)
VECTORIZE_MAT(roundEven)
VECTORIZE_MAT(ceil)
VECTORIZE_MAT(fract)

VECTORIZE_MAT_SCA(mod)
VECTORIZE_MAT_SCA(min)
VECTORIZE_MAT_SCA(max)

VECTORIZE_MAT_MAT(min)
VECTORIZE_MAT_MAT(max)
VECTORIZE_MAT_MAT(mod)

namespace detail {
#ifdef DARWIN
// Adding missing template specialization for size_t
// Needed in inviwo/dev/ext/glm/gtx/string_cast.inl
// when size_t is a unsigned long
template <>
struct prefix<size_t> {
    GLM_FUNC_QUALIFIER static char const* value() { return "u64"; };
};
#endif
}  // namespace detail

}  // namespace glm

#endif  // IVW_GLM_H
