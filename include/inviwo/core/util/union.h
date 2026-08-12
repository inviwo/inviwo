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
#include <inviwo/core/util/settransform.h>

#include <ranges>
#include <concepts>
#include <functional>
#include <optional>
#include <utility>

namespace inviwo::views {

namespace detail {
// Set-union merge function: emit the element present at each merge position. When both ranges
// hold an equivalent element the pair collapses to a single output (`p1`).
struct union_fn {
    template <typename T>
    constexpr std::optional<T> operator()(const T* p1, const T* p2) const {
        return p1 ? std::optional<T>{*p1} : std::optional<T>{*p2};
    }
};
}  // namespace detail

template <std::ranges::input_range V1, std::ranges::input_range V2,
          typename Comp = std::ranges::less>
class set_union : public set_transform<V1, V2, Comp, detail::union_fn> {
public:
    using set_transform<V1, V2, Comp, detail::union_fn>::set_transform;
};

template <std::ranges::viewable_range R1, std::ranges::viewable_range R2,
          typename Comp = std::ranges::less>
set_union(R1&&, R2&&, Comp = {}) -> set_union<std::views::all_t<R1>, std::views::all_t<R2>, Comp>;

namespace detail {
template <std::ranges::view V2, typename Comp = std::ranges::less>
class set_union_adaptor : public std::ranges::range_adaptor_closure<set_union_adaptor<V2, Comp>> {
    V2 r2_;
    Comp comp_;

public:
    constexpr set_union_adaptor(V2 r2, Comp comp = {})
        : r2_(std::move(r2)), comp_(std::move(comp)) {}

    template <std::ranges::viewable_range R1>
    constexpr auto operator()(R1&& r1) const {
        return set_union{std::views::all(std::forward<R1>(r1)), r2_, comp_};
    }
};
}  // namespace detail

template <std::ranges::viewable_range R2, typename Comp = std::ranges::less>
constexpr auto set_union_with(R2&& r2, Comp comp = {}) {
    return detail::set_union_adaptor<std::views::all_t<R2>, Comp>{
        std::views::all(std::forward<R2>(r2)), std::move(comp)};
}

}  // namespace inviwo::views
