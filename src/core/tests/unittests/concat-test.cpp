/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2026 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/util/concat.h>

#include <vector>
#include <array>
#include <list>
#include <string>
#include <string_view>
#include <ranges>
#include <algorithm>
#include <numeric>

namespace inviwo {

TEST(ConcatView, TwoVectors) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{4, 5, 6};
    std::vector<int> result;
    for (int x : views::concat(a, b)) result.push_back(x);
    EXPECT_EQ(result, (std::vector<int>{1, 2, 3, 4, 5, 6}));
}

TEST(ConcatView, ThreeRanges) {
    std::vector<int> a{1, 2};
    std::array<int, 2> b{3, 4};
    std::vector<int> c{5, 6};
    std::vector<int> result;
    for (int x : views::concat(a, b, c)) result.push_back(x);
    EXPECT_EQ(result, (std::vector<int>{1, 2, 3, 4, 5, 6}));
}

TEST(ConcatView, FirstRangeEmpty) {
    std::vector<int> a{};
    std::vector<int> b{1, 2, 3};
    std::vector<int> result;
    for (int x : views::concat(a, b)) result.push_back(x);
    EXPECT_EQ(result, (std::vector<int>{1, 2, 3}));
}

TEST(ConcatView, SecondRangeEmpty) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{};
    std::vector<int> result;
    for (int x : views::concat(a, b)) result.push_back(x);
    EXPECT_EQ(result, (std::vector<int>{1, 2, 3}));
}

TEST(ConcatView, MiddleRangeEmpty) {
    std::vector<int> a{1, 2};
    std::vector<int> b{};
    std::vector<int> c{3, 4};
    std::vector<int> result;
    for (int x : views::concat(a, b, c)) result.push_back(x);
    EXPECT_EQ(result, (std::vector<int>{1, 2, 3, 4}));
}

TEST(ConcatView, AllRangesEmpty) {
    std::vector<int> a{}, b{}, c{};
    std::vector<int> result;
    for (int x : views::concat(a, b, c)) result.push_back(x);
    EXPECT_TRUE(result.empty());
}

TEST(ConcatView, SingleRange) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> result;
    for (int x : views::concat(a)) result.push_back(x);
    EXPECT_EQ(result, (std::vector<int>{1, 2, 3}));
}

TEST(ConcatView, VectorAndArray) {
    std::vector<int> a{1, 2};
    std::array<int, 3> b{3, 4, 5};
    std::vector<int> result;
    for (int x : views::concat(a, b)) result.push_back(x);
    EXPECT_EQ(result, (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST(ConcatView, RangesAlgorithmEqual) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{4, 5, 6};
    std::vector<int> expected{1, 2, 3, 4, 5, 6};
    EXPECT_TRUE(std::ranges::equal(views::concat(a, b), expected));
}

TEST(ConcatView, RangesCountIf) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{4, 5, 6};
    auto evens = std::ranges::count_if(views::concat(a, b), [](int x) { return x % 2 == 0; });
    EXPECT_EQ(evens, 3);
}

TEST(ConcatView, RangesFind) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{4, 5, 6};
    auto c = views::concat(a, b);
    auto it = std::ranges::find(c, 4);
    EXPECT_NE(it, std::ranges::end(c));
    EXPECT_EQ(*it, 4);
}

TEST(ConcatView, PipeWithFilter) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{4, 5, 6};
    std::vector<int> result;
    for (int x : views::concat(a, b) | std::views::filter([](int x) { return x % 2 == 0; })) {
        result.push_back(x);
    }
    EXPECT_EQ(result, (std::vector<int>{2, 4, 6}));
}

TEST(ConcatView, PipeWithTransform) {
    std::vector<int> a{1, 2};
    std::vector<int> b{3, 4};
    std::vector<int> result;
    for (int x : views::concat(a, b) | std::views::transform([](int x) { return x * 2; })) {
        result.push_back(x);
    }
    EXPECT_EQ(result, (std::vector<int>{2, 4, 6, 8}));
}

TEST(ConcatView, ListRanges) {
    std::list<int> a{1, 2, 3};
    std::list<int> b{4, 5};
    std::vector<int> result;
    for (int x : views::concat(a, b)) result.push_back(x);
    EXPECT_EQ(result, (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST(ConcatView, StringViews) {
    std::string a = "Hello";
    std::string b = " World";
    std::string result;
    for (char c : views::concat(a, b)) result.push_back(c);
    EXPECT_EQ(result, "Hello World");
}

TEST(ConcatView, EmptyHelper) {
    std::vector<int> a{};
    std::vector<int> b{};
    EXPECT_TRUE(views::concat(a, b).empty());

    std::vector<int> c{1};
    EXPECT_FALSE(views::concat(a, c).empty());
}

TEST(ConcatView, ConstIteration) {
    std::vector<int> a{1, 2};
    std::vector<int> b{3, 4};
    const auto view = views::concat(a, b);
    std::vector<int> result;
    for (int x : view) result.push_back(x);
    EXPECT_EQ(result, (std::vector<int>{1, 2, 3, 4}));
}

TEST(ConcatView, Temporaries) {
    std::vector<int> result;
    for (int x : views::concat(std::vector<int>{1, 2}, std::vector<int>{3, 4})) {
        result.push_back(x);
    }
    EXPECT_EQ(result, (std::vector<int>{1, 2, 3, 4}));
}

TEST(ConcatView, RangesCopy) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{4, 5};
    std::vector<int> result;
    std::ranges::copy(views::concat(a, b), std::back_inserter(result));
    EXPECT_EQ(result, (std::vector<int>{1, 2, 3, 4, 5}));
}

}  // namespace inviwo
