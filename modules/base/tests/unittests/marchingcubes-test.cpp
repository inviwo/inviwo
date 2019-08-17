/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>
#include <modules/base/algorithm/volume/volumegeneration.h>

#include <modules/base/algorithm/volume/marchingcubes.h>
#include <modules/base/algorithm/volume/marchingcubesopt.h>

#include <glm/gtx/normal.hpp>

namespace inviwo {

template <typename T>
std::vector<T>& getBufferData(Mesh& mesh, size_t ind) {
    if (auto buffer = mesh.getBuffer(ind)) {
        if (buffer->getDataFormat() == DataFormat<T>::get()) {
            auto tbuffer = static_cast<Buffer<vec3>*>(buffer);
            if (auto ram = tbuffer->getEditableRAMRepresentation()) {
                return ram->getDataContainer();
            }
        }
    }
    throw Exception("Unable to get data container from mesh", IVW_CONTEXT_CUSTOM("getBufferData"));
}

std::vector<uint32_t>& getBufferIndexData(Mesh& mesh, size_t ind) {
    if (auto buffer = mesh.getIndices(ind)) {
        if (auto ram = buffer->getEditableRAMRepresentation()) {
            return ram->getDataContainer();
        }
    }
    throw Exception("Unable to get index data container from mesh",
                    IVW_CONTEXT_CUSTOM("getBufferIndexData"));
}

TEST(Marchingcubes, empty) {
    auto vol = std::shared_ptr<Volume>(
        util::generateVolume(size3_t{2}, mat3(1.0f), [&](const size3_t&) { return 0.0f; }));
    auto mesh = util::marchingCubesOpt(vol, 0.5, {1.0f, 0.0f, 0.0f, 1.0f}, false, false);
    auto& pos = getBufferData<vec3>(*mesh, 0);
    auto& ind = getBufferIndexData(*mesh, 0);
    EXPECT_EQ(pos.size(), 0);
    EXPECT_EQ(ind.size(), 0);
}

TEST(Marchingcubes, full) {
    auto vol = std::shared_ptr<Volume>(
        util::generateVolume(size3_t{2}, mat3(1.0f), [&](const size3_t&) { return 1.0f; }));
    auto mesh = util::marchingCubesOpt(vol, 0.5, {1.0f, 0.0f, 0.0f, 1.0f}, false, false);
    auto& pos = getBufferData<vec3>(*mesh, 0);
    auto& ind = getBufferIndexData(*mesh, 0);
    EXPECT_EQ(pos.size(), 0);
    EXPECT_EQ(ind.size(), 0);
}

TEST(Marchingcubes, one) {
    const std::array<size3_t, 8> voxels = {
        {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}}};

    auto order = [](auto& a, auto& b) {
        return std::lexicographical_compare(glm::value_ptr(a), glm::value_ptr(a) + 3,
                                            glm::value_ptr(b), glm::value_ptr(b) + 3);
    };
    auto map = [](auto&& vec) {
        std::transform(glm::value_ptr(vec), glm::value_ptr(vec) + 3, glm::value_ptr(vec),
                       [](auto x) { return x > 1.0f ? x - 1.0f : x; });
        return vec;
    };

    const vec3 center{0.5f};

    for (auto& v : voxels) {
        auto vol = std::shared_ptr<Volume>(
            util::generateVolume(size3_t{2}, mat3(1.0f), [&](const size3_t& ind) {
                if (ind == v) {
                    return 1.0f;
                } else {
                    return 0.0f;
                }
            }));
        auto mesh = util::marchingCubesOpt(vol, 0.5, {1.0f, 0.0f, 0.0f, 1.0f}, false, false);
        auto& pos = getBufferData<vec3>(*mesh, 0);
        auto& ind = getBufferIndexData(*mesh, 0);
        ASSERT_EQ(pos.size(), 3);
        ASSERT_EQ(ind.size(), 3);

        const vec3 vert(v);

        std::vector<vec3> vals = {map(vert + vec3{0.5f, 0.0f, 0.0f}),
                                  map(vert + vec3{0.0f, 0.5f, 0.0f}),
                                  map(vert + vec3{0.0f, 0.0f, 0.5f})};
        std::sort(vals.begin(), vals.end(), order);

        std::vector<vec3> spos(pos);
        std::sort(spos.begin(), spos.end(), order);

        EXPECT_EQ(vals, spos);

        auto triNormal = glm::triangleNormal(pos[ind[0]], pos[ind[1]], pos[ind[2]]);
        EXPECT_TRUE(glm::dot(triNormal, (center - vert)) > 0.0);
    }

