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

#include <inviwo/core/datastructures/geometry/typedmesh.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/glm.h>

#include <array>
#include <vector>

namespace inviwo {

template <typename... BufferTraits>
bool operator==(const TypedMesh<BufferTraits...> &a, const TypedMesh<BufferTraits...> &b) {
    if (a.getDefaultMeshInfo() != b.getDefaultMeshInfo()) {
        return false;
    }
    if ((a.getNumberOfBuffers() != b.getNumberOfBuffers()) ||
        (a.getNumberOfIndicies() != b.getNumberOfIndicies())) {
        return false;
    }
    auto compareBuffers = [](const auto &buffersA, const auto &buffersB) {
        for (auto &&elem : util::zip(buffersA, buffersB)) {
            auto &bufA = get<0>(elem);
            auto &bufB = get<1>(elem);
            if (bufA.first != bufB.first) {
                return false;
            }
            // check buffer contents
            if (*bufA.second.get() != *bufB.second.get()) {
                return false;
            }
        }
        return true;
    };

    // compare buffers
    if (!compareBuffers(a.getBuffers(), b.getBuffers())) {
        return false;
    }
    // compare index buffers
    if (!compareBuffers(a.getIndexBuffers(), b.getIndexBuffers())) {
        return false;
    }
    return true;
}

template <typename... BufferTraits>
bool operator!=(const TypedMesh<BufferTraits...> &a, const TypedMesh<BufferTraits...> &b) {
    return !(a == b);
}

TEST(buffertraits, positionsbuffer) {
    EXPECT_EQ(BufferType::PositionAttrib, buffertraits::PositionsBuffer::bi().type);
    ASSERT_EQ(3, util::extent<buffertraits::PositionsBuffer::type>::value)
        << "buffertraits::PositionsBuffer expected to hold vec3";

    buffertraits::PositionsBuffer posbuf;
    posbuf.getEditableVertices()->setSize(1);
    posbuf.setVertexPosition(0, vec3(1.0f));

    const auto &data = posbuf.getTypedDataContainer();
    ASSERT_EQ(1, data.size()) << "position buffer size != 1";
    EXPECT_EQ(vec3(1.0f), data[0]);
}

TEST(buffertraits, colorsbuffer) {
    EXPECT_EQ(BufferType::ColorAttrib, buffertraits::ColorsBuffer::bi().type);
    ASSERT_EQ(4, util::extent<buffertraits::ColorsBuffer::type>::value)
        << "buffertraits::ColorsBuffer expected to hold vec4";

    buffertraits::ColorsBuffer colorbuf;
    colorbuf.getEditableColors()->setSize(1);
    colorbuf.setVertexColor(0, vec4(0.5f, 0.25f, 0.0f, 1.0f));

    const auto &data = colorbuf.getTypedDataContainer();
    ASSERT_EQ(1, data.size()) << "color buffer size != 1";
    EXPECT_EQ(vec4(0.5f, 0.25f, 0.0f, 1.0f), data[0]);
}

TEST(buffertraits, normalbuffer) {
    EXPECT_EQ(BufferType::NormalAttrib, buffertraits::NormalBuffer::bi().type);
    ASSERT_EQ(3, util::extent<buffertraits::NormalBuffer::type>::value)
        << "buffertraits::NormalBuffer expected to hold vec3";

    buffertraits::NormalBuffer normalbuf;
    normalbuf.getEditableNormals()->setSize(1);
    normalbuf.setVertexNormal(0, vec3(0.0f, 1.0f, 0.0f));

    const auto &data = normalbuf.getTypedDataContainer();
    ASSERT_EQ(1, data.size()) << "normal buffer size != 1";
    EXPECT_EQ(vec3(0.0f, 1.0f, 0.0f), data[0]);
}

TEST(buffertraits, texcoordbuffer2) {
    EXPECT_EQ(BufferType::TexcoordAttrib, buffertraits::TexcoordBuffer<2>::bi().type);
    ASSERT_EQ(2, util::extent<buffertraits::TexcoordBuffer<2>::type>::value)
        << "buffertraits::TexcoordBuffer<2> expected to hold vec2";

    buffertraits::TexcoordBuffer<2> texcoordbuf;
    texcoordbuf.getEditableTexCoords()->setSize(1);
    texcoordbuf.setVertexTexCoord(0, vec2(0.25f, 1.0f));

    const auto &data = texcoordbuf.getTypedDataContainer();
    ASSERT_EQ(1, data.size()) << "texcoord buffer size != 1";
    EXPECT_EQ(vec2(0.25f, 1.0f), data[0]);
}

TEST(buffertraits, radiibuffer) {
    EXPECT_EQ(BufferType::RadiiAttrib, buffertraits::RadiiBuffer::bi().type);
    ASSERT_EQ(1, util::extent<buffertraits::RadiiBuffer::type>::value)
        << "buffertraits::RadiiBuffer expected to hold float";

    buffertraits::RadiiBuffer radiibuf;
    radiibuf.getEditableRadii()->setSize(1);
    radiibuf.setVertexRadius(0, 3.14f);

    const auto &data = radiibuf.getTypedDataContainer();
    ASSERT_EQ(1, data.size()) << "radius buffer size != 1";
    EXPECT_FLOAT_EQ(3.14f, data[0]);
}

TEST(typedmesh, compilationChecks) {
    using MyMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::NormalBuffer,
                             buffertraits::ColorsBuffer>;

    MyMesh mesh;
    // vertex operations
    mesh.addVertex(vec3(0.0f), vec3(0, 1, 0), vec4(1.0f));
    mesh.addVertex(MyMesh::Vertex{vec3(1.0f), vec3(1, 0, 0), vec4(0.5f)});

    mesh.addVertices({{vec3(2.0f), vec3(1.0f), vec4(1.0f)}, {vec3(3.0f), vec3(1.0f), vec4(1.0f)}});

    mesh.setVertex(1, vec3(1.5f), vec3(1.0f, 0.0f, 0.0f), vec4(1.0f));
    mesh.setVertex(2, MyMesh::Vertex{vec3(2.5f), vec3(0.0f), vec4(1.0f)});
    mesh.setVertex(3, {vec3(3.5f), vec3(0.0f), vec4(1.0f)});

    mesh.setVertex<buffertraits::ColorsBuffer>(3, vec4(0.2f));
    mesh.setVertexPosition(0, vec3(0.5f));
    mesh.setVertexColor(0, vec4(0.0f));
    mesh.setVertexNormal(0, vec3(1.0f, 0.0f, 0.0f));

    // typed mesh clone
    auto m = std::shared_ptr<MyMesh>(mesh.clone());

    // buffers
    mesh.getTypedBuffer<buffertraits::NormalBuffer>();
    mesh.getTypedRAMRepresentation<buffertraits::NormalBuffer>();
    mesh.getTypedEditableRAMRepresentation<buffertraits::NormalBuffer>();
    mesh.getTypedDataContainer<buffertraits::NormalBuffer>();

    MyMesh copy(mesh);
    auto copy2 = mesh;

    using FloatBuffer =
        buffertraits::TypedMeshBufferBase<float, 1, static_cast<int>(BufferType::PositionAttrib)>;

    using TestMesh = TypedMesh<FloatBuffer, buffertraits::ColorsBuffer>;
    TestMesh t;
    t.addVertex(1.0f, vec4(1.0f));
    t.setVertex(0, 1.5f, vec4(1.0f));

    t.addVertex(TestMesh::Vertex{2.0, vec4(0.5f)});
    t.setVertex(1, {2.5f, vec4(0.5f)});
}

TEST(typedmesh, copyconstructor) {
    using MyMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::ColorsBuffer>;

    MyMesh mesh;
    mesh.addVertex(vec3(1.0f), vec4(1, 0, 0, 1));

    MyMesh copy(mesh);

    EXPECT_TRUE(mesh == copy) << "mesh copy not identical";

    // modify copy, check for equality
    copy.setVertexPosition(0, vec3(0.0f));
    EXPECT_FALSE(mesh == copy) << "mesh created with copy constructor is not a deep copy";
}

TEST(typedmesh, assignment) {
    using MyMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::ColorsBuffer>;

    MyMesh mesh;
    mesh.addVertex(vec3(1.0f), vec4(1, 0, 0, 1));

    MyMesh copy = mesh;

    EXPECT_TRUE(mesh == copy) << "mesh copy not identical";

    // modify copy, check for equality
    copy.setVertexPosition(0, vec3(0.0f));
    EXPECT_FALSE(mesh == copy) << "mesh resulting from assignment is not a deep copy";
}

TEST(typedmesh, clone) {
    using MyMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::ColorsBuffer>;

    MyMesh mesh;
    mesh.addVertex(vec3(1.0f), vec4(1, 0, 0, 1));

    auto copy = std::shared_ptr<MyMesh>(mesh.clone());

    EXPECT_TRUE(mesh == *copy.get()) << "cloned mesh not identical";

    // modify copy, check for equality
    copy->setVertexPosition(0, vec3(0.0f));
    EXPECT_FALSE(mesh == *copy.get()) << "cloned mesh is not a deep copy";
}

TEST(vertexop, addVertexTraits) {
    using MyMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::ColorsBuffer>;

    std::array<vec3, 2> vertices = {{vec3(0.0f), vec3(1.0f)}};
    std::array<vec4, 2> colors = {{vec4(1, 0, 0, 1), vec4(0, 1, 0, 1)}};

    MyMesh mesh;
    uint32_t vertexCount = 0;
    for (auto &&elem : util::zip(vertices, colors)) {
        uint32_t index = mesh.addVertex(get<0>(elem), get<1>(elem));
        EXPECT_EQ(vertexCount++, index);
    }

    const auto &posbuf = mesh.getTypedDataContainer<buffertraits::PositionsBuffer>();
    const auto &colorbuf = mesh.getTypedDataContainer<buffertraits::ColorsBuffer>();

    ASSERT_EQ(vertices.size(), posbuf.size()) << "number of vertices do not match";
    ASSERT_EQ(colors.size(), colorbuf.size()) << "number of colors do not match";

    for (size_t i = 0; i < vertices.size(); ++i) {
        EXPECT_EQ(vertices[i], posbuf[i]) << "position mismatch (" << i << ")";
        EXPECT_EQ(colors[i], colorbuf[i]) << "color mismatch (" << i << ")";
    }
}

TEST(vertexop, addVertex) {
    using MyMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::ColorsBuffer>;

    std::array<vec3, 2> vertices = {{vec3(0.0f), vec3(1.0f)}};
    std::array<vec4, 2> colors = {{vec4(1, 0, 0, 1), vec4(0, 1, 0, 1)}};

    MyMesh mesh;
    uint32_t vertexCount = 0;
    for (auto &&elem : util::zip(vertices, colors)) {
        MyMesh::Vertex v = {get<0>(elem), get<1>(elem)};
        uint32_t index = mesh.addVertex(v);
        EXPECT_EQ(vertexCount++, index);
    }

    const auto &posbuf = mesh.getTypedDataContainer<buffertraits::PositionsBuffer>();
    const auto &colorbuf = mesh.getTypedDataContainer<buffertraits::ColorsBuffer>();

    ASSERT_EQ(vertices.size(), posbuf.size()) << "number of vertices do not match";
    ASSERT_EQ(colors.size(), colorbuf.size()) << "number of colors do not match";

    for (size_t i = 0; i < vertices.size(); ++i) {
        EXPECT_EQ(vertices[i], posbuf[i]) << "position mismatch (" << i << ")";
        EXPECT_EQ(colors[i], colorbuf[i]) << "color mismatch (" << i << ")";
    }
}

TEST(vertexop, addVertices) {
    using MyMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::ColorsBuffer>;

    std::array<vec3, 2> vertices = {{vec3(0.0f), vec3(1.0f)}};
    std::array<vec4, 2> colors = {{vec4(1, 0, 0, 1), vec4(0, 1, 0, 1)}};

    std::vector<MyMesh::Vertex> vertexData;
    for (auto &&elem : util::zip(vertices, colors)) {
        vertexData.emplace_back(get<0>(elem), get<1>(elem));
    }
    MyMesh mesh;
    mesh.addVertices(vertexData);

    const auto &posbuf = mesh.getTypedDataContainer<buffertraits::PositionsBuffer>();
    const auto &colorbuf = mesh.getTypedDataContainer<buffertraits::ColorsBuffer>();

    ASSERT_EQ(vertices.size(), posbuf.size()) << "number of vertices do not match";
    ASSERT_EQ(colors.size(), colorbuf.size()) << "number of colors do not match";

    for (size_t i = 0; i < vertices.size(); ++i) {
        EXPECT_EQ(vertices[i], posbuf[i]) << "position mismatch (" << i << ")";
        EXPECT_EQ(colors[i], colorbuf[i]) << "color mismatch (" << i << ")";
    }
}

TEST(vertexop, setVertexTraits) {
    using MyMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::ColorsBuffer>;

    std::array<vec3, 2> vertices = {{vec3(0.0f), vec3(1.0f)}};
    std::array<vec4, 2> colors = {{vec4(1, 0, 0, 1), vec4(0, 1, 0, 1)}};

    MyMesh mesh;
    mesh.addVertex(vertices[0], colors[0]);
    mesh.setVertex(0, vertices[1], colors[1]);

    const auto &posbuf = mesh.getTypedDataContainer<buffertraits::PositionsBuffer>();
    const auto &colorbuf = mesh.getTypedDataContainer<buffertraits::ColorsBuffer>();
    EXPECT_EQ(vertices[1], posbuf[0]) << "position mismatch";
    EXPECT_EQ(colors[1], colorbuf[0]) << "color mismatch";
}

TEST(vertexop, setVertex) {
    using MyMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::ColorsBuffer>;

    std::array<vec3, 2> vertices = {{vec3(0.0f), vec3(1.0f)}};
    std::array<vec4, 2> colors = {{vec4(1, 0, 0, 1), vec4(0, 1, 0, 1)}};

    MyMesh mesh;
    mesh.addVertex(vertices[0], colors[0]);
    MyMesh::Vertex v = {vertices[1], colors[1]};
    mesh.setVertex(0, v);

    const auto &posbuf = mesh.getTypedDataContainer<buffertraits::PositionsBuffer>();
    const auto &colorbuf = mesh.getTypedDataContainer<buffertraits::ColorsBuffer>();
    EXPECT_EQ(vertices[1], posbuf[0]) << "position mismatch";
    EXPECT_EQ(colors[1], colorbuf[0]) << "color mismatch";
}

TEST(vertexop, setVertexPos) {
    using MyMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::ColorsBuffer>;

    std::array<vec3, 2> vertices = {{vec3(0.0f), vec3(1.0f)}};
    std::array<vec4, 2> colors = {{vec4(1, 0, 0, 1), vec4(0, 1, 0, 1)}};

    MyMesh mesh;
    mesh.addVertex(vertices[0], colors[0]);
    mesh.setVertex<buffertraits::PositionsBuffer>(0, vertices[1]);

    const auto &posbuf = mesh.getTypedDataContainer<buffertraits::PositionsBuffer>();
    const auto &colorbuf = mesh.getTypedDataContainer<buffertraits::ColorsBuffer>();
    EXPECT_EQ(vertices[1], posbuf[0]) << "position mismatch";
    EXPECT_EQ(colors[0], colorbuf[0]) << "color mismatch";
}

}  // namespace inviwo
