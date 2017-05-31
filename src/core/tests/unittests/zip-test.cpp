/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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

#include <vector>
#include <numeric>
#include <tuple>
#include <iostream>
#include <memory>

#include <inviwo/core/util/zip.h>


namespace inviwo{

TEST(ZipIterTest, Minimal) {
    std::vector<int> a(10);
    std::iota(a.begin(), a.end(), 0);
    int count = 0;
    for (auto&& i : util::zip(a)) {
        EXPECT_EQ(count, std::get<0>(i));
        ++count;
    }
}

TEST(ZipIterTest, Pair) {

    std::vector<int> a(10);
    std::vector<int> b(10);

    std::iota(a.begin(), a.end(), 0);
    std::iota(b.begin(), b.end(), 10);

    int count = 0;
    for (auto&& i : util::zip(a, b)) {
        EXPECT_EQ(count, std::get<0>(i));
        EXPECT_EQ(count+10, std::get<1>(i));
        ++count;
    }
}

TEST(ZipIterTest, NoCopy) {
    struct S {
        S() = delete;
        S(const S&) = delete;
        S(S&&) = delete;
        S& operator=(const S&) = delete;
        S& operator=(S&&) = delete;

        S(int a) : a_{a} {}
        int a_;
    };

    std::vector<std::unique_ptr<S>> a;
    std::vector<int> b;

    for (int i = 0; i<10; ++i) {
        a.push_back(std::make_unique<S>(i));
        b.push_back(i+10);
    }


    int count = 0;
    for (auto&& i : util::zip(a, b)) {
        EXPECT_EQ(count, std::get<0>(i)->a_);
        EXPECT_EQ(count + 10, std::get<1>(i));
        ++count;
    }
}

TEST(ZipIterTest, Ranges) {
    int count = 0;
    auto r1 = util::make_range(0, 10, 1);
    auto r2 = util::make_range(10, 20, 1);
    for (auto&& i : util::zip(r1, r2)) {
        EXPECT_EQ(count, std::get<0>(i));
        EXPECT_EQ(count + 10, std::get<1>(i));
        ++count;
    }
}

TEST(ZipIterTest, DifferenLenght1) {
    int count = 0;
    auto r1 = util::make_range(0, 10, 1);
    auto r2 = util::make_range(10, 30, 1);
    for (auto&& i : util::zip(r1, r2)) {
        EXPECT_EQ(count, std::get<0>(i));
        EXPECT_EQ(count + 10, std::get<1>(i));
        ++count;
    }
}
TEST(ZipIterTest, DifferenLenght2) {
    int count = 0;
    auto r1 = util::make_range(0, 20, 1);
    auto r2 = util::make_range(10, 20, 1);
    for (auto&& i : util::zip(r1, r2)) {
        EXPECT_EQ(count, std::get<0>(i));
        EXPECT_EQ(count + 10, std::get<1>(i));
        ++count;
    }
}

TEST(ZipIterTest, Temporaries) {
    int count = 0;
    for (auto&& i : util::zip(util::make_range(0, 20, 1), util::make_range(10, 20, 1))) {
        EXPECT_EQ(count, std::get<0>(i));
        EXPECT_EQ(count + 10, std::get<1>(i));
        ++count;
    }
}

}  // namespace
