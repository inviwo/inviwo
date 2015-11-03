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

// Normalized conversions. Only to floating point types.

// Scalar to Scalar conversion,
// Floating point to floating point, only cast
template <typename To = double, typename From,
          typename std::enable_if<util::rank<From>::value == 0 && util::rank<To>::value == 0 &&
                                      util::is_floating_point<To>::value &&
                                      util::is_floating_point<From>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(x);
}

// Unsigned integer to floating point, normalize and cast
template <typename To = double, typename From,
          typename std::enable_if<util::rank<From>::value == 0 && util::rank<To>::value == 0 &&
                                      util::is_floating_point<To>::value &&
                                      std::is_integral<From>::value &&
                                      std::is_unsigned<From>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(x) / static_cast<To>(std::numeric_limits<From>::max());
}

// Signed integer to floating point, normalize and cast
template <typename To = double, typename From,
          typename std::enable_if<util::rank<From>::value == 0 && util::rank<To>::value == 0 &&
                                      util::is_floating_point<To>::value &&
                                      std::is_integral<From>::value && std::is_signed<From>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    return (static_cast<To>(x) - static_cast<To>(std::numeric_limits<From>::lowest())) /
           (static_cast<To>(std::numeric_limits<From>::max()) -
            static_cast<To>(std::numeric_limits<From>::lowest()));
}


// Scalar to Vector conversion
// Floating point to floating point, only cast
template <class To, typename From,
          typename std::enable_if<util::rank<From>::value == 0 && util::rank<To>::value == 1 &&
                                      util::is_floating_point<typename To::value_type>::value &&
                                      util::is_floating_point<From>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    To res(0);
    res[0] = static_cast<typename To::value_type>(x);
    return res;
}

// Unsigned integer to floating point, normalize and cast
template <
    class To, typename From,
    typename std::enable_if<util::rank<From>::value == 0 && util::rank<To>::value == 1 &&
                                util::is_floating_point<typename To::value_type>::value &&
                                std::is_integral<From>::value && std::is_unsigned<From>::value,
                            int>::type = 0>
To glm_convert_normalized(From x) {
    To res(0);
    typedef typename To::value_type T;
    res[0] = static_cast<T>(x) / static_cast<T>(std::numeric_limits<From>::max());
    return res;
}

// Signed integer to floating point, normalize and cast
template <class To, typename From,
          typename std::enable_if<util::rank<From>::value == 0 && util::rank<To>::value == 1 &&
                                      util::is_floating_point<typename To::value_type>::value &&
                                      std::is_integral<From>::value && std::is_signed<From>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    To res(0);
    typedef typename To::value_type T;
    res[0] = (static_cast<T>(x) - static_cast<T>(std::numeric_limits<From>::lowest())) /
             (static_cast<T>(std::numeric_limits<From>::max()) -
              static_cast<T>(std::numeric_limits<From>::lowest()));
    return res;
}


// Vector to Scalar conversion
// Floating point to floating point, only cast
template <typename To = double, class From,
          typename std::enable_if<util::rank<From>::value == 1 && util::rank<To>::value == 0 &&
                                      util::is_floating_point<To>::value &&
                                      util::is_floating_point<typename From::value_type>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    return static_cast<To>(x[0]);
}

// Unsigned integer to floating point, normalize and cast
template <typename To = double, class From,
          typename std::enable_if<util::rank<From>::value == 1 && util::rank<To>::value == 0 &&
                                      util::is_floating_point<To>::value &&
                                      std::is_integral<typename From::value_type>::value &&
                                      std::is_unsigned<typename From::value_type>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    typedef typename From::value_type F;
    return static_cast<To>(x[0]) / static_cast<To>(std::numeric_limits<F>::max());
}

// Signed integer to floating point, normalize and cast
template <typename To = double, class From,
          typename std::enable_if<util::rank<From>::value == 1 && util::rank<To>::value == 0 &&
                                      util::is_floating_point<To>::value &&
                                      std::is_integral<typename From::value_type>::value &&
                                      std::is_signed<typename From::value_type>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    typedef typename From::value_type F;
    return (static_cast<To>(x[0]) - static_cast<To>(std::numeric_limits<F>::lowest())) /
           (static_cast<To>(std::numeric_limits<F>::max()) -
            static_cast<To>(std::numeric_limits<F>::lowest()));
}


// Vector to Vector conversion
// Floating point to floating point, only cast
template <class To, class From,
          typename std::enable_if<util::rank<From>::value == 1 && util::rank<To>::value == 1 &&
                                      util::is_floating_point<typename To::value_type>::value &&
                                      util::is_floating_point<typename From::value_type>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    typedef typename To::value_type T;
    To res(static_cast<T>(0));
    size_t max = std::min(util::extent<To, 0>::value, util::extent<From, 0>::value);
    for (size_t i = 0; i < max; ++i) res[i] = static_cast<T>(x[i]);
    return res;
}

// Unsigned integer to floating point, normalize and cast
template <class To, class From,
          typename std::enable_if<util::rank<From>::value == 1 && util::rank<To>::value == 1 &&
                                      util::is_floating_point<typename To::value_type>::value &&
                                      std::is_integral<typename From::value_type>::value &&
                                      std::is_unsigned<typename From::value_type>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    typedef typename To::value_type T;
    typedef typename From::value_type F;

    To res(static_cast<T>(0));
    size_t max = std::min(util::extent<To, 0>::value, util::extent<From, 0>::value);
    for (size_t i = 0; i < max; ++i) {
        res[i] = static_cast<T>(x[i]) / static_cast<T>(std::numeric_limits<F>::max());
    }

    return res;
}

// Signed integer to floating point, normalize and cast
template <class To, class From,
          typename std::enable_if<util::rank<From>::value == 1 && util::rank<To>::value == 1 &&
                                      util::is_floating_point<typename To::value_type>::value &&
                                      std::is_integral<typename From::value_type>::value &&
                                      std::is_signed<typename From::value_type>::value,
                                  int>::type = 0>
To glm_convert_normalized(From x) {
    typedef typename To::value_type T;
    typedef typename From::value_type F;
    To res(static_cast<T>(0));
    size_t max = std::min(util::extent<To, 0>::value, util::extent<From, 0>::value);
    for (size_t i = 0; i < max; ++i) {
        res[i] = (static_cast<T>(x[i]) - static_cast<T>(std::numeric_limits<F>::lowest())) /
                 (static_cast<T>(std::numeric_limits<F>::max()) -
                  static_cast<T>(std::numeric_limits<F>::lowest()));
    }
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

