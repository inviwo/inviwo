/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <modules/oit/datastructures/halfedges.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

#include <inviwo/core/util/indexmapper.h>

#include <modules/base/algorithm/meshutils.h>

namespace inviwo {

using ::testing::UnorderedElementsAre;
using ::testing::ElementsAre;
using ::testing::Pointee;
using ::testing::Pair;
using ::testing::Eq;
using ::testing::Matches;

/*
 *  y3 ◆ ◀──────▲ ◆ ◀──────▲ ◆ ◀──────▲ ◆ ◀──────▲
 *     │╲ ╲f17  │ │╲ ╲f19  │ │╲ ╲f21  │ │╲ ╲f23  │
 *     │ ╲ ╲    │ │ ╲ ╲    │ │ ╲ ╲    │ │ ╲ ╲    │
 *     │  ╲ ╲   │ │  ╲ ╲   │ │  ╲ ╲   │ │  ╲ ╲   │
 *     │   ╲ ╲  │ │   ╲ ╲  │ │   ╲ ╲  │ │   ╲ ╲  │
 *     │    ╲ ╲ │ │    ╲ ╲ │ │    ╲ ╲ │ │    ╲ ╲ │
 *     │ f16 ╲ ╲│ │ f18 ╲ ╲│ │ f20 ╲ ╲│ │ f22 ╲ ╲│
 *     ▼ ─────▶ ◆ ▼ ─────▶ ◆ ▼ ─────▶ ◆ ▼ ─────▶ ◆
 *  y2 ◆ ◀──────▲ ◆ ◀──────▲ ◆ ◀──────▲ ◆ ◀──────▲
 *     │╲ ╲ f9  │ │╲ ╲f11  │ │╲ ╲f13  │ │╲ ╲f15  │
 *     │ ╲ ╲    │ │ ╲ ╲    │ │ ╲ ╲    │ │ ╲ ╲    │
 *     │  ╲ ╲   │ │  ╲ ╲   │ │  ╲ ╲   │ │  ╲ ╲   │
 *     │   ╲ ╲  │ │   ╲ ╲  │ │   ╲ ╲  │ │   ╲ ╲  │
 *     │    ╲ ╲ │ │    ╲ ╲ │ │    ╲ ╲ │ │    ╲ ╲ │
 *     │  f8 ╲ ╲│ │ f10 ╲ ╲│ │ f12 ╲ ╲│ │ f14 ╲ ╲│
 *     ▼ ─────▶ ◆ ▼ ─────▶ ◆ ▼ ─────▶ ◆ ▼ ─────▶ ◆
 *  y1 ◆ ◀─e4───▲ ◆ ◀──────▲ ◆ ◀──────▲ ◆ ◀──────▲
 *     │╲ ╲ f1  │ │╲ ╲ f3  │ │╲ ╲ f5  │ │╲ ╲ f7  │
 *     │ ╲ ╲    │ │ ╲ ╲    │ │ ╲ ╲    │ │ ╲ ╲    │
 *     │  ╲ e5 e3 │  ╲ ╲   │ │  ╲ ╲   │ │  ╲ ╲   │
 *    e2  e1 ╲  │ │   ╲ ╲  │ │   ╲ ╲  │ │   ╲ ╲  │
 *     │    ╲ ╲ │ │    ╲ ╲ │ │    ╲ ╲ │ │    ╲ ╲ │
 *     │  f0 ╲ ╲│ │  f2 ╲ ╲│ │  f4 ╲ ╲│ │  f6 ╲ ╲│
 *  y0 ▼ ─e0──▶ ◆ ▼ ─────▶ ◆ ▼ ─────▶ ◆ ▼ ─────▶ ◆
 *    x0        x1         x2         x3        x4
 */

IndexBuffer createPlane(int width, int height) {
    IndexBuffer b{};
    auto indices = b.getEditableRAMRepresentation();

    util::IndexMapper<2, std::uint32_t> im{glm::uvec2{width + 1, height + 1}};

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            indices->add(im(x + 0, y + 0));
            indices->add(im(x + 1, y + 0));
            indices->add(im(x + 0, y + 1));

            indices->add(im(x + 1, y + 0));
            indices->add(im(x + 1, y + 1));
            indices->add(im(x + 0, y + 1));
        }
    }

    return b;
}

