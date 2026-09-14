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

#include <tuple>
#include <utility>
#include <type_traits>
#include <functional>
#include <concepts>

namespace inviwo::util {

namespace detail {

template <typename F, typename Tuple, std::size_t... Is>
consteval bool invocable_prefix_impl(std::index_sequence<Is...>) {
    using Tup = std::remove_reference_t<Tuple>;
    return std::invocable<F, std::tuple_element_t<Is, Tup>...>;
}

template <std::size_t N, typename F, typename Tuple>
consteval bool invocable_prefix() {
    return invocable_prefix_impl<F, Tuple>(std::make_index_sequence<N>{});
}

template <typename F, typename Tuple, std::size_t... Ns>
consteval std::size_t best_prefix_from_seq(std::index_sequence<Ns...>) {
    std::size_t best = 0;
    ((best = invocable_prefix<Ns, F, Tuple>() ? Ns : best), ...);
    return best;
}

template <typename F, typename Tuple>
consteval std::size_t best_prefix() {
    constexpr std::size_t M = std::tuple_size_v<std::remove_reference_t<Tuple>>;
    return best_prefix_from_seq<F, Tuple>(std::make_index_sequence<M + 1>{});
}

template <std::size_t N, typename F, typename Tuple, std::size_t... Is>
decltype(auto) invoke_prefix_impl(F&& f, Tuple&& t, std::index_sequence<Is...>) {
    return std::invoke(std::forward<F>(f), std::get<Is>(std::forward<Tuple>(t))...);
}

template <std::size_t N, typename F, typename Tuple>
decltype(auto) invoke_prefix(F&& f, Tuple&& t) {
    return invoke_prefix_impl<N>(std::forward<F>(f), std::forward<Tuple>(t),
                                 std::make_index_sequence<N>{});
}

}  // namespace detail

/**
 * @brief Invoke a callable with the longest invocable prefix of the provided arguments.
 *
 * Determines, at compile time, the largest prefix length `N` (where `0 <= N <= sizeof...(Args)`)
 * such that @p f is invocable with the first `N` arguments. Then calls @p f using exactly that
 * prefix and ignores any remaining trailing arguments.
 *
 * Use case:
 * In connection with QObject::connect since Qt's signaling mechanism supports calling
 * slots/callbacks with fewer arguments than provided by the signal.
 * @see util::exceptionGuarded
 *
 * @tparam F    Callable type (forwarding reference).
 * @tparam Args Variadic argument types (forwarding references).
 *
 * @param f     Callable object/function to invoke.
 * @param args  Candidate arguments from which the invocable prefix is selected.
 *
 * @return The result of invoking @p f with the selected prefix.
 */
template <typename F, typename... Args>
decltype(auto) invoke(F&& f, Args&&... args) {
    auto tup = std::forward_as_tuple(std::forward<Args>(args)...);
    using Tup = decltype(tup);
    using Fn = F&&;

    constexpr std::size_t N = detail::best_prefix<Fn, Tup>();

    static_assert(N > 0 || std::invocable<Fn>,
                  "Callable is not invocable with any provided prefix (including empty).");

    return detail::invoke_prefix<N>(std::forward<F>(f), tup);
}

}  // namespace inviwo::util
