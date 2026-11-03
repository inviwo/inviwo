/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <algorithm>
#include <functional>
#include <tuple>
#include <utility>

namespace inviwo::util {

// Zip corresponding elements into a tuple of tuples
template <typename... Tuples>
constexpr auto tuple_zip(Tuples&&... tuples) {
    static_assert(sizeof...(Tuples) > 0);

    constexpr auto N = std::min({std::tuple_size_v<std::remove_cvref_t<Tuples>>...});

    return [&]<std::size_t... I>(std::index_sequence<I...>) {
        return std::tuple{[&]<std::size_t J>() {
            return std::tuple{std::get<J>(tuples)...};
        }.template operator()<I>()...};
    }
    (std::make_index_sequence<N>{});
}

// Transform corresponding elements and collect the results
template <typename F, typename... Tuples>
constexpr auto tuple_zip_transform(F&& f, Tuples&&... tuples) {
    static_assert(sizeof...(Tuples) > 0);

    constexpr auto N = std::min({std::tuple_size_v<std::remove_cvref_t<Tuples>>...});

    auto call = [&]<std::size_t I>() { return std::invoke(f, std::get<I>(tuples)...); };

    return [&]<std::size_t... I>(std::index_sequence<I...>) {
        return std::tuple{call.template operator()<I>()...};
    }
    (std::make_index_sequence<N>{});
}

// Invoke a callable on corresponding elements, discarding results
template <typename F, typename... Tuples>
constexpr void tuple_zip_for_each(F&& f, Tuples&&... tuples) {
    static_assert(sizeof...(Tuples) > 0);

    constexpr auto N = std::min({std::tuple_size_v<std::remove_cvref_t<Tuples>>...});

    auto call = [&]<std::size_t I>() { std::invoke(f, std::get<I>(tuples)...); };

    [&]<std::size_t... I>(std::index_sequence<I...>) { (call.template operator()<I>(), ...); }
    (std::make_index_sequence<N>{});
}

}  // namespace inviwo::util
