/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <inviwo/core/util/interpolation.h>

namespace inviwo {

TEST(InterpolationTest, LinerInterpolationDoubles) {
    using Inter = Interpolation<double, double>;

    EXPECT_DOUBLE_EQ(Inter::linear(0.0, 1.0, 0.0), 0.0);
    EXPECT_DOUBLE_EQ(Inter::linear(0.0, 1.0, 1.0), 1.0);
    EXPECT_DOUBLE_EQ(Inter::linear(0.0, 1.0, 0.5), 0.5);
    EXPECT_DOUBLE_EQ(Inter::linear(0.0, 1.0, 0.1), 0.1);
    EXPECT_DOUBLE_EQ(Inter::linear(0.0, 1.0, 0.00001), 0.00001);
    EXPECT_DOUBLE_EQ(Inter::linear(0.0, 1.0, 0.99999), 0.99999);

    EXPECT_DOUBLE_EQ(Inter::linear(0.0, 1.0, -1.0), -1.0);
    EXPECT_DOUBLE_EQ(Inter::linear(0.0, 1.0, -2.0), -2.0);
    EXPECT_DOUBLE_EQ(Inter::linear(0.0, 1.0, 2.0), 2.0);
    EXPECT_DOUBLE_EQ(Inter::linear(0.0, 1.0, 3.0), 3.0);
}

TEST(InterpolationTest, LinerInterpolationChar) {
    using Inter = Interpolation<unsigned char, double>;

    EXPECT_EQ(Inter::linear(0, 255, 0.0), 0);
    EXPECT_EQ(Inter::linear(0, 255, 0.5), 127);
    EXPECT_EQ(Inter::linear(0, 255, 1.0), 255);

    EXPECT_EQ(Inter::linear(237, 34, 0.5), 135);
    EXPECT_NE(Inter::linear(237, 34, 0.5), 7)
        << "If this is true we are likely using wrong types when interpolating";
}

TEST(InterpolationTest, LinerInterpolationCharVec) {
    using u8vec3 = glm::u8vec3;
    using Inter = Interpolation<u8vec3, double>;

    // helpers to print numberical values instead of chars incase of error
    auto expect_eq = [](u8vec3 a, u8vec3 b, std::string msg = "") {
        using u16vec3 = glm::u16vec3;
        if (msg != "") {
            EXPECT_EQ(u16vec3(a), u16vec3(b)) << msg;
        } else {
            EXPECT_EQ(u16vec3(a), u16vec3(b));
        }
    };

    expect_eq(Inter::linear(u8vec3(0), u8vec3(255), 0.0), u8vec3(0));
    expect_eq(Inter::linear(u8vec3(0), u8vec3(255), 0.5), u8vec3(127));
    expect_eq(Inter::linear(u8vec3(0), u8vec3(255), 1.0), u8vec3(255));

    // This was equal to 7 before fix, since the results if subtracting two unsigned chars is int
    expect_eq(Inter::linear(u8vec3(237), u8vec3(34), 0.5), u8vec3(135),
              "If this equals to seven, the interpolation is likley using wrong type while "
              "interpolating");
}

TEST(InterpolationTest, LinerInterpolationShortVec) {
    using u16vec3 = glm::u16vec3;
    using Inter = Interpolation<u16vec3, double>;

    EXPECT_EQ(Inter::linear(u16vec3(0), u16vec3(255), 0.0), u16vec3(0));
    EXPECT_EQ(Inter::linear(u16vec3(0), u16vec3(255), 0.5), u16vec3(127));
    EXPECT_EQ(Inter::linear(u16vec3(0), u16vec3(255), 1.0), u16vec3(255));

    EXPECT_EQ(Inter::linear(u16vec3(237), u16vec3(34), 0.5), u16vec3(135));
}

TEST(InterpolationTest, LinerInterpolationUInt32Vec) {
    using u32vec3 = glm::u32vec3;
    using Inter = Interpolation<u32vec3, double>;

    EXPECT_EQ(Inter::linear(u32vec3(0), u32vec3(255), 0.0), u32vec3(0));
    EXPECT_EQ(Inter::linear(u32vec3(0), u32vec3(255), 0.5), u32vec3(127));
    EXPECT_EQ(Inter::linear(u32vec3(0), u32vec3(255), 1.0), u32vec3(255));

    EXPECT_EQ(Inter::linear(u32vec3(237), u32vec3(34), 0.5), u32vec3(135));
}

TEST(InterpolationTest, LinerInterpolationUInt64Vec) {
    using u64vec3 = glm::u64vec3;
    using Inter = Interpolation<u64vec3, double>;

    EXPECT_EQ(Inter::linear(u64vec3(0), u64vec3(255), 0.0), u64vec3(0));
    EXPECT_EQ(Inter::linear(u64vec3(0), u64vec3(255), 0.5), u64vec3(127));
    EXPECT_EQ(Inter::linear(u64vec3(0), u64vec3(255), 1.0), u64vec3(255));

    EXPECT_EQ(Inter::linear(u64vec3(237), u64vec3(34), 0.5), u64vec3(135));
}
}  // namespace inviwo
