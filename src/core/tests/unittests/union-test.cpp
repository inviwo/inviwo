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

#include <inviwo/core/util/union.h>

#include <vector>
#include <list>
#include <string>
#include <ranges>
#include <iterator>
#include <algorithm>
#include <functional>
#include <concepts>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo {

using IntView = views::set_union<std::ranges::ref_view<std::vector<int>>,
                                 std::ranges::ref_view<std::vector<int>>>;

static_assert(std::ranges::input_range<IntView>);
static_assert(std::ranges::view<IntView>);
static_assert(std::input_iterator<std::ranges::iterator_t<IntView>>);
static_assert(std::same_as<std::ranges::range_value_t<IntView>, int>);

TEST(SetUnion, InterleavedDisjoint) {
    std::vector<int> a{1, 3, 5, 7};
    std::vector<int> b{2, 4, 6, 8};
    auto v =
        views::set_union{std::views::all(a), std::views::all(b)} | std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8}));
}

TEST(SetUnion, EqualElementsDeduplicated) {
    std::vector<int> a{1, 2, 3, 4};
    std::vector<int> b{2, 3, 5};
    auto v =
        views::set_union{std::views::all(a), std::views::all(b)} | std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST(SetUnion, FirstEmpty) {
    std::vector<int> a{};
    std::vector<int> b{1, 2, 3};
    auto v =
        views::set_union{std::views::all(a), std::views::all(b)} | std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3}));
}

TEST(SetUnion, SecondEmpty) {
    std::vector<int> a{4, 5, 6};
    std::vector<int> b{};
    auto v =
        views::set_union{std::views::all(a), std::views::all(b)} | std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{4, 5, 6}));
}

TEST(SetUnion, BothEmpty) {
    std::vector<int> a{}, b{};
    auto v =
        views::set_union{std::views::all(a), std::views::all(b)} | std::ranges::to<std::vector>();
    EXPECT_TRUE(v.empty());
}

TEST(SetUnion, OneFullyBeforeOther) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{10, 20, 30};
    auto v =
        views::set_union{std::views::all(a), std::views::all(b)} | std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3, 10, 20, 30}));
}

TEST(SetUnion, AllEqual) {
    std::vector<int> a{2, 2, 2};
    std::vector<int> b{2, 2, 2};
    auto v =
        views::set_union{std::views::all(a), std::views::all(b)} | std::ranges::to<std::vector>();
    // set-union semantics: pairs collapse, leftover length = max(|a|,|b|)
    EXPECT_EQ(v, (std::vector<int>{2, 2, 2}));
}

TEST(SetUnion, CustomComparatorDescending) {
    std::vector<int> a{7, 5, 3, 1};
    std::vector<int> b{8, 4, 2};
    auto v = views::set_union{std::views::all(a), std::views::all(b), std::greater<int>{}} |
             std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{8, 7, 5, 4, 3, 2, 1}));
}

TEST(SetUnion, WorksWithStrings) {
    std::vector<std::string> a{"apple", "cherry"};
    std::vector<std::string> b{"banana", "date"};
    auto v =
        views::set_union{std::views::all(a), std::views::all(b)} | std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<std::string>{"apple", "banana", "cherry", "date"}));
}

TEST(SetUnion, WorksWithDifferentRangeTypes) {
    std::vector<int> a{1, 4, 6};
    std::list<int> b{2, 3, 5, 7};
    auto v =
        views::set_union{std::views::all(a), std::views::all(b)} | std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3, 4, 5, 6, 7}));
}

TEST(SetUnion, ChainedWithOtherViews) {
    std::vector<int> a{1, 2, 3, 4, 5};
    std::vector<int> b{2, 4, 6, 8};
    auto v = views::set_union{std::views::all(a), std::views::all(b)} |
             std::views::transform([](int x) { return x * 10; }) | std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{10, 20, 30, 40, 50, 60, 80}));
}

TEST(SetUnion, PipeAdaptorClosure) {
    std::vector<int> a{1, 3, 5};
    std::vector<int> b{2, 3, 4};
    auto v = a | views::set_union_with(b) | std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST(SetUnion, IteratorIncrementPostfixCompiles) {
    std::vector<int> a{1, 2};
    std::vector<int> b{1, 3};
    auto v = views::set_union{std::views::all(a), std::views::all(b)};
    auto it = v.begin();
    EXPECT_EQ(*it, 1);
    it++;
    EXPECT_EQ(*it, 2);
    it++;
    EXPECT_EQ(*it, 3);
    it++;
    EXPECT_TRUE(it == std::default_sentinel);
}

}  // namespace inviwo
