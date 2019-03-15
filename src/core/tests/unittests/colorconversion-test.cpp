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

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

using namespace color;

TEST(colorconversion, rgba2hex) {
    EXPECT_EQ("#aabbccdd", rgba2hex(vec4(0xaa, 0xbb, 0xcc, 0xdd) / 255.0f));
    EXPECT_EQ("#0a0b0c0d", rgba2hex(vec4(0x0a, 0x0b, 0x0c, 0x0d) / 255.0f));
    EXPECT_EQ("#d0c0b0a0", rgba2hex(vec4(0xd0, 0xc0, 0xb0, 0xa0) / 255.0f));
    EXPECT_EQ("#00000001", rgba2hex(vec4(0x00, 0x00, 0x00, 0x01) / 255.0f));
    EXPECT_EQ("#10000000", rgba2hex(vec4(0x10, 0x00, 0x00, 0x00) / 255.0f));
}

TEST(colorconversion, rgb2hex) {
    EXPECT_EQ("#aabbcc", rgb2hex(vec3(0xaa, 0xbb, 0xcc) / 255.0f));
    EXPECT_EQ("#0a0b0c", rgb2hex(vec3(0x0a, 0x0b, 0x0c) / 255.0f));
    EXPECT_EQ("#c0b0a0", rgb2hex(vec3(0xc0, 0xb0, 0xa0) / 255.0f));
    EXPECT_EQ("#000001", rgb2hex(vec3(0x00, 0x00, 0x01) / 255.0f));
    EXPECT_EQ("#100000", rgb2hex(vec3(0x10, 0x00, 0x00) / 255.0f));
}

TEST(colorconversion, hex2rgba) {
    EXPECT_THROW(hex2rgba("ffaabbcc"), Exception);
    EXPECT_THROW(hex2rgba("#aa"), Exception);
    EXPECT_THROW(hex2rgba("#aaffc"), Exception);
    EXPECT_THROW(hex2rgba("#aaffccdda"), Exception);
    EXPECT_THROW(hex2rgba("#test"), Exception);

    EXPECT_EQ(vec4(0xaa, 0xbb, 0xcc, 0xff) / 255.0f, hex2rgba("#abc"));
    EXPECT_EQ(vec4(0xaa, 0xbb, 0xcc, 0xdd) / 255.0f, hex2rgba("#abcd"));

    EXPECT_EQ(vec4(0xaa, 0xbb, 0xcc, 0xff) / 255.0f, hex2rgba("#aabbcc"));
    EXPECT_EQ(vec4(0xaa, 0xbb, 0xcc, 0xdd) / 255.0f, hex2rgba("#aabbccdd"));
    EXPECT_EQ(vec4(0x0a, 0x0b, 0x0c, 0x0d) / 255.0f, hex2rgba("#0a0b0c0d"));
    EXPECT_EQ(vec4(0xd0, 0xc0, 0xb0, 0xa0) / 255.0f, hex2rgba("#d0c0b0a0"));
    EXPECT_EQ(vec4(0x00, 0x00, 0x00, 0x01) / 255.0f, hex2rgba("#00000001"));
    EXPECT_EQ(vec4(0x10, 0x00, 0x00, 0x00) / 255.0f, hex2rgba("#10000000"));
}

TEST(colorconversion, rgba2hexIdentity) {
    EXPECT_EQ("#aabbccdd", rgba2hex(hex2rgba("#aabbccdd")));
    EXPECT_EQ("#0a0b0c0d", rgba2hex(hex2rgba("#0a0b0c0d")));
    EXPECT_EQ("#d0c0b0a0", rgba2hex(hex2rgba("#d0c0b0a0")));
    EXPECT_EQ("#00000001", rgba2hex(hex2rgba("#00000001")));
    EXPECT_EQ("#10000000", rgba2hex(hex2rgba("#10000000")));
}

TEST(colorconversion, rgb2hexIdentity) {
    EXPECT_EQ("#aabbcc", rgb2hex(hex2rgba("#aabbcc")));
    EXPECT_EQ("#0a0b0c", rgb2hex(hex2rgba("#0a0b0c")));
    EXPECT_EQ("#c0b0a0", rgb2hex(hex2rgba("#c0b0a0")));
    EXPECT_EQ("#000001", rgb2hex(hex2rgba("#000001")));
    EXPECT_EQ("#100000", rgb2hex(hex2rgba("#100000")));
}

}  // namespace inviwo
