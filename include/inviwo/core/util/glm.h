/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmutils.h>
#include <inviwo/core/util/glmconvert.h>

#include <warn/push>
#include <warn/ignore/all>

#include <glm/glm.hpp>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/scalar_relational.hpp>
#include <glm/gtx/scalar_multiplication.hpp>

#include <glm/ext/scalar_relational.hpp>
#include <glm/ext/vector_relational.hpp>
#include <glm/ext/matrix_relational.hpp>

#include <half/half.hpp>

#include <warn/pop>

#include <limits>
#include <type_traits>
#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace inviwo {

static_assert(std::is_standard_layout<half_float::half>::value, "");
static_assert(std::is_trivially_copyable<half_float::half>::value, "");
static_assert(std::is_default_constructible<half_float::half>::value, "");
static_assert(std::is_trivially_default_constructible<half_float::half>::value, "");
static_assert(std::is_trivial<half_float::half>::value, "");
static_assert(std::is_pod<half_float::half>::value, "");

using quat = glm::quat;

namespace util {

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

template <typename T = double, glm::length_t C = 1, glm::length_t R = 1,
          glm::qualifier Q = glm::defaultp>
using glmtype_t = typename glmtype<T, C, R, Q>::type;

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

namespace detail {

template <typename T>
struct epsilon {
    static T value() { return std::numeric_limits<T>::epsilon(); }
};

template <glm::length_t L, typename T, glm::qualifier Q>
struct epsilon<glm::vec<L, T, Q>> {
    static glm::vec<L, T, Q> value() { return glm::vec<L, T, Q>{epsilon<T>::value()}; }
};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct epsilon<glm::mat<C, R, T, Q>> {
    static glm::mat<C, R, T, Q> value() { return glm::mat<C, R, T, Q>{0} + epsilon<T>::value(); }
};

template <typename T>
struct min {
    static T value() { return std::numeric_limits<T>::min(); }
};

template <glm::length_t L, typename T, glm::qualifier Q>
struct min<glm::vec<L, T, Q>> {
    static glm::vec<L, T, Q> value() { return glm::vec<L, T, Q>{min<T>()}; }
};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct min<glm::mat<C, R, T, Q>> {
    static glm::mat<C, R, T, Q> value() { return glm::mat<C, R, T, Q>{0} + min<T>(); }
};

template <typename T>
struct almostEqual {
    template <typename U = T,
              typename std::enable_if<std::is_floating_point<U>::value, int>::type = 0>
    static bool value(const T& x, const T& y, int ulp) {
        return glm::equal(x, y, ulp);
    }

    template <typename U = T,
              typename std::enable_if<!std::is_floating_point<U>::value, int>::type = 0>
    static bool value(const T& x, const T& y, int) {
        return x == y;
    }
};

template <glm::length_t L, typename T, glm::qualifier Q>
struct almostEqual<glm::vec<L, T, Q>> {
    template <typename U = T,
              typename std::enable_if<std::is_floating_point<U>::value, int>::type = 0>
    static bool value(const glm::vec<L, T, Q>& x, const glm::vec<L, T, Q>& y, int ulp) {
        return glm::all(glm::equal(x, y, ulp));
    }

    template <typename U = T,
              typename std::enable_if<!std::is_floating_point<U>::value, int>::type = 0>
    static bool value(const glm::vec<L, T, Q>& x, const glm::vec<L, T, Q>& y, int) {
        return x == y;
    }
};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct almostEqual<glm::mat<C, R, T, Q>> {
    template <typename U = T,
              typename std::enable_if<std::is_floating_point<U>::value, int>::type = 0>
    static bool value(const glm::mat<C, R, T, Q>& x, const glm::mat<C, R, T, Q>& y, int ulp) {
        return glm::all(glm::equal(x, y, ulp));
    }

    template <typename U = T,
              typename std::enable_if<!std::is_floating_point<U>::value, int>::type = 0>
    static bool value(const glm::mat<C, R, T, Q>& x, const glm::mat<C, R, T, Q>& y, int ulp) {
        return x == y;
    }
};

}  // namespace detail

/**
 * Utility function to create a matrix filled with a constant.
 * For example:
 * \code{.cpp}
 * auto m = util::filled<mat3>(3.16);
 * // | 3.16 3.16 3.16 |
 * // | 3.16 3.16 3.16 |
 * // | 3.16 3.16 3.16 |
 * \endcode
 */
template <typename M, typename T = typename M::value_type>
M filled(T v) {
    return M{T(0)} + v;
}

template <typename T>
T epsilon() {
    return detail::epsilon<T>::value();
}

template <typename T>
bool almostEqual(const T& x, const T& y, int ulp = 2) {
    return detail::almostEqual<T>::value(x, y, ulp);
}

}  // namespace util

}  // namespace inviwo

