/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#ifndef GLM_SWIZZLE
#define GLM_SWIZZLE
#endif
#ifndef GLM_FORCE_SIZE_T_LENGTH
#define GLM_FORCE_SIZE_T_LENGTH
#endif

#include <warn/push>
#include <warn/ignore/shadow>
#include <warn/ignore/sign-compare>
#include <warn/ignore/conversion>

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
#include <glm/detail/precision.hpp>
#include <glm/gtx/io.hpp>

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

namespace util {

template <typename T>
struct is_floating_point : public std::is_floating_point<T> {};

template <>
struct is_floating_point<half_float::half> : std::true_type {};


template <typename T>
struct rank : public std::rank<T> {};

template <typename T, glm::precision P>
struct rank<glm::detail::tvec2<T, P>> : public std::integral_constant<std::size_t, 1> {};
template <typename T, glm::precision P>
struct rank<glm::detail::tvec3<T, P>> : public std::integral_constant<std::size_t, 1> {};
template <typename T, glm::precision P>
struct rank<glm::detail::tvec4<T, P>> : public std::integral_constant<std::size_t, 1> {};

template <typename T, glm::precision P>
struct rank<glm::detail::tmat2x2<T, P>> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct rank<glm::detail::tmat3x3<T, P>> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct rank<glm::detail::tmat4x4<T, P>> : public std::integral_constant<std::size_t, 2> {};

template <typename T, glm::precision P>
struct rank<glm::detail::tmat2x3<T, P>> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct rank<glm::detail::tmat2x4<T, P>> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct rank<glm::detail::tmat3x2<T, P>> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct rank<glm::detail::tmat3x4<T, P>> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct rank<glm::detail::tmat4x2<T, P>> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct rank<glm::detail::tmat4x3<T, P>> : public std::integral_constant<std::size_t, 2> {};


template <typename T, unsigned N = 0>
struct extent : public std::extent<T,N> {};

template <typename T, glm::precision P>
struct extent<glm::detail::tvec2<T, P>, 0> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tvec3<T, P>, 0> : public std::integral_constant<std::size_t, 3> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tvec4<T, P>, 0> : public std::integral_constant<std::size_t, 4> {};

template <typename T, glm::precision P>
struct extent<glm::detail::tmat2x2<T, P>, 0> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat3x3<T, P>, 0> : public std::integral_constant<std::size_t, 3> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat4x4<T, P>, 0> : public std::integral_constant<std::size_t, 4> {};

template <typename T, glm::precision P>
struct extent<glm::detail::tmat2x2<T, P>, 1> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat3x3<T, P>, 1> : public std::integral_constant<std::size_t, 3> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat4x4<T, P>, 1> : public std::integral_constant<std::size_t, 4> {};

template <typename T, glm::precision P>
struct extent<glm::detail::tmat2x3<T, P>, 0> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat2x4<T, P>, 0> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat3x2<T, P>, 0> : public std::integral_constant<std::size_t, 3> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat3x4<T, P>, 0> : public std::integral_constant<std::size_t, 3> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat4x2<T, P>, 0> : public std::integral_constant<std::size_t, 4> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat4x3<T, P>, 0> : public std::integral_constant<std::size_t, 4> {};

template <typename T, glm::precision P>
struct extent<glm::detail::tmat2x3<T, P>, 1> : public std::integral_constant<std::size_t, 3> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat2x4<T, P>, 1> : public std::integral_constant<std::size_t, 4> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat3x2<T, P>, 1> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat3x4<T, P>, 1> : public std::integral_constant<std::size_t, 4> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat4x2<T, P>, 1> : public std::integral_constant<std::size_t, 2> {};
template <typename T, glm::precision P>
struct extent<glm::detail::tmat4x3<T, P>, 1> : public std::integral_constant<std::size_t, 3> {};


template <typename T, int N>
struct flat_extent_impl
    : public std::integral_constant<
          std::size_t, util::extent<T, N-1>::value * flat_extent_impl<T, N - 1>::value> {};

template <typename T>
struct flat_extent_impl<T, 0> : public std::integral_constant<std::size_t, 1> {};

template <typename T>
struct flat_extent : flat_extent_impl<T, util::rank<T>::value> {};

template<class U, class T, class BinaryOperation>
U accumulate(T x, U init, BinaryOperation op) {
    init = op(init, x);
    return init;
}

template <class U, glm::precision P, template <typename, glm::precision> class vecType,
          class BinaryOperation>
typename std::enable_if<util::rank<vecType<U, P>>::value == 1, U >::type
    accumulate(vecType<U, P> const& x, U init, BinaryOperation op) {
    for (size_t i = 0; i < util::extent<vecType<U, P>,0>::value; ++i) init = op(init, x[i]);

    return init;
}

template <class U, glm::precision P, template <typename, glm::precision> class vecType,
          class BinaryOperation>
typename std::enable_if<util::rank<vecType<U, P>>::value == 2, U >::type
    accumulate(vecType<U, P> const& x, U init, BinaryOperation op) {
    for (size_t i = 0; i < util::extent<vecType<U, P>,0>::value; ++i)
        for (size_t j = 0; j< util::extent<vecType<U, P>,1>::value; ++j)
            init = op(init, x[i][j]);

    return init;
}

template <typename T = double, int dimX = 1, int dimY = 1, glm::precision P = glm::defaultp>
struct glmtype {};

template <typename T, glm::precision P>
struct glmtype<T, 1, 1, P> { typedef T type; };

template <typename T, glm::precision P>
struct glmtype<T, 2, 1, P> { typedef glm::detail::tvec2<T,P> type; };

template <typename T, glm::precision P>
struct glmtype<T, 3, 1, P> { typedef glm::detail::tvec3<T,P> type; };

template <typename T, glm::precision P>
struct glmtype<T, 4, 1, P> { typedef glm::detail::tvec4<T,P> type; };

template <typename T, glm::precision P>
struct glmtype<T, 2, 2, P> { typedef glm::detail::tmat2x2<T,P> type; };

template <typename T, glm::precision P>
struct glmtype<T, 3, 3, P> { typedef glm::detail::tmat3x3<T,P> type; };

template <typename T, glm::precision P>
struct glmtype<T, 4, 4, P> { typedef glm::detail::tmat4x4<T,P> type; };

template <typename T, typename U>
struct same_extent { typedef U type; };

template <typename T, glm::precision P, template <typename, glm::precision> class G, typename U>
struct same_extent<G<T,P>, U> { typedef G<U,P> type; };


// Type conversion


// disable conversion warning
#pragma warning(push)
#pragma warning(disable: 4244)

// Standard conversion simple casts
// Just using standard casts. When targets has more dimensions then the source
// pad with zero. When there are more dimensions in source just discard the extra ones.

// Scalar to Scalar conversion
template <typename To = double, typename From,
          typename std::enable_if<util::rank<From>::value == 0 && util::rank<To>::value == 0,
                                  int>::type = 0>
To glm_convert(From x) {
    return static_cast<To>(x);
}

// Scalar to Vector conversion
template <class To, typename From,
          typename std::enable_if<util::rank<From>::value == 0 && util::rank<To>::value == 1,
                                  int>::type = 0>
To glm_convert(From x) {
    To res(0);
    res[0] = static_cast<typename To::value_type>(x);
    return res;
}

// Vector to Scalar conversion
template <typename To = double, class From,
          typename std::enable_if<util::rank<From>::value == 1 && util::rank<To>::value == 0,
                                  int>::type = 0>
To glm_convert(From x) {
    return static_cast<To>(x[0]);
}

// Vector to Vector conversion
template <class To, class From,
          typename std::enable_if<util::rank<From>::value == 1 && util::rank<To>::value == 1,
                                  int>::type = 0>
To glm_convert(From x) {
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
    static const bool value =
        !std::is_same<To, From>::value && util::rank<To>::value == 0 && util::rank<From>::value == 0;
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
    static const To value = static_cast<To>(rangeRatioImpl<uTo, std::numeric_limits<uTo>::digits,
                                            std::numeric_limits<uFrom>::digits>::value);
};

}  // namespace

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
    return static_cast<To>((static_cast<detail::grow_t<From>>(x) - std::numeric_limits<From>::lowest()) /
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

#pragma warning(pop)

// GLM element access wrapper functions. 

// vector like access
template <typename T, typename std::enable_if<util::rank<T>::value == 0, int>::type = 0> 
auto glmcomp(T& elem, size_t i) -> T& {
    return elem;
}
template <typename T, typename std::enable_if<util::rank<T>::value == 1, int>::type = 0> 
auto glmcomp(T& elem, size_t i) -> typename T::value_type& {
    return elem[i];
}
template <typename T, typename std::enable_if<util::rank<T>::value == 2, int>::type = 0> 
auto glmcomp(T& elem, size_t i) -> typename T::value_type& {
    return elem[i / util::extent<T, 0>::value][i % util::extent<T, 1>::value];
}

// matrix like access
template <typename T, typename std::enable_if<util::rank<T>::value == 0, int>::type = 0> 
auto glmcomp(T& elem, size_t i, size_t j) -> T& {
    return elem;
}
template <typename T, typename std::enable_if<util::rank<T>::value == 1, int>::type = 0> 
auto glmcomp(T& elem, size_t i, size_t j) -> typename T::value_type& {
    return elem[i];
}
template <typename T, typename std::enable_if<util::rank<T>::value == 2, int>::type = 0> 
auto glmcomp(T& elem, size_t i, size_t j) -> typename T::value_type&{
    return elem[i][j];
}


} // namespace util

