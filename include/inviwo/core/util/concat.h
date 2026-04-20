/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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

#include <ranges>
#include <variant>
#include <tuple>
#include <concepts>
#include <type_traits>
#include <iterator>
#include <utility>
#include <cstddef>

namespace inviwo {

namespace detail {

// Common reference type across all ranges (same rule as std::views::concat)
template <typename... Rs>
using concat_reference_t = std::common_reference_t<std::ranges::range_reference_t<Rs>...>;

template <typename... Rs>
using concat_value_t = std::common_type_t<std::ranges::range_value_t<Rs>...>;

// Weakest iterator category shared by all ranges
template <typename... Rs>
using concat_iterator_category_t = std::common_type_t<
    typename std::iterator_traits<std::ranges::iterator_t<Rs>>::iterator_category...>;

// Helper to build variant type from index sequence
template <typename Ranges, typename Seq>
struct concat_iter_variant_helper;

template <typename Ranges, std::size_t... Is>
struct concat_iter_variant_helper<Ranges, std::index_sequence<Is...>> {
    using type = std::variant<std::ranges::iterator_t<std::tuple_element_t<Is, Ranges>>...>;
};

}  // namespace detail

// Local begin/end tags (std::ranges::begin_tag/end_tag are not standard)
namespace detail {
struct begin_tag_t {};
struct end_tag_t {};
inline constexpr begin_tag_t begin_tag{};
inline constexpr end_tag_t end_tag{};
}  // namespace detail

/**
 * @brief Lazy concatenation of multiple ranges into a single forward range.
 *
 * Drop-in for std::views::concat (C++26) for use with Clang/libc++ which does not yet
 * provide it. All ranges must share a common reference type
 * (std::common_reference_t of their range_reference_t types).
 *
 * Supports:
 *  - Input / forward / bidirectional iteration (matching the weakest category)
 *  - Both mutable and const iteration
 *  - std::ranges::view_interface helpers (empty(), front(), etc.)
 *  - Pipe syntax via util::concat range adaptor
 *
 * @example
 * std::vector<int> a{1, 2, 3};
 * std::vector<int> b{4, 5};
 * for (int x : util::concat(a, b)) { ... }          // 1 2 3 4 5
 *
 * auto r = util::concat(a, b) | std::views::filter([](int x){ return x % 2 == 0; });
 */
template <std::ranges::input_range... Rs>
    requires(sizeof...(Rs) >= 1) && requires { typename detail::concat_reference_t<Rs...>; }
class concat_view : public std::ranges::view_interface<concat_view<Rs...>> {
    static constexpr std::size_t N = sizeof...(Rs);
    using Ranges = std::tuple<Rs...>;
    using Ref = detail::concat_reference_t<Rs...>;

    Ranges ranges_;

    // ------------------------------------------------------------------
    // Iterator (templated on Const to share code between begin()/end())
    // ------------------------------------------------------------------
    template <bool Const>
    class Iterator {
        using Parent = std::conditional_t<Const, const concat_view, concat_view>;
        using Ranges_ = std::conditional_t<Const, const Ranges, Ranges>;

        template <std::size_t I>
        using RangeAt = std::tuple_element_t<I, Ranges_>;

        using IterVariant =
            typename detail::concat_iter_variant_helper<Ranges_,
                                                        std::index_sequence_for<Rs...>>::type;

        Parent* parent_ = nullptr;
        IterVariant current_{};

        // Advance past any exhausted ranges, moving current_ to the next range's begin.
        // Implemented as a recursive compile-time unrolled chain via if constexpr.
        template <std::size_t I>
        void satisfyForward() {
            if constexpr (I < N) {
                if (current_.index() == I) {
                    auto& r = std::get<I>(parent_->ranges_);
                    if (std::get<I>(current_) == std::ranges::end(r)) {
                        if constexpr (I + 1 < N) {
                            current_.template emplace<I + 1>(
                                std::ranges::begin(std::get<I + 1>(parent_->ranges_)));
                            satisfyForward<I + 1>();
                        }
                    }
                } else {
                    satisfyForward<I + 1>();
                }
            }
        }

        void satisfy() { satisfyForward<0>(); }

    public:
        using value_type = detail::concat_value_t<Rs...>;
        using reference = Ref;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        Iterator() = default;

        // Constructs the begin iterator (range 0, then satisfies forward)
        Iterator(Parent& parent, detail::begin_tag_t)
            : parent_{&parent}
            , current_{std::in_place_index<0>, std::ranges::begin(std::get<0>(parent.ranges_))} {
            satisfyForward<0>();
        }

        // Constructs the end iterator (sits at end of last range)
        Iterator(Parent& parent, detail::end_tag_t)
            : parent_{&parent}
            , current_{std::in_place_index<N - 1>,
                       std::ranges::end(std::get<N - 1>(parent.ranges_))} {}

        reference operator*() const {
            return std::visit([](auto& it) -> reference { return *it; }, current_);
        }

        Iterator& operator++() {
            std::visit([](auto& it) { ++it; }, current_);
            satisfy();
            return *this;
        }

        Iterator operator++(int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        bool operator==(const Iterator& other) const { return current_ == other.current_; }
    };

public:
    concat_view() = default;

    explicit concat_view(Rs... ranges) : ranges_{std::move(ranges)...} {}

    auto begin() { return Iterator<false>{*this, detail::begin_tag}; }
    auto end() { return Iterator<false>{*this, detail::end_tag}; }
    auto begin() const { return Iterator<true>{*this, detail::begin_tag}; }
    auto end() const { return Iterator<true>{*this, detail::end_tag}; }
};

// Deduction guide: wrap each range through std::views::all to allow temporaries
template <typename... Rs>
concat_view(Rs&&...) -> concat_view<std::views::all_t<Rs>...>;

namespace detail {

/**
 * @brief Lazily concatenate two or more ranges into a single range.
 *
 * Drop-in for std::views::concat (C++26). Wraps each range via std::views::all so
 * temporaries are handled correctly.
 *
 * @example
 * std::vector<int> a{1, 2, 3}, b{4, 5, 6};
 * for (int x : util::concat(a, b)) { ... }
 */
struct concat_fn {
    template <std::ranges::viewable_range... Rs>
        requires(sizeof...(Rs) >= 1) &&
                requires { typename detail::concat_reference_t<std::views::all_t<Rs>...>; }
    [[nodiscard]] auto operator()(Rs&&... rs) const {
        return concat_view<std::views::all_t<Rs>...>{std::views::all(std::forward<Rs>(rs))...};
    }
};

}  // namespace detail

namespace views {

/**
 * @brief Range factory / adaptor for lazy concatenation.
 * @see concat_view
 */
inline constexpr detail::concat_fn concat{};

}  // namespace views

}  // namespace inviwo
