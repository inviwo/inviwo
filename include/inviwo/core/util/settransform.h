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
#include <optional>
#include <type_traits>
#include <utility>
#include <memory>

namespace inviwo::views {

// Lazy engine shared by set_union and set_intersection (and any other sorted-merge set
// operation). It walks two sorted ranges in lock-step and, at every merge position, invokes
// `Fn` with two pointers into the common value type:
//   * both non-null  -> the two ranges hold an equivalent element here
//   * (p, nullptr)   -> `*p` exists only in the first range (or the second is exhausted)
//   * (nullptr, p)   -> `*p` exists only in the second range (or the first is exhausted)
// `Fn` returns a `std::optional`: an engaged value is yielded, `std::nullopt` skips the
// position. The elements are materialized into the common value type before being passed to
// `Fn`, so ranges yielding prvalues (e.g. transformed ranges) are supported.
template <std::ranges::input_range V1, std::ranges::input_range V2, typename Comp, typename Fn>
    requires std::ranges::view<V1> && std::ranges::view<V2> &&
             std::common_with<std::ranges::range_value_t<V1>, std::ranges::range_value_t<V2>> &&
             std::indirect_strict_weak_order<Comp, std::ranges::iterator_t<V1>,
                                             std::ranges::iterator_t<V2>>
class set_transform : public std::ranges::view_interface<set_transform<V1, V2, Comp, Fn>> {
public:
    using element_type =
        std::common_type_t<std::ranges::range_value_t<V1>, std::ranges::range_value_t<V2>>;

private:
    using fn_result_t = std::invoke_result_t<const Fn&, const element_type*, const element_type*>;

public:
    using value_type = std::remove_cvref_t<decltype(*std::declval<fn_result_t>())>;
    using reference = value_type;

    using difference_type = std::common_type_t<std::ranges::range_difference_t<V1>,
                                               std::ranges::range_difference_t<V2>>;

private:
    static constexpr bool both_forward =
        std::ranges::forward_range<V1> && std::ranges::forward_range<V2>;

    static constexpr bool both_common =
        std::ranges::common_range<V1> && std::ranges::common_range<V2>;

    V1 base1_{};
    V2 base2_{};
    Comp comp_{};
    Fn fn_{};

public:
    class iterator {
    public:
        using iterator_concept =
            std::conditional_t<both_forward, std::forward_iterator_tag, std::input_iterator_tag>;
        // Stay at input_iterator_tag: operator* returns a materialized prvalue, and C++20's
        // iterator_category contract requires an lvalue reference for anything stronger.
        using iterator_category = std::input_iterator_tag;
        using value_type = set_transform::value_type;
        using reference = set_transform::reference;
        using difference_type = set_transform::difference_type;
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
        Fn* fn_ = nullptr;

        friend class set_transform;

        bool atEnd() const { return it1_ == end1_ && it2_ == end2_; }

        // NOLINTBEGIN(bugprone-branch-clone)

        // Materialize the element(s) at the current merge position into `b1`/`b2`. The side(s)
        // that participate in the position are engaged, the other stays empty. Precondition:
        // !atEnd().
        void fill(std::optional<element_type>& b1, std::optional<element_type>& b2) const {
            if (it1_ == end1_) {
                b2.emplace(static_cast<element_type>(*it2_));
            } else if (it2_ == end2_) {
                b1.emplace(static_cast<element_type>(*it1_));
            } else if ((*comp_)(*it1_, *it2_)) {
                b1.emplace(static_cast<element_type>(*it1_));
            } else if ((*comp_)(*it2_, *it1_)) {
                b2.emplace(static_cast<element_type>(*it2_));
            } else {
                b1.emplace(static_cast<element_type>(*it1_));
                b2.emplace(static_cast<element_type>(*it2_));
            }
        }

        // Step to the next merge position. Precondition: !atEnd().
        void advance() {
            if (it1_ == end1_) {
                ++it2_;
            } else if (it2_ == end2_) {
                ++it1_;
            } else if ((*comp_)(*it1_, *it2_)) {
                ++it1_;
            } else if ((*comp_)(*it2_, *it1_)) {
                ++it2_;
            } else {
                ++it1_;
                ++it2_;
            }
        }
        // NOLINTEND(bugprone-branch-clone)

        // Advance until `fn_` yields a value at the current position or both ranges are
        // exhausted. This keeps the iterator on an emitted element between increments.
        void satisfy() {
            while (!atEnd()) {
                std::optional<element_type> b1;
                std::optional<element_type> b2;
                fill(b1, b2);
                const element_type* p1 = b1 ? std::addressof(*b1) : nullptr;
                const element_type* p2 = b2 ? std::addressof(*b2) : nullptr;
                if (std::invoke(*fn_, p1, p2)) return;
                advance();
            }
            // Both ranges exhausted: canonicalize to a unique past-the-end state so end() and
            // iterator equality are well-defined for common ranges.
            if constexpr (both_forward && both_common) {
                it1_ = end1_;
                it2_ = end2_;
            }
        }

    public:
        iterator() = default;

        iterator(It1 it1, Sen1 end1, It2 it2, Sen2 end2, Comp& comp, Fn& fn)
            : it1_(std::move(it1))
            , end1_(std::move(end1))
            , it2_(std::move(it2))
            , end2_(std::move(end2))
            , comp_(std::addressof(comp))
            , fn_(std::addressof(fn)) {
            satisfy();
        }

        reference operator*() const {
            std::optional<element_type> b1;
            std::optional<element_type> b2;
            fill(b1, b2);
            const element_type* p1 = b1 ? std::addressof(*b1) : nullptr;
            const element_type* p2 = b2 ? std::addressof(*b2) : nullptr;
            return static_cast<reference>(*std::invoke(*fn_, p1, p2));
        }

        iterator& operator++() {
            advance();
            satisfy();
            return *this;
        }

        // For input-only, operator++(int) must return void. For forward, it must return
        // a copy of the iterator (the "multipass" post-increment).
        decltype(auto) operator++(int) {
            if constexpr (both_forward) {
                auto tmp = *this;
                ++(*this);
                return tmp;
            } else {
                ++(*this);
            }
        }

        friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.atEnd(); }

        // Iterator/iterator equality: only meaningful (and only required) for forward.
        friend bool operator==(const iterator& a, const iterator& b)
            requires both_forward
        {
            return a.it1_ == b.it1_ && a.it2_ == b.it2_;
        }
    };

    set_transform()
        requires std::default_initializable<V1> && std::default_initializable<V2> &&
                     std::default_initializable<Comp> && std::default_initializable<Fn>
    = default;

    constexpr set_transform(V1 b1, V2 b2, Comp c = Comp{}, Fn f = Fn{})
        : base1_(std::move(b1)), base2_(std::move(b2)), comp_(std::move(c)), fn_(std::move(f)) {}

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
                        std::ranges::begin(base2_), std::ranges::end(base2_), comp_, fn_};
    }

    // When both bases are forward + common, expose a real end() iterator so the view
    // models common_range. Otherwise fall back to default_sentinel.
    constexpr auto end() {
        if constexpr (both_forward && both_common) {
            return iterator{std::ranges::end(base1_), std::ranges::end(base1_),
                            std::ranges::end(base2_), std::ranges::end(base2_), comp_, fn_};
        } else {
            return std::default_sentinel;
        }
    }
};

template <std::ranges::viewable_range R1, std::ranges::viewable_range R2, typename Comp,
          typename Fn>
set_transform(R1&&, R2&&, Comp, Fn)
    -> set_transform<std::views::all_t<R1>, std::views::all_t<R2>, Comp, Fn>;

}  // namespace inviwo::views
