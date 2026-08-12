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
// Set-intersection merge function: emit an element only where both ranges hold an equivalent
// element, skip positions unique to either range.
struct intersection_fn {
    template <typename T>
    constexpr std::optional<T> operator()(const T* p1, const T* p2) const {
        if (p1 && p2) return std::optional<T>{*p1};
        return std::nullopt;
    }
};
}  // namespace detail

template <std::ranges::input_range V1, std::ranges::input_range V2,
          typename Comp = std::ranges::less>
class set_intersection : public set_transform<V1, V2, Comp, detail::intersection_fn> {
public:
    using set_transform<V1, V2, Comp, detail::intersection_fn>::set_transform;
};

template <std::ranges::viewable_range R1, std::ranges::viewable_range R2,
          typename Comp = std::ranges::less>
set_intersection(R1&&, R2&&, Comp = {})
    -> set_intersection<std::views::all_t<R1>, std::views::all_t<R2>, Comp>;

namespace detail {
template <std::ranges::view V2, typename Comp = std::ranges::less>
class set_intersection_adaptor
    : public std::ranges::range_adaptor_closure<set_intersection_adaptor<V2, Comp>> {
    V2 r2_;
    Comp comp_;

public:
    explicit constexpr set_intersection_adaptor(V2 r2, Comp comp = {})
        : r2_(std::move(r2)), comp_(std::move(comp)) {}

    template <std::ranges::viewable_range R1>
    constexpr auto operator()(R1&& r1) const {
        return set_intersection{std::views::all(std::forward<R1>(r1)), r2_, comp_};
    }
};
}  // namespace detail

template <std::ranges::viewable_range R2, typename Comp = std::ranges::less>
constexpr auto set_intersection_with(R2&& r2, Comp comp = {}) {
    return detail::set_intersection_adaptor<std::views::all_t<R2>, Comp>{
        std::views::all(std::forward<R2>(r2)), std::move(comp)};
}

}  // namespace inviwo::views
