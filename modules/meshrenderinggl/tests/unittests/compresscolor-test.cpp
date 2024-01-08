/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <inviwo/core/util/glmvec.h>

#include <cstring>

namespace inviwo {

unsigned int compressColor(glm::vec3 color) {
    unsigned int c = (int((color.r * 1023)) & 0x3ff) << 20;
    c += (int((color.g * 1023)) & 0x3ff) << 10;
    c += (int((color.b * 1023)) & 0x3ff);
    return c;
}
glm::vec3 uncompressColor(unsigned int c) {
    glm::vec3 color;
    color.r = float((c >> 20) & 0x3ff) / 1023.0f;
    color.g = float((c >> 10) & 0x3ff) / 1023.0f;
    color.b = float(c & 0x3ff) / 1023.0f;
    return color;
}

struct ColorPack {
    unsigned int b : 10;
    unsigned int g : 10;
    unsigned int r : 10;
    unsigned int unused : 2;
};

static_assert(sizeof(ColorPack) == 4);

TEST(CompressColor, colorpack) {

    auto compress = compressColor(glm::vec3{1.f, 0.5f, 0.0f});
    auto color1 = uncompressColor(compress);

    ColorPack color2;
    std::memcpy(&color2, &compress, 4);

    glm::vec3 color3{color2.r / 1023.0f, color2.g / 1023.0f, color2.b / 1023.0f};

    EXPECT_EQ(color1, color3);
}

}  // namespace inviwo