TEST(HalfEdges, plane) {
    constexpr int width = 4;
    constexpr int height = 3;
    const IndexBuffer plane = createPlane(width, height);
    const auto& index = plane.getRAMRepresentation()->getDataContainer();

    util::IndexMapper<2, std::uint32_t> im{glm::uvec2{width + 1, height + 1}};

    EXPECT_EQ(index.size(), 3 * 2 * width * height);

    HalfEdges edges(Mesh::MeshInfo{DrawType::Triangles, ConnectivityType::None}, plane);

    EXPECT_EQ(std::distance(edges.faces().begin(), edges.faces().end()), 2 * width * height);
    EXPECT_EQ(std::distance(edges.vertices().begin(), edges.vertices().end()),
              (width + 1) * (height + 1));

    // First face
    const auto e0 = edges.faceToEdge(0);
    const auto e1 = e0.next();
    const auto e2 = e1.next();

    EXPECT_EQ(e0.vertex(), im(0, 0));
    EXPECT_EQ(e1.vertex(), im(1, 0));
    EXPECT_EQ(e2.vertex(), im(0, 1));

    EXPECT_EQ(e0, e2.next());
    EXPECT_EQ(e0.prev(), e2);

    EXPECT_EQ(e0.face(), 0);
    EXPECT_EQ(e1.face(), 0);
    EXPECT_EQ(e2.face(), 0);

    EXPECT_FALSE(e0.twin());
    EXPECT_TRUE(e1.twin());
    EXPECT_FALSE(e2.twin());

    // Second face
    const auto e3 = edges.faceToEdge(1);
    const auto e4 = e3.next();
    const auto e5 = e4.next();

    EXPECT_EQ(e3.vertex(), im(1, 0));
    EXPECT_EQ(e4.vertex(), im(1, 1));
    EXPECT_EQ(e5.vertex(), im(0, 1));

    EXPECT_EQ(e3, e5.next());
    EXPECT_EQ(e3.prev(), e5);

    EXPECT_EQ(e3.face(), 1);
    EXPECT_EQ(e4.face(), 1);
    EXPECT_EQ(e5.face(), 1);

    EXPECT_TRUE(e3.twin());
    EXPECT_TRUE(e4.twin());
    EXPECT_TRUE(e5.twin());

    EXPECT_EQ(*e1.twin(), e5);
    EXPECT_EQ(*e5.twin(), e1);

    // Second to Last face
    const auto e66 = edges.faceToEdge(22);
    const auto e67 = e66.next();
    const auto e68 = e67.next();

    EXPECT_EQ(e66.vertex(), im(3, 2));
    EXPECT_EQ(e67.vertex(), im(4, 2));
    EXPECT_EQ(e68.vertex(), im(3, 3));

    EXPECT_EQ(e66, e68.next());
    EXPECT_EQ(e66.prev(), e68);

    EXPECT_EQ(e66.face(), 22);
    EXPECT_EQ(e67.face(), 22);
    EXPECT_EQ(e68.face(), 22);

    EXPECT_TRUE(e66.twin());
    EXPECT_TRUE(e67.twin());
    EXPECT_TRUE(e68.twin());

    // Last face
    const auto e69 = edges.faceToEdge(23);
    const auto e70 = e69.next();
    const auto e71 = e70.next();

    EXPECT_EQ(e69.vertex(), im(4, 2));
    EXPECT_EQ(e70.vertex(), im(4, 3));
    EXPECT_EQ(e71.vertex(), im(3, 3));

    EXPECT_EQ(e69, e71.next());
    EXPECT_EQ(e69.prev(), e71);

    EXPECT_EQ(e69.face(), 23);
    EXPECT_EQ(e70.face(), 23);
    EXPECT_EQ(e71.face(), 23);

    EXPECT_FALSE(e69.twin());
    EXPECT_FALSE(e70.twin());
    EXPECT_TRUE(e71.twin());

    EXPECT_EQ(*e67.twin(), e71);
    EXPECT_EQ(*e71.twin(), e67);

    // Check orientation
    for (auto edge : edges.faces()) {
        const auto v0 = dvec3{im(edge.vertex()), 0.0};
        const auto v1 = dvec3{im((++edge).vertex()), 0.0};
        const auto v2 = dvec3{im((++edge).vertex()), 0.0};

        EXPECT_GT(glm::dot(glm::cross(v1 - v0, v2 - v1), dvec3{0.0, 0.0, 1.0}), 0.0);
        EXPECT_GT(glm::dot(glm::cross(v2 - v1, v0 - v2), dvec3{0.0, 0.0, 1.0}), 0.0);
        EXPECT_GT(glm::dot(glm::cross(v0 - v2, v1 - v0), dvec3{0.0, 0.0, 1.0}), 0.0);
    }
}

TEST(HalfEdges, indexbuffer) {
    constexpr int width = 4;
    constexpr int height = 3;
    const IndexBuffer plane = createPlane(width, height);
    util::IndexMapper<2, std::uint32_t> im{glm::uvec2{width + 1, height + 1}};

    HalfEdges edges(Mesh::MeshInfo{DrawType::Triangles, ConnectivityType::None}, plane);
    const auto indices = edges.createIndexBuffer();

    std::unordered_set<glm::u32vec3> org;
    std::unordered_set<glm::u32vec3> copy;

    meshutil::forEachTriangle(Mesh::MeshInfo{DrawType::Triangles, ConnectivityType::None}, plane,
                              [&](std::uint32_t a, std::uint32_t b, std::uint32_t c) {
                                  EXPECT_TRUE(org.emplace(a, b, c).second);
                              });
    EXPECT_EQ(org.size(), width * height * 2);

    meshutil::forEachTriangle(
        Mesh::MeshInfo{DrawType::Triangles, ConnectivityType::None}, indices,
        [&](std::uint32_t a, std::uint32_t b, std::uint32_t c) {
            EXPECT_TRUE(copy.emplace(a, b, c).second);

            const auto v0 = dvec3{im(a), 0.0};
            const auto v1 = dvec3{im(b), 0.0};
            const auto v2 = dvec3{im(c), 0.0};

            EXPECT_GT(glm::dot(glm::cross(v1 - v0, v2 - v1), dvec3{0.0, 0.0, 1.0}), 0.0);
            EXPECT_GT(glm::dot(glm::cross(v2 - v1, v0 - v2), dvec3{0.0, 0.0, 1.0}), 0.0);
            EXPECT_GT(glm::dot(glm::cross(v0 - v2, v1 - v0), dvec3{0.0, 0.0, 1.0}), 0.0);
        });

    EXPECT_EQ(org, copy);
}

