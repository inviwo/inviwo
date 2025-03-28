/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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

#include <glm/fwd.hpp>

#include <type_traits>

namespace inviwo {

namespace util {

template <typename T>
struct is_floating_point : public std::is_floating_point<T> {};

template <typename T>
constexpr bool is_floating_point_v = is_floating_point<T>::value;

template <typename T>
struct rank : public std::rank<T> {};

template <typename T>
struct rank<const T> : public rank<T> {};

template <glm::length_t L, typename T, glm::qualifier Q>
struct rank<glm::vec<L, T, Q>> : public std::integral_constant<std::size_t, 1> {};

template <typename T, glm::qualifier Q>
struct rank<glm::tquat<T, Q>> : public std::integral_constant<std::size_t, 1> {};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct rank<glm::mat<C, R, T, Q>> : public std::integral_constant<std::size_t, 2> {};

template <class T>
constexpr size_t rank_v = rank<T>::value;

template <class T, unsigned N = 0>
struct extent : std::integral_constant<std::size_t, 1> {};

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

template <class T, unsigned N = 0>
constexpr size_t extent_v = extent<T, N>::value;

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

template <class T>
constexpr size_t flat_extent_v = flat_extent<T>::value;

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

template <typename T>
using value_type_t = typename value_type<T>::type;

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

template <typename T, typename U>
using same_extent_t = typename same_extent<T, U>::type;

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

}  // namespace inviwo
