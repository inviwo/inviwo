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

#include <inviwo/core/util/glmutils.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmmat.h>

namespace inviwo::util {

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
constexpr auto glmcomp(T& elem, size_t, size_t) -> T& {
    return elem;
}
template <typename T, typename std::enable_if<util::rank<T>::value == 1, int>::type = 0>
constexpr auto glmcomp(T& elem, size_t i, size_t) ->
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

}  // namespace inviwo::util