    for (auto& v : voxels) {
        auto vol = std::shared_ptr<Volume>(
            util::generateVolume(size3_t{2}, mat3(1.0f), [&](const size3_t& ind) {
                if (ind != v) {
                    return 1.0f;
                } else {
                    return 0.0f;
                }
            }));
        auto mesh = util::marchingCubesOpt(vol, 0.5, {1.0f, 0.0f, 0.0f, 1.0f}, false, false);
        auto& pos = getBufferData<vec3>(*mesh, 0);
        auto& ind = getBufferIndexData(*mesh, 0);
        ASSERT_EQ(pos.size(), 3);
        ASSERT_EQ(ind.size(), 3);

        const vec3 vert(v);

        std::vector<vec3> vals = {map(vert + vec3{0.5f, 0.0f, 0.0f}),
                                  map(vert + vec3{0.0f, 0.5f, 0.0f}),
                                  map(vert + vec3{0.0f, 0.0f, 0.5f})};
        std::sort(vals.begin(), vals.end(), order);

        std::vector<vec3> spos(pos);
        std::sort(spos.begin(), spos.end(), order);

        EXPECT_EQ(vals, spos);

        auto triNormal = glm::triangleNormal(pos[ind[0]], pos[ind[1]], pos[ind[2]]);
        EXPECT_TRUE(glm::dot(triNormal, (center - vert)) < 0.0);
    }
}

TEST(Marchingcubes, two) {
    const std::array<size3_t, 8> voxels = {
        {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}}};

    auto order = [](auto& a, auto& b) {
        return std::lexicographical_compare(glm::value_ptr(a), glm::value_ptr(a) + 3,
                                            glm::value_ptr(b), glm::value_ptr(b) + 3);
    };
    auto map = [](auto&& vec) {
        std::transform(glm::value_ptr(vec), glm::value_ptr(vec) + 3, glm::value_ptr(vec),
                       [](auto x) { return x > 1.0f ? x - 1.0f : x; });
        return vec;
    };

    std::vector<vec3> vals = {vec3{0.5f, 0.0f, 0.0f}, vec3{0.0f, 0.5f, 0.0f},
                              vec3{0.0f, 0.0f, 0.5f}};
    int n1nn = 0;
    int n2nn = 0;
    int n3nn = 0;
    for (auto& v1 : voxels) {
        for (auto& v2 : voxels) {
            if (v1 == v2) continue;

            auto vol = std::shared_ptr<Volume>(
                util::generateVolume(size3_t{2}, mat3(1.0f), [&](const size3_t& ind) {
                    if (ind == v1 || ind == v2) {
                        return 1.0f;
                    } else {
                        return 0.0f;
                    }
                }));
            auto mesh = util::marchingCubesOpt(vol, 0.5, {1.0f, 0.0f, 0.0f, 1.0f}, false, false);

            auto& pos = getBufferData<vec3>(*mesh, 0);
            auto& ind = getBufferIndexData(*mesh, 0);

            vec3 fv1(v1);
            vec3 fv2(v2);

            auto l2 = glm::compAdd((v2 - v1) * (v2 - v1));

            if (l2 == 1) {
                ++n1nn;

                ASSERT_EQ(4, pos.size());
                ASSERT_EQ(6, ind.size());

                std::vector<vec3> vertices;
                for (int i = 0; i < 3; ++i) {
                    if (v1[i] == v2[i]) {
                        vertices.push_back(map(fv1 + vals[i]));
                        vertices.push_back(map(fv2 + vals[i]));
                    }
                }
                std::sort(vertices.begin(), vertices.end(), order);
                std::vector<vec3> spos(pos);
                std::sort(spos.begin(), spos.end(), order);

                EXPECT_EQ(vertices, spos);

            } else if (l2 == 2) {
                ++n2nn;
                ASSERT_EQ(6, pos.size());
                ASSERT_EQ(6, ind.size());
            } else if (l2 == 3) {
                ++n3nn;
                ASSERT_EQ(6, pos.size());
                ASSERT_EQ(6, ind.size());
            } else {
                FAIL();
            }
        }
    }
    EXPECT_EQ(24, n1nn);
    EXPECT_EQ(24, n2nn);
    EXPECT_EQ(8, n3nn);
}

