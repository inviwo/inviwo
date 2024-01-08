/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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
#include <gmock/gmock.h>
#include <warn/pop>

#include <inviwo/core/util/brickiterator.h>

#include <inviwo/core/util/indexmapper.h>

#include <numeric>

using ::testing::ElementsAre;

namespace inviwo {

TEST(BrickIteratorTest, subBlock) {
    std::vector<int> data(27);
    std::iota(data.begin(), data.end(), 0);

    auto it =
        util::BrickIterator{data.begin(), size3_t{3, 3, 3}, size3_t{1, 1, 1}, size3_t{2, 2, 2}};

    std::vector<int> block(it, it.end());

    EXPECT_EQ(8, std::distance(it, it.end()));

    EXPECT_THAT(block, ElementsAre(13, 14, 16, 17, 22, 23, 25, 26));
}

TEST(BrickIteratorTest, decrement) {
    std::vector<int> data(27);
    std::iota(data.begin(), data.end(), 0);

    auto it =
        util::BrickIterator{data.begin(), size3_t{3, 3, 3}, size3_t{0, 0, 0}, size3_t{1, 1, 1}};
    EXPECT_EQ(*data.begin(), *it);
    EXPECT_EQ(*data.begin(), *--it.end());

    auto it2 =
        util::BrickIterator{data.begin(), size3_t{3, 3, 3}, size3_t{1, 1, 1}, size3_t{2, 2, 2}};
    EXPECT_EQ(*std::next(it2, 7), *--it2.end());
}

TEST(BrickIteratorTest, extractVoxel) {
    std::vector<int> data(27);
    std::iota(data.begin(), data.end(), 0);
    util::IndexMapper3D im(size3_t{3, 3, 3});

    auto it =
        util::BrickIterator{data.begin(), size3_t{3, 3, 3}, size3_t{0, 0, 0}, size3_t{1, 1, 1}};

    EXPECT_EQ(*data.begin(), *it);

    it = util::BrickIterator{data.begin(), size3_t{3, 3, 3}, size3_t{2, 0, 0}, size3_t{1, 1, 1}};
    EXPECT_EQ(*(data.begin() + im({2, 0, 0})), *it);

    it = util::BrickIterator{data.begin(), size3_t{3, 3, 3}, size3_t{2, 2, 0}, size3_t{1, 1, 1}};
    EXPECT_EQ(*(data.begin() + im({2, 2, 0})), *it);

    it = util::BrickIterator{data.begin(), size3_t{3, 3, 3}, size3_t{2, 2, 2}, size3_t{1, 1, 1}};
    EXPECT_EQ(*(data.begin() + im({2, 2, 2})), *it);
}

TEST(BrickIteratorTest, entireBlock) {
    std::vector<int> data(27);
    std::iota(data.begin(), data.end(), 0);

    auto it =
        util::BrickIterator{data.begin(), size3_t{3, 3, 3}, size3_t{0, 0, 0}, size3_t{3, 3, 3}};
    std::vector<int> block(it, it.end());

    EXPECT_EQ(block, data) << "extracted block is different from source data";
}

}  // namespace inviwo
