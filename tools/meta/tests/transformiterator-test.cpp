/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>

#include <inviwo/meta/iter/transformiterator.hpp>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

using namespace inviwo::meta;

using foo = std::pair<int, int>;

struct bar {
    int x;
    void inc() { ++x; }
};
bool operator==(const bar& a, const bar& b) { return a.x == b.x; }

TEST(Transformiterator, plain) {
    std::vector<int> a = {2, 4, 6, 8};

    auto trans = [](const int& k) -> double { return k * 2.0; };
    auto begin = makeTransformIterator(trans, a.begin());
    auto end = makeTransformIterator(trans, a.end());

    std::vector<double> res;
    std::copy(begin, end, std::back_inserter(res));

    const auto exp = std::vector<double>{4.0, 8.0, 12.0, 16.0};
    EXPECT_EQ(res, exp);
}

TEST(Transformiterator, pair) {
    std::vector<foo> a = {{2, 2}, {4, 4}, {6, 6}, {8, 8}};

    auto trans = [](foo& k) -> int& { return k.first; };
    auto begin = makeTransformIterator(trans, a.begin());
    auto end = makeTransformIterator(trans, a.end());

    std::for_each(begin, end, [](int& x) { x *= 2; });
    const auto exp = std::vector<foo>{{4, 2}, {8, 4}, {12, 6}, {16, 8}};
    EXPECT_EQ(a, exp);
}

TEST(Transformiterator, Class) {
    std::vector<bar> a = {{2}, {4}, {6}, {8}};

    auto trans = [](bar& i) -> bar& { return i; };
    auto begin = makeTransformIterator(trans, a.begin());
    auto end = makeTransformIterator(trans, a.end());

    for (auto it = begin; it != end; ++it) {
        it->inc();
    }

    std::for_each(begin, end, [](bar& x) { x.inc(); });
    const auto exp = std::vector<bar>{{4}, {6}, {8}, {10}};
    EXPECT_EQ(a, exp);
}
