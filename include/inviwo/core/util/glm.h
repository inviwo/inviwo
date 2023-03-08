/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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
#include <inviwo/core/util/glmcomp.h>
#include <inviwo/core/util/glmconvert.h>
#include <inviwo/core/util/glmmatext.h>
#include <inviwo/core/util/glmfmt.h>

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

namespace inviwo {

static_assert(std::is_standard_layout<half_float::half>::value, "");
static_assert(std::is_trivially_copyable<half_float::half>::value, "");
static_assert(std::is_default_constructible<half_float::half>::value, "");
static_assert(std::is_trivially_default_constructible<half_float::half>::value, "");
static_assert(std::is_trivial<half_float::half>::value, "");
static_assert(std::is_standard_layout<half_float::half>::value, "");

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

}  // namespace util

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
