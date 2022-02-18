/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <modules/discretedata/channels/extendedchannel.h>
#include <modules/discretedata/channels/analyticchannel.h>

namespace inviwo {
namespace discretedata {

TEST(DataSet, ExtendedChannel) {
    using IArr5 = std::array<int, 5>;

    // Set up two channels, extend into one.
    auto baseChannel = std::make_shared<AnalyticChannel<int, 3, ivec3>>(
        [](ivec3& a, ind idx) {
            a[0] = idx * 3;
            a[1] = idx * 3 + 1;
            a[2] = idx * 3 + 2;
        },
        4, "AscendingNumbers", GridPrimitive::Vertex);
    auto extendChannel = std::make_shared<AnalyticChannel<int, 2, ivec2>>(
        [](ivec2& a, ind idx) {
            a[0] = -idx * 2;
            a[1] = -idx * 2 - 1;
        },
        3, "DescendingNumbers", GridPrimitive::Vertex);

    auto extendedChannel = createExtendedChannel(baseChannel, extendChannel, "ExtendedChannel");

    // Check type of extended channel.
    EXPECT_EQ(extendedChannel->getNumComponents(), 5);
    EXPECT_EQ(extendedChannel->getDataFormatId(), baseChannel->getDataFormatId());
    auto extendedChannelTyped = std::dynamic_pointer_cast<ExtendedChannel<int, 5>>(extendedChannel);

    EXPECT_TRUE(extendedChannelTyped);
    EXPECT_EQ(extendedChannel->size(), 12);

    // Lookup table.
    constexpr std::array<IArr5, 12> expectedValues = {
        IArr5{0, 1, 2, 0, -1},  IArr5{3, 4, 5, 0, -1},
        IArr5{6, 7, 8, 0, -1},  IArr5{9, 10, 11, 0, -1},

        IArr5{0, 1, 2, -2, -3}, IArr5{3, 4, 5, -2, -3},
        IArr5{6, 7, 8, -2, -3}, IArr5{9, 10, 11, -2, -3},

        IArr5{0, 1, 2, -4, -5}, IArr5{3, 4, 5, -4, -5},
        IArr5{6, 7, 8, -4, -5}, IArr5{9, 10, 11, -4, -5}};

    // Set up test cases for different starting indices and sizes.
    // Compare to lookup table.
    IArr5* sampledValues = new IArr5[12];

    constexpr std::array<ivec2, 5> testCasesAsIndexAndSize = {
        ivec2{0, 4}, ivec2{0, 12}, ivec2{5, 7}, ivec2{5, 5}, ivec2{1, 9}};

    for (auto& testCase : testCasesAsIndexAndSize) {
        int startIndex = testCase.x;
        int numElements = testCase.y;
        std::fill_n(sampledValues, 12, IArr5{42, 42, 42, 42, 42});
        extendedChannelTyped->fill(sampledValues[startIndex], startIndex, numElements);
        SCOPED_TRACE(fmt::format("In range [{}, {})", startIndex, startIndex + numElements));

        for (int idx = startIndex; idx < startIndex + numElements; ++idx) {
            SCOPED_TRACE(fmt::format("In index {}", idx));
            for (size_t comp = 0; comp < 5; ++comp) {
                SCOPED_TRACE(fmt::format("In component {}", comp));
                EXPECT_EQ(sampledValues[idx][comp], expectedValues[idx][comp]);
            }
        }
    }

    delete[] sampledValues;
}

}  // namespace discretedata
}  // namespace inviwo
