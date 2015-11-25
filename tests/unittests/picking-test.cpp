/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/util/stdextensions.h>

#include <unordered_set>

namespace inviwo{


TEST(PickingTests, GenerateColor0) {
    auto c0 = PickingManager::indexToColor(0);
    EXPECT_EQ(c0, uvec3(19, 177, 59));
}

TEST(PickingTests, GenerateColor1) {
    auto c1 = PickingManager::indexToColor(1);
    EXPECT_EQ(c1, uvec3(39, 98, 118));
}
TEST(PickingTests, GenerateIndex0) {
    auto i0 = PickingManager::colorToIndex(uvec3(19, 177, 59));
    EXPECT_EQ(i0, 0);
}
TEST(PickingTests, GenerateIndex1) {
    auto i1 = PickingManager::colorToIndex(uvec3(39, 98, 118));
    EXPECT_EQ(i1, 1);
}

TEST(PickingTests, GenerateLots) {
    for(size_t i = 0; i < 1000000; ++i) {
        auto c = PickingManager::indexToColor(i);
        auto ind = PickingManager::colorToIndex(c);
        EXPECT_EQ(ind, i);
    }
}

TEST(PickingTests, Unique) {
    std::unordered_set<uvec3> colors;

    for (size_t i = 0; i < 1000000; ++i) {
        auto c = PickingManager::indexToColor(i);
        colors.insert(c);
    }

    EXPECT_EQ(colors.size(), 1000000);
}










}