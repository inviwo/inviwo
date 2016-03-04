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

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/glm.h>

namespace inviwo {

using namespace util;

TEST(ConversionTests, DoubleToUInt8) {
    auto res = glm_convert_normalized<unsigned char>(1.0);
    EXPECT_EQ(res, std::numeric_limits<unsigned char>::max());
}

TEST(ConversionTests, DoubleToUInt16) {
    auto res = glm_convert_normalized<unsigned short>(1.0);
    EXPECT_EQ(res, std::numeric_limits<unsigned short>::max());
}

TEST(ConversionTests, DoubleToUInt32) {
    auto res = glm_convert_normalized<unsigned int>(1.0);
    EXPECT_EQ(res, std::numeric_limits<unsigned int>::max());
}

TEST(ConversionTests, DoubleToUInt64) {
    auto res = glm_convert_normalized<unsigned long>(1.0);
    EXPECT_EQ(res, std::numeric_limits<unsigned long>::max());
}

TEST(ConversionTests, UInt8ToUInt8) {
    unsigned char a{255};
    auto res = glm_convert_normalized<unsigned char>(a);
    EXPECT_EQ(res, std::numeric_limits<unsigned char>::max());
}

TEST(ConversionTests, UInt8ToUInt16) {
    unsigned char a{255};
    auto res = glm_convert_normalized<unsigned short>(a);
    EXPECT_EQ(res, std::numeric_limits<unsigned short>::max());
}

TEST(ConversionTests, UInt8ToUInt32) {
    unsigned char a{255};
    auto res = glm_convert_normalized<unsigned int>(a);
    EXPECT_EQ(res, std::numeric_limits<unsigned int>::max());
}

TEST(ConversionTests, UInt8ToUInt64) {
    unsigned char a{255};
    auto res = glm_convert_normalized<unsigned long long>(a);
    EXPECT_EQ(res, std::numeric_limits<unsigned long long>::max());
}

}