/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <cmath>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/isovaluecollection.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/glmconvert.h>

#include <modules/base/algorithm/image/marchingsquares.h>
#include <modules/base/algorithm/image/layergeneration.h>
#include <modules/base/algorithm/dataminmax.h>

#include <glm/gtx/normal.hpp>

#include <vector>
#include <algorithm>
#include <ranges>

namespace inviwo {

namespace {

template <typename T>
std::vector<T>& getBufferData(Mesh& mesh, size_t ind) {
    if (auto* buffer = mesh.getBuffer(ind)) {
        if (buffer->getDataFormat() == DataFormat<T>::get()) {
            auto* tbuffer = static_cast<Buffer<vec2>*>(buffer);
            if (auto* ram = tbuffer->getEditableRAMRepresentation()) {
                return ram->getDataContainer();
            }
        }
    }
    throw Exception("Unable to get data container from mesh");
}

std::vector<uint32_t>& getBufferIndexData(Mesh& mesh, size_t ind) {
    if (auto* buffer = mesh.getIndices(ind)) {
        if (auto* ram = buffer->getEditableRAMRepresentation()) {
            return ram->getDataContainer();
        }
    }
    throw Exception("Unable to get index data container from mesh");
}

constexpr auto order = [](auto& a, auto& b) {
    return std::lexicographical_compare(glm::value_ptr(a), glm::value_ptr(a) + a.length(),
                                        glm::value_ptr(b), glm::value_ptr(b) + b.length());
};

}  // namespace

TEST(Marchingsquares, empty) {
    const IsoValueCollection isovalues{{{.pos = 0.5, .color = vec4{1.0f}}},
                                       TFPrimitiveSetType::Relative};

    auto layer = std::shared_ptr<Layer>(
        util::generateLayer(size2_t{2}, mat3(1.0f), [](const size2_t&) { return 0.0f; }));
    auto mesh = util::marchingSquares(layer->getRepresentation<LayerRAM>(), isovalues, 0);
    EXPECT_FALSE(mesh->hasBuffer(BufferType::PositionAttrib));
    EXPECT_EQ(mesh->getNumberOfIndices(), 0);
}

TEST(Marchingsquares, full) {
    const IsoValueCollection isovalues{{{.pos = 0.5, .color = vec4{1.0f}}},
                                       TFPrimitiveSetType::Relative};

    auto layer = std::shared_ptr<Layer>(
        util::generateLayer(size2_t{2}, mat3(1.0f), [](const size2_t&) { return 1.0f; }));
    auto mesh = util::marchingSquares(layer->getRepresentation<LayerRAM>(), isovalues, 0);
    EXPECT_FALSE(mesh->hasBuffer(BufferType::PositionAttrib));
    EXPECT_EQ(mesh->getNumberOfIndices(), 0);
}

TEST(Marchingsquares, case1) {
    const IsoValueCollection isovalues{{{.pos = 0.5, .color = vec4{1.0f}}},
                                       TFPrimitiveSetType::Relative};

    auto layer = std::shared_ptr<Layer>(util::makeSingleTexelLayer<double>(size2_t{2, 2}));
    auto mesh = util::marchingSquares(layer->getRepresentation<LayerRAM>(), isovalues, 0);
    ASSERT_TRUE(mesh->hasBuffer(BufferType::PositionAttrib));
    ASSERT_EQ(mesh->getNumberOfIndices(), 1);

    const auto& pos = getBufferData<vec2>(*mesh, 0);
    const auto& ind = getBufferIndexData(*mesh, 0);

    size_t expectedIndices = 2;
    if (mesh->getIndexMeshInfo(0).ct == ConnectivityType::StripAdjacency) {
        expectedIndices += 2;
    }

    EXPECT_EQ(pos.size(), 2);
    EXPECT_EQ(ind.size(), expectedIndices);

    std::vector<vec2> spos(pos);
    std::ranges::sort(spos, order);

    std::vector expected = {vec2{0.5f, 0.25f}, vec2{0.25f, 0.5f}};
    std::ranges::sort(expected, order);

    EXPECT_EQ(spos, expected);
}

TEST(Marchingsquares, openloop) {
    const IsoValueCollection isovalues{{{.pos = 0.5, .color = vec4{1.0f}}},
                                       TFPrimitiveSetType::Relative};

    auto layer = std::shared_ptr<Layer>(util::makeSingleTexelLayer<double>(size2_t{2, 3}));
    auto mesh = util::marchingSquares(layer->getRepresentation<LayerRAM>(), isovalues, 0);
    ASSERT_TRUE(mesh->hasBuffer(BufferType::PositionAttrib));
    ASSERT_EQ(mesh->getNumberOfIndices(), 1);

    const auto& pos = getBufferData<vec2>(*mesh, 0);
    const auto& ind = getBufferIndexData(*mesh, 0);

    size_t expectedIndices = 3;
    if (mesh->getIndexMeshInfo(0).ct == ConnectivityType::StripAdjacency) {
        expectedIndices += 2;
    }

    EXPECT_EQ(pos.size(), 3);
    EXPECT_EQ(ind.size(), expectedIndices);

    std::vector<vec2> spos(pos);
    std::ranges::sort(spos, order);

    std::vector expected = {vec2{0.25f, 1.0f / 3.0f}, vec2{0.5f, 0.5f}, vec2{0.25f, 2.0f / 3.0f}};
    std::ranges::sort(expected, order);

    EXPECT_EQ(spos, expected);
}

TEST(Marchingsquares, circle) {
    const IsoValueCollection isovalues{{{.pos = 0.5, .color = vec4{1.0f}}},
                                       TFPrimitiveSetType::Relative};

    auto layer = std::shared_ptr<Layer>(util::makeSingleTexelLayer<double>(size2_t{3, 3}));
    auto mesh = util::marchingSquares(layer->getRepresentation<LayerRAM>(), isovalues, 0);
    ASSERT_TRUE(mesh->hasBuffer(BufferType::PositionAttrib));
    ASSERT_EQ(mesh->getNumberOfIndices(), 1);

    const auto& pos = getBufferData<vec2>(*mesh, 0);
    const auto& ind = getBufferIndexData(*mesh, 0);

    size_t expectedIndices = 5;
    if (mesh->getIndexMeshInfo(0).ct == ConnectivityType::StripAdjacency) {
        expectedIndices += 2;
    }

    EXPECT_EQ(pos.size(), 4);
    EXPECT_EQ(ind.size(), expectedIndices);

    std::vector<vec2> spos(pos);
    std::ranges::sort(spos, order);

    std::vector expected = {vec2{1.0f / 3.0f, 0.5f}, vec2{0.5f, 1.0f / 3.0f},
                            vec2{2.0f / 3.0f, 0.5f}, vec2{0.5f, 2.0f / 3.0f}};
    std::ranges::sort(expected, order);

    EXPECT_EQ(spos, expected);
}

}  // namespace inviwo
