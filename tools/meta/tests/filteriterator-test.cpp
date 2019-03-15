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

#include <inviwo/meta/iter/filteriterator.hpp>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

using namespace inviwo::meta;

TEST(filteriterator, basic) {
    std::vector<int> a = {1, 2, 3, 4, 5, 6, 7, 8};
    auto pred = [](const int& i) { return i % 2 == 0; };
    auto begin = makeFilterIterator(pred, a.begin(), a.end());
    auto end = makeFilterIterator(pred, a.end(), a.end());

    std::vector<int> res;
    std::copy(begin, end, std::back_inserter(res));

    auto res2 = std::vector<int>{2, 4, 6, 8};
    EXPECT_EQ(res, res2);
}

TEST(filteriterator, end) {
    std::vector<int> a = {1, 2, 3, 4, 5, 6, 7, 8};
    auto begin = makeFilterIterator([](const int& i) { return i % 2 == 0; }, a.begin(), a.end());
    auto end = begin.end();

    std::vector<int> res;
    std::copy(begin, end, std::back_inserter(res));

    auto res2 = std::vector<int>{2, 4, 6, 8};
    EXPECT_EQ(res, res2);
}

TEST(filteriterator, range) {
    std::vector<int> a = {1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<int> res;
    for (auto& i :
         makeFilterIterator([](const int& k) { return k % 2 == 0; }, a.begin(), a.end())) {
        res.push_back(i);
    }
    auto res2 = std::vector<int>{2, 4, 6, 8};
    EXPECT_EQ(res, res2);
}