TEST(Marchingcubes, minimal) {
    auto v = std::shared_ptr<Volume>(util::makeSingleVoxelVolume(size3_t{3}));

    auto mesh1 = util::marchingcubes(v, 0.5, {0.5f, 0.0f, 0.0f, 1.0f}, false, false);
    auto mesh2 = util::marchingCubesOpt(v, 0.5, {0.5f, 0.0f, 0.0f, 1.0f}, false, false);

    ASSERT_EQ(mesh1->getNumberOfBuffers(), 4);
    ASSERT_EQ(mesh2->getNumberOfBuffers(), 4);

    EXPECT_EQ(mesh1->getBuffers()[0].first.type, BufferType::PositionAttrib);
    EXPECT_EQ(mesh2->getBuffers()[0].first.type, BufferType::PositionAttrib);

    auto buffer1 = mesh1->getBuffers()[0].second;
    auto buffer2 = mesh2->getBuffers()[0].second;

    ASSERT_EQ(buffer1->getDataFormat(), DataVec3Float32::get());
    ASSERT_EQ(buffer2->getDataFormat(), DataVec3Float32::get());

    auto posBuffer1 = static_cast<Buffer<vec3>*>(buffer1.get());
    auto posBuffer2 = static_cast<Buffer<vec3>*>(buffer2.get());

    auto ram1 = posBuffer1->getRAMRepresentation();
    auto ram2 = posBuffer2->getRAMRepresentation();

    auto& data1 = ram1->getDataContainer();
    auto& data2 = ram2->getDataContainer();

    EXPECT_EQ(data1.size(), 6);
    EXPECT_EQ(data2.size(), 6);

    EXPECT_EQ(data1, data2);

    ASSERT_EQ(mesh1->getNumberOfIndicies(), 1);
    ASSERT_EQ(mesh2->getNumberOfIndicies(), 1);

    auto indBuffer1 = mesh1->getIndices(0);
    auto indBuffer2 = mesh2->getIndices(0);

    auto& ind1 = indBuffer1->getRAMRepresentation()->getDataContainer();
    auto& ind2 = indBuffer2->getRAMRepresentation()->getDataContainer();

    EXPECT_EQ(ind1.size(), 8 * 3);
    EXPECT_EQ(ind2.size(), 8 * 3);

    // EXPECT_EQ(ind1, ind2); the order are not the same...
}

TEST(Marchingcubes, sphere) {
    auto v = std::shared_ptr<Volume>(util::makeSphericalVolume(size3_t{5}));

    auto mesh1 = util::marchingcubes(v, 0.5, {0.5f, 0.0f, 0.0f, 1.0f}, false, false);
    auto mesh2 = util::marchingCubesOpt(v, 0.5, {0.5f, 0.0f, 0.0f, 1.0f}, false, false);

    ASSERT_EQ(mesh1->getNumberOfBuffers(), 4);
    ASSERT_EQ(mesh2->getNumberOfBuffers(), 4);

    EXPECT_EQ(mesh1->getBuffers()[0].first.type, BufferType::PositionAttrib);
    EXPECT_EQ(mesh2->getBuffers()[0].first.type, BufferType::PositionAttrib);

    auto buffer1 = mesh1->getBuffers()[0].second;
    auto buffer2 = mesh2->getBuffers()[0].second;

    ASSERT_EQ(buffer1->getDataFormat(), DataVec3Float32::get());
    ASSERT_EQ(buffer2->getDataFormat(), DataVec3Float32::get());

    /*
    auto posBuffer1 = static_cast<Buffer<vec3>*>(buffer1.get());
    auto posBuffer2 = static_cast<Buffer<vec3>*>(buffer2.get());

    auto ram1 = posBuffer1->getRAMRepresentation();
    auto ram2 = posBuffer2->getRAMRepresentation();

    auto& data1 = ram1->getDataContainer();
    auto& data2 = ram2->getDataContainer();

    EXPECT_EQ(data1, data2);  the order are not the same...
    */

    ASSERT_EQ(mesh1->getNumberOfIndicies(), 1);
    ASSERT_EQ(mesh2->getNumberOfIndicies(), 1);

    /*
    auto indBuffer1 = mesh1->getIndices(0);
    auto indBuffer2 = mesh2->getIndices(0);

    auto& ind1 = indBuffer1->getRAMRepresentation()->getDataContainer();
    auto& ind2 = indBuffer2->getRAMRepresentation()->getDataContainer();

    EXPECT_EQ(ind1, ind2); // the order are not the same...
    */
}

}  // namespace inviwo