TEST(HalfEdges, indexbufferAdjacency) {
    constexpr int width = 4;
    constexpr int height = 3;
    const IndexBuffer plane = createPlane(width, height);

    util::IndexMapper<2, std::uint32_t> im{glm::uvec2{width + 1, height + 1}};

    HalfEdges edges(Mesh::MeshInfo{DrawType::Triangles, ConnectivityType::None}, plane);
    const auto indices = edges.createIndexBufferWithAdjacency();

    std::unordered_set<glm::u32vec3> org;
    std::unordered_set<glm::u32vec3> copy;

    meshutil::forEachTriangle(Mesh::MeshInfo{DrawType::Triangles, ConnectivityType::None}, plane,
                              [&](std::uint32_t a, std::uint32_t b, std::uint32_t c) {
                                  EXPECT_TRUE(org.emplace(a, b, c).second);
                              });
    EXPECT_EQ(org.size(), width * height * 2);

    meshutil::forEachTriangle(
        Mesh::MeshInfo{DrawType::Triangles, ConnectivityType::Adjacency}, indices,
        [&](std::uint32_t a, std::uint32_t b, std::uint32_t c) {
            EXPECT_TRUE(copy.emplace(a, b, c).second);

            const auto v0 = dvec3{im(a), 0.0};
            const auto v1 = dvec3{im(b), 0.0};
            const auto v2 = dvec3{im(c), 0.0};

            EXPECT_GT(glm::dot(glm::cross(v1 - v0, v2 - v1), dvec3{0.0, 0.0, 1.0}), 0.0);
            EXPECT_GT(glm::dot(glm::cross(v2 - v1, v0 - v2), dvec3{0.0, 0.0, 1.0}), 0.0);
            EXPECT_GT(glm::dot(glm::cross(v0 - v2, v1 - v0), dvec3{0.0, 0.0, 1.0}), 0.0);
        });

    EXPECT_EQ(org, copy);

    const auto& data = indices.getRAMRepresentation()->getDataContainer();

    for (size_t i = 0; i < data.size(); i += 6) {
        const auto v1 = im(data[i + 0]);
        const auto v2 = im(data[i + 2]);
        const auto v3 = im(data[i + 4]);
        const auto a12 = im(data[i + 1]);
        const auto a23 = im(data[i + 3]);
        const auto a31 = im(data[i + 5]);

        {
            const auto fb12 = dvec2{v1} + dvec2{v2} - dvec2{v3};
            const auto ub12 = glm::ivec2(glm::round(fb12));

            const auto b12 = glm::any(glm::lessThan(ub12, glm::ivec2{0})) ||
                                     glm::any(glm::greaterThan(ub12, glm::ivec2{width, height}))
                                 ? glm::ivec2{v3}
                                 : ub12;

            EXPECT_EQ(b12, glm::ivec2{a12}) << "V1 " << v1 << " V2 " << v2 << " V3 " << v3;
        }

        {
            const auto fb23 = dvec2{v2} + dvec2{v3} - dvec2{v1};
            const auto ub23 = glm::ivec2(glm::round(fb23));

            const auto b23 = glm::any(glm::lessThan(ub23, glm::ivec2{0})) ||
                                     glm::any(glm::greaterThan(ub23, glm::ivec2{width, height}))
                                 ? glm::ivec2{v1}
                                 : ub23;

            EXPECT_EQ(b23, glm::ivec2{a23}) << "V1 " << v1 << " V2 " << v2 << " V3 " << v3;
        }
        {

            const auto fb31 = dvec2{v3} + dvec2{v1} - dvec2{v2};
            const auto ub31 = glm::ivec2(glm::round(fb31));

            const auto b31 = glm::any(glm::lessThan(ub31, glm::ivec2{0})) ||
                                     glm::any(glm::greaterThan(ub31, glm::ivec2{width, height}))
                                 ? glm::ivec2{v2}
                                 : ub31;

            EXPECT_EQ(b31, glm::ivec2{a31}) << "V1 " << v1 << " V2 " << v2 << " V3 " << v3;
        }
    }
}

}  // namespace inviwo
