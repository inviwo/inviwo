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

#include <inviwo/core/util/intersection.h>

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

using IntView = views::set_intersection<std::ranges::ref_view<std::vector<int>>,
                                        std::ranges::ref_view<std::vector<int>>>;

static_assert(std::ranges::input_range<IntView>);
static_assert(std::ranges::view<IntView>);
static_assert(std::input_iterator<std::ranges::iterator_t<IntView>>);
static_assert(std::same_as<std::ranges::range_value_t<IntView>, int>);

TEST(SetIntersection, InterleavedDisjoint) {
    std::vector<int> a{1, 3, 5, 7};
    std::vector<int> b{2, 4, 6, 8};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)} |
             std::ranges::to<std::vector>();
    EXPECT_TRUE(v.empty());
}

TEST(SetIntersection, SomeCommonElements) {
    std::vector<int> a{1, 2, 3, 4};
    std::vector<int> b{2, 3, 5};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)} |
             std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{2, 3}));
}

TEST(SetIntersection, FirstEmpty) {
    std::vector<int> a{};
    std::vector<int> b{1, 2, 3};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)} |
             std::ranges::to<std::vector>();
    EXPECT_TRUE(v.empty());
}

TEST(SetIntersection, SecondEmpty) {
    std::vector<int> a{4, 5, 6};
    std::vector<int> b{};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)} |
             std::ranges::to<std::vector>();
    EXPECT_TRUE(v.empty());
}

TEST(SetIntersection, BothEmpty) {
    std::vector<int> a{}, b{};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)} |
             std::ranges::to<std::vector>();
    EXPECT_TRUE(v.empty());
}

TEST(SetIntersection, OneFullyBeforeOther) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{10, 20, 30};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)} |
             std::ranges::to<std::vector>();
    EXPECT_TRUE(v.empty());
}

TEST(SetIntersection, Identical) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{1, 2, 3};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)} |
             std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3}));
}

TEST(SetIntersection, DuplicatesTakeMinCount) {
    std::vector<int> a{2, 2, 2};
    std::vector<int> b{2, 2};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)} |
             std::ranges::to<std::vector>();
    // set-intersection semantics: run of equal elements collapses to min(|a|,|b|)
    EXPECT_EQ(v, (std::vector<int>{2, 2}));
}

TEST(SetIntersection, CustomComparatorDescending) {
    std::vector<int> a{7, 5, 3, 1};
    std::vector<int> b{8, 5, 3, 2};
    auto v =
        views::set_intersection{std::views::all(a), std::views::all(b), std::greater<int>{}} |
        std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{5, 3}));
}

TEST(SetIntersection, WorksWithStrings) {
    std::vector<std::string> a{"apple", "banana", "cherry"};
    std::vector<std::string> b{"banana", "cherry", "date"};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)} |
             std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<std::string>{"banana", "cherry"}));
}

TEST(SetIntersection, WorksWithDifferentRangeTypes) {
    std::vector<int> a{1, 2, 3, 4, 6};
    std::list<int> b{2, 3, 5, 6, 7};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)} |
             std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{2, 3, 6}));
}

TEST(SetIntersection, ChainedWithOtherViews) {
    std::vector<int> a{1, 2, 3, 4, 5};
    std::vector<int> b{2, 4, 6, 8};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)} |
             std::views::transform([](int x) { return x * 10; }) | std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{20, 40}));
}

TEST(SetIntersection, PipeAdaptorClosure) {
    std::vector<int> a{1, 3, 5};
    std::vector<int> b{2, 3, 4, 5};
    auto v = a | views::set_intersection_with(b) | std::ranges::to<std::vector>();
    EXPECT_EQ(v, (std::vector<int>{3, 5}));
}

TEST(SetIntersection, IteratorIncrementPostfixCompiles) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{2, 3, 4};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)};
    auto it = v.begin();
    EXPECT_EQ(*it, 2);
    it++;
    EXPECT_EQ(*it, 3);
    it++;
    EXPECT_TRUE(it == std::default_sentinel);
}

using V = views::set_intersection<std::ranges::ref_view<std::vector<int>>,
                                  std::ranges::ref_view<std::vector<int>>>;

static_assert(std::ranges::forward_range<V>);
static_assert(std::forward_iterator<std::ranges::iterator_t<V>>);
static_assert(std::ranges::common_range<V>);

TEST(SetIntersection, MultipassYieldsSameSequence) {
    std::vector<int> a{1, 3, 5}, b{3, 4, 5};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)};

    std::vector<int> first, second;
    for (auto x : v) first.push_back(x);
    for (auto x : v) second.push_back(x);  // second pass
    EXPECT_EQ(first, second);
    EXPECT_EQ(first, (std::vector<int>{3, 5}));
}

TEST(SetIntersection, IteratorEquality) {
    std::vector<int> a{1, 2, 3}, b{2, 3, 4};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)};
    auto i = v.begin();
    auto j = v.begin();
    EXPECT_TRUE(i == j);
    ++i;
    EXPECT_FALSE(i == j);
    ++j;
    EXPECT_TRUE(i == j);
}

TEST(SetIntersection, EndIsIteratorAndDistanceWorks) {
    std::vector<int> a{1, 2, 3, 4, 5, 6}, b{2, 4, 6};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)};
    EXPECT_EQ(std::ranges::distance(v.begin(), v.end()), 3);
    auto it = std::ranges::find(v, 4);
    EXPECT_NE(it, v.end());
    EXPECT_EQ(*it, 4);
}

TEST(SetIntersection, PostIncrementReturnsCopy) {
    std::vector<int> a{1, 2, 3}, b{2, 3};
    auto v = views::set_intersection{std::views::all(a), std::views::all(b)};
    auto it = v.begin();
    auto old = it++;
    EXPECT_EQ(*old, 2);
    EXPECT_EQ(*it, 3);
}

}  // namespace inviwo