namespace glm {

namespace detail {

// Vectorize mat
template <typename F, length_t C, length_t R, typename T, qualifier Q, size_t... Is>
constexpr auto vectorize_mat_helper(F&& f, const mat<C, R, T, Q>& a, std::index_sequence<Is...>) {
    using U = decltype(std::forward<F>(f)(value_ptr(a)[0]));
    return mat<C, R, U, Q>{std::forward<F>(f)(value_ptr(a)[Is])...};
}
template <typename F, length_t C, length_t R, typename T, qualifier Q>
constexpr auto vectorize_mat(F&& f, const mat<C, R, T, Q>& a) {
    return vectorize_mat_helper(std::forward<F>(f), a, std::make_index_sequence<C * R>{});
}

// Vectorize mat scalar
template <typename F, length_t C, length_t R, typename T, qualifier Q, typename Scalar,
          size_t... Is>
constexpr auto vectorize_mat_scalar_helper(F&& f, const mat<C, R, T, Q>& a, Scalar s,
                                           std::index_sequence<Is...>) {
    using U = decltype(std::forward<F>(f)(value_ptr(a)[0], s));
    return mat<C, R, U, Q>{std::forward<F>(f)(value_ptr(a)[Is], s)...};
}
template <typename F, length_t C, length_t R, typename T, qualifier Q, typename Scalar>
constexpr auto vectorize_mat_scalar(F&& f, const mat<C, R, T, Q>& a, Scalar s) {
    return vectorize_mat_scalar_helper(std::forward<F>(f), a, s, std::make_index_sequence<C * R>{});
}

// Vectorize mat scalar scalar
template <typename F, length_t C, length_t R, typename T, qualifier Q, typename Scalar,
          size_t... Is>
constexpr auto vectorize_mat_scalar_scalar_helper(F&& f, const mat<C, R, T, Q>& a, Scalar s1,
                                                  Scalar s2, std::index_sequence<Is...>) {
    using U = decltype(std::forward<F>(f)(value_ptr(a)[0], s1, s2));
    return mat<C, R, U, Q>{std::forward<F>(f)(value_ptr(a)[Is], s1, s2)...};
}
template <typename F, length_t C, length_t R, typename T, qualifier Q, typename Scalar>
constexpr auto vectorize_mat_scalar_scalar(F&& f, const mat<C, R, T, Q>& a, Scalar s1, Scalar s2) {
    return vectorize_mat_scalar_scalar_helper(std::forward<F>(f), a, s1, s2,
                                              std::make_index_sequence<C * R>{});
}

// Vectorize mat mat
template <typename F, length_t C, length_t R, typename T, typename V, qualifier Q, size_t... Is>
constexpr auto vectorize_mat_mat_helper(F&& f, const mat<C, R, T, Q>& a, const mat<C, R, V, Q>& b,
                                        std::index_sequence<Is...>) {
    using U = decltype(std::forward<F>(f)(value_ptr(a)[0], value_ptr(b)[0]));
    return mat<C, R, U, Q>{std::forward<F>(f)(value_ptr(a)[Is], value_ptr(b)[Is])...};
}
template <typename F, length_t C, length_t R, typename T, typename V, qualifier Q>
constexpr auto vectorize_mat_mat(F&& f, const mat<C, R, T, Q>& a, const mat<C, R, V, Q>& b) {
    return vectorize_mat_mat_helper(std::forward<F>(f), a, b, std::make_index_sequence<C * R>{});
}

// Vectorize mat mat scalar
template <typename F, length_t C, length_t R, typename T, typename V, qualifier Q, typename Scalar,
          size_t... Is>
constexpr auto vectorize_mat_mat_scalar_helper(F&& f, const mat<C, R, T, Q>& a,
                                               const mat<C, R, V, Q>& b, Scalar s,
                                               std::index_sequence<Is...>) {
    using U = decltype(std::forward<F>(f)(value_ptr(a)[0], value_ptr(b)[0], s));
    return mat<C, R, U, Q>{std::forward<F>(f)(value_ptr(a)[Is], value_ptr(b)[Is], s)...};
}
template <typename F, length_t C, length_t R, typename T, typename V, qualifier Q, typename Scalar>
constexpr auto vectorize_mat_mat_scalar(F&& f, const mat<C, R, T, Q>& a, const mat<C, R, V, Q>& b,
                                        Scalar s) {
    return vectorize_mat_mat_scalar_helper(std::forward<F>(f), a, b, s,
                                           std::make_index_sequence<C * R>{});
}

// Vectorize mat mat mat
template <typename F, length_t C, length_t R, typename T, typename V, typename W, qualifier Q,
          size_t... Is>
constexpr auto vectorize_mat_mat_mat_helper(F&& f, const mat<C, R, T, Q>& a,
                                            const mat<C, R, V, Q>& b, const mat<C, R, W, Q>& c,
                                            std::index_sequence<Is...>) {
    using U = decltype(std::forward<F>(f)(value_ptr(a)[0], value_ptr(b)[0], value_ptr(c)[0]));
    return mat<C, R, U, Q>{
        std::forward<F>(f)(value_ptr(a)[Is], value_ptr(b)[Is], value_ptr(c)[Is])...};
}
template <typename F, length_t C, length_t R, typename T, typename V, typename W, qualifier Q>
constexpr auto vectorize_mat_mat_mat(F&& f, const mat<C, R, T, Q>& a, const mat<C, R, V, Q>& b,
                                     const mat<C, R, W, Q>& c) {
    return vectorize_mat_mat_mat_helper(std::forward<F>(f), a, b, c,
                                        std::make_index_sequence<C * R>{});
}

}  // namespace detail

template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> abs(const mat<C, R, T, Q>& a) {
    return detail::vectorize_mat(static_cast<T (*)(T)>(abs), a);
}
template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> sign(const mat<C, R, T, Q>& a) {
    return detail::vectorize_mat(static_cast<T (*)(T)>(sign), a);
}
template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> floor(const mat<C, R, T, Q>& a) {
    return detail::vectorize_mat(static_cast<T (*)(T)>(floor), a);
}
template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> trunc(const mat<C, R, T, Q>& a) {
    return detail::vectorize_mat(static_cast<T (*)(T)>(trunc), a);
}
template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> round(const mat<C, R, T, Q>& a) {
    return detail::vectorize_mat(static_cast<T (*)(T)>(round), a);
}
template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> roundEven(const mat<C, R, T, Q>& a) {
    return detail::vectorize_mat(static_cast<T (*)(T)>(roundEven), a);
}
template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> ceil(const mat<C, R, T, Q>& a) {
    return detail::vectorize_mat(static_cast<T (*)(T)>(ceil), a);
}
template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> fract(const mat<C, R, T, Q>& a) {
    return detail::vectorize_mat(static_cast<T (*)(T)>(fract), a);
}

template <length_t C, length_t R, typename T, qualifier Q, typename Scalar>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> mod(const mat<C, R, T, Q>& a, Scalar s) {
    return detail::vectorize_mat_scalar(static_cast<T (*)(T, T)>(mod), a, s);
}

template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> min(const mat<C, R, T, Q>& a, T s) {
    return detail::vectorize_mat_scalar(static_cast<T (*)(T, T)>(min), a, s);
}
template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> min(T s, const mat<C, R, T, Q>& a) {
    return detail::vectorize_mat_scalar(static_cast<T (*)(T, T)>(min), a, s);
}

template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> max(const mat<C, R, T, Q>& a, T s) {
    return detail::vectorize_mat_scalar(static_cast<T (*)(T, T)>(max), a, s);
}
template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> max(T s, const mat<C, R, T, Q>& a) {
    return detail::vectorize_mat_scalar(static_cast<T (*)(T, T)>(max), a, s);
}

template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> min(const mat<C, R, T, Q>& a, const mat<C, R, T, Q>& b) {
    return detail::vectorize_mat_mat(static_cast<T (*)(T, T)>(min), a, b);
}
template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> max(const mat<C, R, T, Q>& a, const mat<C, R, T, Q>& b) {
    return detail::vectorize_mat_mat(static_cast<T (*)(T, T)>(max), a, b);
}

template <length_t C, length_t R, typename T, qualifier Q, typename Scalar>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> mix(const mat<C, R, T, Q>& a, const mat<C, R, T, Q>& b,
                                       Scalar s) {
    return detail::vectorize_mat_mat_scalar(static_cast<T (*)(T, T, Scalar)>(mix), a, b, s);
}

template <length_t C, length_t R, typename T, qualifier Q, typename Scalar>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> clamp(const mat<C, R, T, Q>& a, Scalar s1, Scalar s2) {
    return detail::vectorize_mat_scalar_scalar(static_cast<T (*)(T, Scalar, Scalar)>(clamp), a, s1,
                                               s2);
}

template <length_t C, length_t R, typename T, qualifier Q>
GLM_FUNC_QUALIFIER mat<C, R, T, Q> clamp(const mat<C, R, T, Q>& a, const mat<C, R, T, Q>& b,
                                         const mat<C, R, T, Q>& c) {
    return detail::vectorize_mat_mat_mat(static_cast<T (*)(T, T, T)>(clamp), a, b, c);
}

// Quats
template <typename T, precision P>
GLM_FUNC_QUALIFIER tquat<T, P> clamp(tquat<T, P> const& q, T a, T b) {
    return {clamp(q[0], a, b), clamp(q[1], a, b), clamp(q[2], a, b), clamp(q[3], a, b)};
}

template <typename T, precision P>
GLM_FUNC_QUALIFIER tquat<T, P> clamp(tquat<T, P> const& q, const tquat<T, P>& a,
                                     const tquat<T, P>& b) {
    return {clamp(q[0], a[0], b[0]), clamp(q[1], a[1], b[1]), clamp(q[2], a[2], b[2]),
            clamp(q[3], a[3], b[3])};
}

template <typename T, precision P>
GLM_FUNC_QUALIFIER tquat<T, P> min(const tquat<T, P>& a, const tquat<T, P>& b) {
    return {min(a[0], b[0]), min(a[1], b[1]), min(a[2], b[2]), min(a[3], b[3])};
}

template <typename T, precision P>
GLM_FUNC_QUALIFIER tquat<T, P> max(const tquat<T, P>& a, const tquat<T, P>& b) {
    return {max(a[0], b[0]), max(a[1], b[1]), max(a[2], b[2]), max(a[3], b[3])};
}

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

// Get function for the glm vecs. std::get is not a customization point
template <std::size_t N, glm::length_t L, typename T, glm::qualifier Q>
decltype(auto) get(const glm::vec<L, T, Q>& vec) {
    return vec[N];
}

template <std::size_t N, glm::length_t L, typename T, glm::qualifier Q>
decltype(auto) get(glm::vec<L, T, Q>& vec) {
    return vec[N];
}

}  // namespace glm

// Needed to get structured bindings to work for glm vecs
template <glm::length_t L, typename T, glm::qualifier Q>
struct std::tuple_size<glm::vec<L, T, Q>> : std::integral_constant<std::size_t, L> {};

template <std::size_t N, glm::length_t L, typename T, glm::qualifier Q>
struct std::tuple_element<N, glm::vec<L, T, Q>> {
    using type = T;
};

template <glm::length_t L, typename T, glm::qualifier Q>
struct fmt::formatter<glm::vec<L, T, Q>> : fmt::ostream_formatter {};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct fmt::formatter<glm::mat<C, R, T, Q>> : fmt::ostream_formatter {};

template <typename T, glm::qualifier Q>
struct fmt::formatter<glm::qua<T, Q>> : fmt::ostream_formatter {};
