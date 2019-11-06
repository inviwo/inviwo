/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/base/algorithm/mesh/meshclipping.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <modules/base/algorithm/meshutils.h>

#include <glm/gtx/perpendicular.hpp>

namespace inviwo {

TEST(MeshCutting, BarycentricInsidePolygon) {

    auto testCorners = [](const std::vector<vec2>& poly) {
        for (size_t i = 0; i < poly.size(); ++i) {
            const auto res = meshutil::detail::barycentricInsidePolygon(poly[i], poly);
            for (size_t j = 0; j < res.size(); ++j) {
                if (i == j) {
                    EXPECT_FLOAT_EQ(res[j], 1.0f);
                } else {
                    EXPECT_FLOAT_EQ(res[j], 0.0f);
                }
            }
        }
    };

    auto testCenter = [](const std::vector<vec2>& poly) {
        const auto res = meshutil::detail::barycentricInsidePolygon(vec2{0.5, 0.5}, poly);
        for (size_t j = 0; j < res.size(); ++j) {
            EXPECT_FLOAT_EQ(res[j], 1.0f / res.size());
        }
    };

    std::vector<vec2> poly2{{0.0f, 0.0f}, {1.0f, 1.0f}};
    testCorners(poly2);
    testCenter(poly2);

    std::vector<vec2> poly3{{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}};
    testCorners(poly3);

    std::vector<vec2> poly4{{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}};
    testCorners(poly4);
    testCenter(poly4);
}

TEST(MeshCutting, SutherlandHodgman) {

    const glm::u32vec3 triangle{0, 1, 2};
    const Plane plane{vec3{0, 0, 0}, vec3{0, 1, 0}};
    std::vector<vec3> positions{vec3{-1, -1, 0}, vec3{1, -1, 0}, vec3{0, 1, 0}};
    std::vector<std::uint32_t> indicesVec{};
    const meshutil::detail::InterpolateFunctor addInterpolatedVertex =
        [&](const std::vector<uint32_t>& indices, const std::vector<float>& weights,
            std::optional<vec3>) -> uint32_t {
        const auto val = std::inner_product(
            indices.begin(), indices.end(), weights.begin(), vec3{0}, std::plus<>{},
            [&](uint32_t index, float weight) { return positions[index] * weight; });

        positions.push_back(val);
        return static_cast<uint32_t>(positions.size() - 1);
    };

    auto newEdge = meshutil::detail::sutherlandHodgman(triangle, plane, positions, indicesVec,
                                                       addInterpolatedVertex);

    ASSERT_TRUE(newEdge);
    EXPECT_EQ((*newEdge)[0], 3);
    EXPECT_EQ((*newEdge)[1], 4);

    ASSERT_EQ(indicesVec.size(), 3);
    EXPECT_EQ(indicesVec[0], 3);
    EXPECT_EQ(indicesVec[1], 2);
    EXPECT_EQ(indicesVec[2], 4);

    ASSERT_EQ(positions.size(), 5);

    EXPECT_FLOAT_EQ(positions[3][0], 0.5f);
    EXPECT_FLOAT_EQ(positions[3][1], 0.0f);

    EXPECT_FLOAT_EQ(positions[4][0], -0.5f);
    EXPECT_FLOAT_EQ(positions[4][1], 0.0f);
}

TEST(MeshCutting, GatherLoops) {
    const std::vector<vec3> positions{vec3{-1, -1, 0}, vec3{1, -1, 0}, vec3{0, 1, 0}};
    std::vector<glm::u32vec2> edges{{0, 1}, {1, 2}, {2, 0}};

    const auto loops = meshutil::detail::gatherLoops(edges, positions, 0.0000001f);

    ASSERT_EQ(loops.size(), 1);
    ASSERT_EQ(loops[0].size(), 3);
}

TEST(MeshCutting, PolygonCentroid) {

    const auto expected = vec2{0.5f, 0.5f};
    {
        const std::vector<vec2> polygon = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
        const auto centroid = meshutil::detail::polygonCentroid(polygon);

        EXPECT_FLOAT_EQ(centroid.x, expected.x);
        EXPECT_FLOAT_EQ(centroid.y, expected.y);
    }
    {
        const std::vector<vec2> polygon = {
            {0.0f, 0.0f}, {0.1f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
        const auto centroid = meshutil::detail::polygonCentroid(polygon);

        EXPECT_FLOAT_EQ(centroid.x, expected.x);
        EXPECT_FLOAT_EQ(centroid.y, expected.y);
    }
    {
        const std::vector<vec2> polygon = {{0.0f, 0.1f}, {0.0f, 0.0f}, {0.1f, 0.0f},
                                           {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
        const auto centroid = meshutil::detail::polygonCentroid(polygon);

        EXPECT_FLOAT_EQ(centroid.x, expected.x);
        EXPECT_FLOAT_EQ(centroid.y, expected.y);
    }
}

TEST(MeshCutting, PlaneBasis) {
    const Plane p{vec3{1, 1, 1}, vec3{0, 0, 1}};
    const auto trans = glm::inverse(p.inPlaneBasis());

    {
        const auto r = vec3{1, 1, 1};
        const auto t = vec3{trans * vec4{r, 1.0f}};
        const auto expected = vec3{0.0f, 0.0f, 0.0f};

        EXPECT_FLOAT_EQ(t.x, expected.x);
        EXPECT_FLOAT_EQ(t.y, expected.y);
        EXPECT_FLOAT_EQ(t.z, expected.z);
    }

    {
        const auto r = vec3{1, 1, 2};
        const auto t = vec3{trans * vec4{r, 1.0f}};
        const auto expected = vec3{0.0f, 0.0f, 1.0f};

        EXPECT_FLOAT_EQ(t.x, expected.x);
        EXPECT_FLOAT_EQ(t.y, expected.y);
        EXPECT_FLOAT_EQ(t.z, expected.z);
    }

    {
        const auto r = vec3{4, 2, 1};
        const auto t = vec3{trans * vec4{r, 1.0f}};
        EXPECT_FLOAT_EQ(t.z, 0.0f);
    }
}

}  // namespace inviwo
