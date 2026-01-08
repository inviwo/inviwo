/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025-2026 Inviwo Foundation
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

#include <inviwo/core/algorithm/boundingbox.h>

namespace inviwo {

TEST(boundingBox, bboxUnion) {
    const mat4 m{1.0f};
    EXPECT_EQ(util::boundingBoxUnion(m, std::nullopt), m);
    EXPECT_EQ(util::boundingBoxUnion(std::nullopt, m), m);
    EXPECT_EQ(util::boundingBoxUnion(m, m), m);
}

TEST(bboxMinExtent, regular) {
    const mat4 m{1.0f};
    EXPECT_EQ(util::minExtentBoundingBox(m), m);
}

TEST(bboxMinExtent, zeroBasis) {
    const mat4 bbox = util::minExtentBoundingBox(mat4{0.0f});
    const mat3 basis{bbox};
    EXPECT_EQ(glm::length(bbox[0]), 1.0f);
    EXPECT_EQ(glm::length(bbox[1]), 1.0f);
    EXPECT_EQ(glm::length(bbox[2]), 1.0f);
}

TEST(bboxMinExtent, zeroBasisVectorA) {
    const mat4 m{
        vec4{0.0f},
        vec4{1.0f, 0.0f, 0.0f, 0.0f},
        vec4{0.0f, 1.0f, 0.0f, 0.0f},
        vec4{0.0f, 0.0f, 0.0f, 1.0f},
    };

    const mat4 bbox = util::minExtentBoundingBox(m);
    const mat3 basis{bbox};
    EXPECT_GT(glm::length(bbox[0]), 0.0f);
    EXPECT_EQ(glm::length(bbox[1]), 1.0f);
    EXPECT_EQ(glm::length(bbox[2]), 1.0f);
}

TEST(bboxMinExtent, zeroBasisVectorB) {
    const mat4 m{
        vec4{1.0f, 0.0f, 0.0f, 0.0f},
        vec4{0.0f},
        vec4{0.0f, 1.0f, 0.0f, 0.0f},
        vec4{0.0f, 0.0f, 0.0f, 1.0f},
    };

    const mat4 bbox = util::minExtentBoundingBox(m);
    const mat3 basis{bbox};
    EXPECT_EQ(glm::length(bbox[0]), 1.0f);
    EXPECT_GT(glm::length(bbox[1]), 0.0f);
    EXPECT_EQ(glm::length(bbox[2]), 1.0f);
}

TEST(bboxMinExtent, zeroBasisVectorC) {
    const mat4 m{
        vec4{1.0f, 0.0f, 0.0f, 0.0f},
        vec4{0.0f, 1.0f, 0.0f, 0.0f},
        vec4{0.0f},
        vec4{0.0f, 0.0f, 0.0f, 1.0f},
    };

    const mat4 bbox = util::minExtentBoundingBox(m);
    const mat3 basis{bbox};
    EXPECT_EQ(glm::length(bbox[0]), 1.0f);
    EXPECT_EQ(glm::length(bbox[1]), 1.0f);
    EXPECT_GT(glm::length(bbox[2]), 0.0f);
}

TEST(bboxMinExtent, zeroBasisVectorsAC) {
    const mat4 m{
        vec4{0.0f},
        vec4{1.0f, 0.0f, 0.0f, 0.0f},
        vec4{0.0f},
        vec4{0.0f, 0.0f, 0.0f, 1.0f},
    };
    const mat4 bbox = util::minExtentBoundingBox(m);
    const mat3 basis{bbox};
    EXPECT_GT(glm::length(bbox[0]), 0.0f);
    EXPECT_EQ(glm::length(bbox[1]), 1.0f);
    EXPECT_GT(glm::length(bbox[2]), 0.0f);
}

TEST(bboxMinExtent, zeroBasisVectorsBC) {
    const mat4 m{
        vec4{1.0f, 0.0f, 0.0f, 0.0f},
        vec4{0.0f},
        vec4{0.0f},
        vec4{0.0f, 0.0f, 0.0f, 1.0f},
    };
    const mat4 bbox = util::minExtentBoundingBox(m);
    const mat3 basis{bbox};
    EXPECT_EQ(glm::length(bbox[0]), 1.0f);
    EXPECT_GT(glm::length(bbox[1]), 0.0f);
    EXPECT_GT(glm::length(bbox[2]), 0.0f);
}

}  // namespace inviwo
