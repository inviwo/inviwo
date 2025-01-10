/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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

#include <inviwo/core/algorithm/permutations.h>

#include <array>
#include <span>

namespace inviwo {

TEST(Permutations, permutations) {

    std::array values = {0, 1, 2};
    util::Permutations<int> perm(values, 1);

    EXPECT_EQ(*perm.begin(), 0);
    EXPECT_EQ(++perm.begin(), perm.end());

    EXPECT_TRUE(perm.next());
    EXPECT_EQ(*perm.begin(), 1);
    EXPECT_EQ(++perm.begin(), perm.end());

    EXPECT_TRUE(perm.next());
    EXPECT_EQ(*perm.begin(), 2);
    EXPECT_EQ(++perm.begin(), perm.end());

    EXPECT_FALSE(perm.next());

    perm = util::Permutations<int>(values, 2);

    auto it = perm.begin();
    EXPECT_EQ(*it, 0);
    EXPECT_EQ(*++it, 1);
    EXPECT_EQ(++it, perm.end());

    it = perm.begin();
    EXPECT_TRUE(perm.next());
    EXPECT_EQ(*it, 0);
    EXPECT_EQ(*++it, 2);
    EXPECT_EQ(++it, perm.end());

    it = perm.begin();
    EXPECT_TRUE(perm.next());
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(*++it, 0);
    EXPECT_EQ(++it, perm.end());

    it = perm.begin();
    EXPECT_TRUE(perm.next());
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(*++it, 2);
    EXPECT_EQ(++it, perm.end());

    it = perm.begin();
    EXPECT_TRUE(perm.next());
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(*++it, 0);
    EXPECT_EQ(++it, perm.end());

    it = perm.begin();
    EXPECT_TRUE(perm.next());
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(*++it, 1);
    EXPECT_EQ(++it, perm.end());

    EXPECT_FALSE(perm.next());

    perm = util::Permutations<int>(values, 3);

    it = perm.begin();
    EXPECT_EQ(*it, 0);
    EXPECT_EQ(*++it, 1);
    EXPECT_EQ(*++it, 2);
    EXPECT_EQ(++it, perm.end());

    it = perm.begin();
    EXPECT_TRUE(perm.next());
    EXPECT_EQ(*it, 0);
    EXPECT_EQ(*++it, 2);
    EXPECT_EQ(*++it, 1);
    EXPECT_EQ(++it, perm.end());

    it = perm.begin();
    EXPECT_TRUE(perm.next());
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(*++it, 0);
    EXPECT_EQ(*++it, 2);
    EXPECT_EQ(++it, perm.end());

    it = perm.begin();
    EXPECT_TRUE(perm.next());
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(*++it, 2);
    EXPECT_EQ(*++it, 0);
    EXPECT_EQ(++it, perm.end());

    it = perm.begin();
    EXPECT_TRUE(perm.next());
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(*++it, 0);
    EXPECT_EQ(*++it, 1);
    EXPECT_EQ(++it, perm.end());

    it = perm.begin();
    EXPECT_TRUE(perm.next());
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(*++it, 1);
    EXPECT_EQ(*++it, 0);
    EXPECT_EQ(++it, perm.end());

    EXPECT_FALSE(perm.next());
}

TEST(Permutations, Combinations) {

    std::array values = {0, 1, 2};
    util::Combinations<int> comb(values, 1);

    EXPECT_EQ(*comb.begin(), 0);
    EXPECT_EQ(++comb.begin(), comb.end());

    EXPECT_TRUE(comb.next());
    EXPECT_EQ(*comb.begin(), 1);
    EXPECT_EQ(++comb.begin(), comb.end());

    EXPECT_TRUE(comb.next());
    EXPECT_EQ(*comb.begin(), 2);
    EXPECT_EQ(++comb.begin(), comb.end());

    EXPECT_FALSE(comb.next());

    comb = util::Combinations<int>(values, 2);

    auto it = comb.begin();
    EXPECT_EQ(*it, 0);
    EXPECT_EQ(*++it, 1);
    EXPECT_EQ(++it, comb.end());

    it = comb.begin();
    EXPECT_TRUE(comb.next());
    EXPECT_EQ(*it, 0);
    EXPECT_EQ(*++it, 2);
    EXPECT_EQ(++it, comb.end());

    it = comb.begin();
    EXPECT_TRUE(comb.next());
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(*++it, 2);
    EXPECT_EQ(++it, comb.end());

    EXPECT_FALSE(comb.next());

    comb = util::Combinations<int>(values, 3);

    it = comb.begin();
    EXPECT_EQ(*it, 0);
    EXPECT_EQ(*++it, 1);
    EXPECT_EQ(*++it, 2);
    EXPECT_EQ(++it, comb.end());

    EXPECT_FALSE(comb.next());
}

TEST(Permutations, IndexProduct) {

    std::array sizes = {3, 2};
    util::IndexProduct<int> inds{sizes};

    auto it = inds.begin();
    EXPECT_EQ(*it, 0);
    EXPECT_EQ(*++it, 0);
    EXPECT_EQ(++it, inds.end());

    it = inds.begin();
    EXPECT_TRUE(inds.next());
    EXPECT_EQ(*it, 0);
    EXPECT_EQ(*++it, 1);
    EXPECT_EQ(++it, inds.end());

    it = inds.begin();
    EXPECT_TRUE(inds.next());
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(*++it, 0);
    EXPECT_EQ(++it, inds.end());

    it = inds.begin();
    EXPECT_TRUE(inds.next());
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(*++it, 1);
    EXPECT_EQ(++it, inds.end());

    it = inds.begin();
    EXPECT_TRUE(inds.next());
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(*++it, 0);
    EXPECT_EQ(++it, inds.end());

    it = inds.begin();
    EXPECT_TRUE(inds.next());
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(*++it, 1);
    EXPECT_EQ(++it, inds.end());

    EXPECT_FALSE(inds.next());
}
}  // namespace inviwo