template <unsigned int Dim, typename Type>
using Matrix = typename util::glmtype<Type, Dim, Dim>::type;

template <unsigned int Dim, typename Type>
using Vector = typename util::glmtype<Type, Dim, 1>::type;

template <unsigned int N, typename T>
Matrix<N, T> MatrixInvert(const Matrix<N, T>& m) {
    return glm::inverse(m);
}
template <typename T>
Matrix<4, T> MatrixInvert(const glm::detail::tmat4x4<T, glm::defaultp>& m) {
    return glm::inverse(m);
}
template <typename T>
Matrix<3, T> MatrixInvert(const glm::detail::tmat3x3<T, glm::defaultp>& m) {
    return glm::inverse(m);
}
template <typename T>
Matrix<2, T> MatrixInvert(const glm::detail::tmat2x2<T, glm::defaultp>& m) {
    return glm::inverse(m);
}

namespace util {

// Type1 and and Type2 models glm vec 2 types.
template <typename Type1, typename Type2>
Type1 invertY(Type1 vec, Type2 dim) {
    using T = typename Type1::value_type;
    return Type1(vec.x, static_cast<T>(dim.y) - 1 - vec.y);
}

} // namespace util

} // namespace inviwo


namespace glm {


#define VECTORIZE2_MAT(func)                                                        \
    template <typename T, precision P>                                              \
    GLM_FUNC_QUALIFIER detail::tmat2x2<T, P> func(detail::tmat2x2<T, P> const& x) { \
        return detail::tmat2x2<T, P>(                                               \
            func(x[0][0]), func(x[1][0]),                                           \
            func(x[0][1]), func(x[1][1])                                            \
            );                                                                      \
    }

#define VECTORIZE3_MAT(func)                                                          \
    template <typename T, precision P>                                                \
    GLM_FUNC_QUALIFIER detail::tmat3x3<T, P> func(detail::tmat3x3<T, P> const& x) {   \
        return detail::tmat3x3<T, P>(                                                 \
            func(x[0][0]), func(x[1][0]), func(x[2][0]),                              \
            func(x[0][1]), func(x[1][1]), func(x[2][1]),                              \
            func(x[0][2]), func(x[1][2]), func(x[2][2])                               \
            );                                                                        \
    }

#define VECTORIZE4_MAT(func)                                                        \
    template <typename T, precision P>                                              \
    GLM_FUNC_QUALIFIER detail::tmat4x4<T, P> func(detail::tmat4x4<T, P> const& x) { \
        return detail::tmat4x4<T, P>(                                               \
            func(x[0][0]), func(x[1][0]), func(x[2][0]), func(x[3][0]),             \
            func(x[0][1]), func(x[1][1]), func(x[2][1]), func(x[3][1]),             \
            func(x[0][2]), func(x[1][2]), func(x[2][2]), func(x[3][2]),             \
            func(x[0][3]), func(x[1][3]), func(x[2][3]), func(x[3][3])              \
            );                                                                      \
    }


#define VECTORIZE_MAT(func) \
    VECTORIZE2_MAT(func)    \
    VECTORIZE3_MAT(func)    \
    VECTORIZE4_MAT(func)


#define VECTORIZE2_MAT_SCA(func)                                                    \
    template <typename T, precision P>                                              \
    GLM_FUNC_QUALIFIER detail::tmat2x2<T, P> func(detail::tmat2x2<T, P> const& x,   \
                                                  T const& y) {                     \
        return detail::tmat2x2<T, P>(                                               \
            func(x[0][0], y), func(x[1][0], y),                                     \
            func(x[0][1], y), func(x[1][1], y)                                      \
            );                                                                      \
    }

#define VECTORIZE3_MAT_SCA(func)                                                    \
    template <typename T, precision P>                                              \
    GLM_FUNC_QUALIFIER detail::tmat3x3<T, P> func(detail::tmat3x3<T, P> const& x,   \
                                                  T const& y) {                     \
        return detail::tmat3x3<T, P>(                                               \
            func(x[0][0], y), func(x[1][0], y), func(x[2][0], y),                   \
            func(x[0][1], y), func(x[1][1], y), func(x[2][1], y),                   \
            func(x[0][2], y), func(x[1][2], y), func(x[2][2], y)                    \
            );                                                                      \
    }

#define VECTORIZE4_MAT_SCA(func)                                                    \
    template <typename T, precision P>                                              \
    GLM_FUNC_QUALIFIER detail::tmat4x4<T, P> func(detail::tmat4x4<T, P> const& x,   \
                                                  T const& y) {                     \
        return detail::tmat4x4<T, P>(                                               \
            func(x[0][0], y), func(x[1][0], y), func(x[2][0], y), func(x[3][0], y), \
            func(x[0][1], y), func(x[1][1], y), func(x[2][1], y), func(x[3][1], y), \
            func(x[0][2], y), func(x[1][2], y), func(x[2][2], y), func(x[3][2], y), \
            func(x[0][3], y), func(x[1][3], y), func(x[2][3], y), func(x[3][3], y)  \
            );                                                                      \
    }

#define VECTORIZE_MAT_SCA(func) \
    VECTORIZE2_MAT_SCA(func)    \
    VECTORIZE3_MAT_SCA(func)    \
    VECTORIZE4_MAT_SCA(func)


#define VECTORIZE2_MAT_MAT(func)                                                    \
    template <typename T, precision P>                                              \
    GLM_FUNC_QUALIFIER detail::tmat2x2<T, P> func(detail::tmat2x2<T, P> const& x,   \
                                                  detail::tmat2x2<T, P> const& y) { \
        return detail::tmat2x2<T, P>(                                               \
            func(x[0][0], y[0][0]), func(x[1][0], y[1][0]),                         \
            func(x[0][1], y[0][1]), func(x[1][1], y[1][1])                          \
            );                                                                      \
    }

#define VECTORIZE3_MAT_MAT(func)                                                      \
    template <typename T, precision P>                                                \
    GLM_FUNC_QUALIFIER detail::tmat3x3<T, P> func(detail::tmat3x3<T, P> const& x,     \
                                                  detail::tmat3x3<T, P> const& y) {   \
        return detail::tmat3x3<T, P>(                                                 \
            func(x[0][0], y[0][0]), func(x[1][0], y[1][0]), func(x[2][0], y[2][0]),   \
            func(x[0][1], y[0][1]), func(x[1][1], y[1][1]), func(x[2][1], y[2][1]),   \
            func(x[0][2], y[0][2]), func(x[1][2], y[1][2]), func(x[2][2], y[2][2])    \
            );                                                                        \
    }

#define VECTORIZE4_MAT_MAT(func)                                                                            \
    template <typename T, precision P>                                                                      \
    GLM_FUNC_QUALIFIER detail::tmat4x4<T, P> func(detail::tmat4x4<T, P> const& x,                           \
                                                  detail::tmat4x4<T, P> const& y) {                         \
        return detail::tmat4x4<T, P>(                                                                       \
            func(x[0][0], y[0][0]), func(x[1][0], y[1][0]), func(x[2][0], y[2][0]), func(x[3][0], y[3][0]), \
            func(x[0][1], y[0][1]), func(x[1][1], y[1][1]), func(x[2][1], y[2][1]), func(x[3][1], y[3][1]), \
            func(x[0][2], y[0][2]), func(x[1][2], y[1][2]), func(x[2][2], y[2][2]), func(x[3][2], y[3][2]), \
            func(x[0][3], y[0][3]), func(x[1][3], y[1][3]), func(x[2][3], y[2][3]), func(x[3][3], y[3][3])  \
            );                                                                                              \
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

} // namespace

#endif // IVW_GLM_H

