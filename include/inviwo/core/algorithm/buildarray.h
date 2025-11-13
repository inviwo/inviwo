/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <array>
#include <utility>
#include <type_traits>
#include <cstddef>

namespace inviwo::util {

template <std::size_t N, typename Index = size_t, typename Functor>
constexpr auto build_array(const Functor& func) noexcept {
    return []<typename F, Index... Is>(const F& func, std::integer_sequence<Index, Is...>) {
        return std::array{func(std::integral_constant<Index, Is>{})...};
    }(func, std::make_integer_sequence<Index, N>());
}

template <std::size_t N, typename Index = size_t, typename Functor>
constexpr auto build_array_t(const Functor& func) noexcept {
    return []<typename F, Index... Is>(const F& func, std::integer_sequence<Index, Is...>) {
        return std::array { func.template operator()<std::integral_constant<Index, Is>{}>()... };
    }(func, std::make_integer_sequence<Index, N>());
}

template <auto N, decltype(N)... Ns, typename Functor>
constexpr auto build_array_nd(const Functor& func) {
    return [&]<decltype(N)... Is>(std::integer_sequence<decltype(N), Is...>) {
        if constexpr (sizeof...(Ns) == 0) {  // Base case: produce a 1D array
            return std::array{func(std::integral_constant<decltype(N), Is>{})...};
        } else {  // Recursive case
            return std::array{build_array_nd<Ns...>([&](auto... rest) {
                return func(std::integral_constant<decltype(N), Is>{}, rest...);
            })...};
        }
    }(std::make_integer_sequence<decltype(N), N>{});
}

template <auto N, decltype(N)... Ns, typename Functor>
constexpr auto build_array_t_nd(const Functor& func) {
    return [&]<decltype(N)... Is>(std::integer_sequence<decltype(N), Is...>) {
        if constexpr (sizeof...(Ns) == 0) {  // Base case: produce a 1D array
            return std::array { func.template operator()<Is>()... };
        } else {  // Recursive case
            return std::array{build_array_t_nd<Ns...>(
                [&]<auto... rest>() { return func.template operator()<Is, rest...>(); })...};
        }
    }(std::make_integer_sequence<decltype(N), N>{});
}

}  // namespace inviwo::util
