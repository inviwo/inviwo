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

#include <inviwo/core/util/glmmat.h>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

// Add a bunch of element wise functions to glm which are not defined
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

}  // namespace glm
