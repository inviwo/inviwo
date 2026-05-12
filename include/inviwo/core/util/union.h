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

#include <ranges>
#include <iterator>
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>
#include <memory>

namespace inviwo::views {

template <std::ranges::input_range V1, std::ranges::input_range V2,
          typename Comp = std::ranges::less>
    requires std::ranges::view<V1> && std::ranges::view<V2> &&
             std::common_reference_with<std::ranges::range_reference_t<V1>,
                                        std::ranges::range_reference_t<V2>> &&
             std::common_with<std::ranges::range_value_t<V1>, std::ranges::range_value_t<V2>> &&
             std::indirect_strict_weak_order<Comp, std::ranges::iterator_t<V1>,
                                             std::ranges::iterator_t<V2>>
class set_union : public std::ranges::view_interface<set_union<V1, V2, Comp>> {
public:
    using value_type =
        std::common_type_t<std::ranges::range_value_t<V1>, std::ranges::range_value_t<V2>>;

    using reference = std::common_reference_t<std::ranges::range_reference_t<V1>,
                                              std::ranges::range_reference_t<V2>>;

    using difference_type = std::common_type_t<std::ranges::range_difference_t<V1>,
                                               std::ranges::range_difference_t<V2>>;

private:
    V1 base1_{};
    V2 base2_{};
    Comp comp_{};

public:
    class iterator {
    public:
        using iterator_concept = std::input_iterator_tag;
        using iterator_category = std::input_iterator_tag;
        using value_type = set_union::value_type;
        using reference = set_union::reference;
        using difference_type = set_union::difference_type;
        using pointer = void;

    private:
        using It1 = std::ranges::iterator_t<V1>;
        using It2 = std::ranges::iterator_t<V2>;
        using Sen1 = std::ranges::sentinel_t<V1>;
        using Sen2 = std::ranges::sentinel_t<V2>;

        It1 it1_{};
        Sen1 end1_{};
        It2 it2_{};
        Sen2 end2_{};
        Comp* comp_ = nullptr;

    public:
        iterator() = default;

        iterator(It1 it1, Sen1 end1, It2 it2, Sen2 end2, Comp& comp)
            : it1_(std::move(it1))
            , end1_(std::move(end1))
            , it2_(std::move(it2))
            , end2_(std::move(end2))
            , comp_(std::addressof(comp)) {}

        reference operator*() const {
            if (it1_ == end1_) return static_cast<reference>(*it2_);
            if (it2_ == end2_) return static_cast<reference>(*it1_);
            if ((*comp_)(*it2_, *it1_)) return static_cast<reference>(*it2_);
            return static_cast<reference>(*it1_);
        }

        iterator& operator++() {
            if (it1_ == end1_) {
                ++it2_;
            } else if (it2_ == end2_) {
                ++it1_;
            } else if ((*comp_)(*it1_, *it2_)) {
                ++it1_;
            } else if ((*comp_)(*it2_, *it1_)) {
                ++it2_;
            } else {
                // equivalent -> set-union: emit once, skip in both
                // For "merge" semantics (keep duplicates) replace this
                // branch with: ++it1_;
                ++it1_;
                ++it2_;
            }
            return *this;
        }

        void operator++(int) { ++(*this); }

        friend bool operator==(const iterator& it, std::default_sentinel_t) {
            return it.it1_ == it.end1_ && it.it2_ == it.end2_;
        }
    };

    set_union()
        requires std::default_initializable<V1> && std::default_initializable<V2> &&
                     std::default_initializable<Comp>
    = default;

    constexpr set_union(V1 b1, V2 b2, Comp c = Comp{})
        : base1_(std::move(b1)), base2_(std::move(b2)), comp_(std::move(c)) {}

    constexpr V1 base1() const&
        requires std::copy_constructible<V1>
    {
        return base1_;
    }
    constexpr V2 base2() const&
        requires std::copy_constructible<V2>
    {
        return base2_;
    }
    constexpr V1 base1() && { return std::move(base1_); }
    constexpr V2 base2() && { return std::move(base2_); }

    constexpr iterator begin() {
        return iterator{std::ranges::begin(base1_), std::ranges::end(base1_),
                        std::ranges::begin(base2_), std::ranges::end(base2_), comp_};
    }

    constexpr std::default_sentinel_t end() const noexcept { return std::default_sentinel; }
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
