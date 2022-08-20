/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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
#include <modules/base/algorithm/volume/volumevoronoi.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/indexmapper.h>

namespace inviwo {

constexpr auto clamp3D = Wrapping3D{Wrapping::Clamp, Wrapping::Clamp, Wrapping::Clamp};

TEST(VolumeVoronoi, Voronoi_NoSeedPoints_ThrowsException) {
    EXPECT_THROW(util::voronoiSegmentation(
                     /*volumeDimensions*/ size3_t{3, 3, 3},
                     /*indexToModelMatrix*/ mat4(), /*seedPoints*/ {},
                     /*wrapping*/ clamp3D,
                     /*weights*/ std::nullopt),
                 inviwo::Exception);
}

TEST(VolumeVoronoi, WeightedVoronoi_WeightsAndSeedPointsDimensionMissmatch_ThrowsException) {
    const std::vector<std::pair<uint32_t, vec3>> seedPoints = {{1, vec3{0.3, 0.2, 0.1}},
                                                               {2, vec3{0.1, 0.2, 0.3}}};
    const std::vector<float> weights = {3.0, 4.0, 5.0, 6.0, 7.0};

    EXPECT_THROW(util::voronoiSegmentation(/*volumeDimensions*/ size3_t{3, 3, 3},
                                           /*indexToModelMatrix*/ mat4(), seedPoints,
                                           /*wrapping*/ clamp3D, weights),
                 inviwo::Exception);
}

TEST(VolumeVoronoi, Voronoi_OneSeedPoint_WholeVolumeHasSameIndex) {
    const std::vector<std::pair<uint32_t, vec3>> seedPoints = {{1, vec3{1, 1, 1}}};
    const auto dimensions = size3_t{3, 3, 3};

    auto volumeVoronoi =
        util::voronoiSegmentation(dimensions,
                                  /*indexToModelMatrix*/
                                  mat4{vec4{1.0, 0.0, 0.0, 0.0}, vec4{0.0, 1.0, 0.0, 0.0},
                                       vec4{0.0, 0.0, 1.0, 0.0}, vec4{0.0, 0.0, 0.0, 1.0}},
                                  seedPoints, /*wrapping*/ clamp3D, /*weights*/ std::nullopt);

    const VolumeRAMPrecision<unsigned short>* ramtyped =
        dynamic_cast<const VolumeRAMPrecision<unsigned short>*>(
            volumeVoronoi->getRepresentation<VolumeRAM>());

    EXPECT_TRUE(ramtyped != nullptr);

    const auto data = ramtyped->getDataTyped();
    const util::IndexMapper3D im(dimensions);

    for (size_t z = 0; z < dimensions.z; z++) {
        for (size_t y = 0; y < dimensions.y; y++) {
            for (size_t x = 0; x < dimensions.x; x++) {
                const auto val = static_cast<unsigned short>(data[im(x, y, z)]);
                EXPECT_EQ(val, seedPoints[0].first);
            }
        }
    }
}

TEST(VolumeVoronoi, Voronoi_TwoSeedPoints_PartitionsVolumeInTwo) {
    const std::vector<std::pair<uint32_t, vec3>> seedPoints = {{1, vec3{1, 2, 2}},
                                                               {2, vec3{2, 2, 2}}};
    const auto dimensions = size3_t{4, 4, 4};

    auto volumeVoronoi =
        util::voronoiSegmentation(dimensions,
                                  /*indexToModelMatrix*/
                                  mat4{vec4{1.0, 0.0, 0.0, 0.0}, vec4{0.0, 1.0, 0.0, 0.0},
                                       vec4{0.0, 0.0, 1.0, 0.0}, vec4{0.0, 0.0, 0.0, 1.0}},
                                  seedPoints, /*wrapping*/ clamp3D, /*weights*/ std::nullopt);

    const VolumeRAMPrecision<unsigned short>* ramtyped =
        dynamic_cast<const VolumeRAMPrecision<unsigned short>*>(
            volumeVoronoi->getRepresentation<VolumeRAM>());

    EXPECT_TRUE(ramtyped != nullptr);

    const auto data = ramtyped->getDataTyped();
    const util::IndexMapper3D im(dimensions);

    for (size_t z = 0; z < dimensions.z; z++) {
        for (size_t y = 0; y < dimensions.y; y++) {
            // First half of volume
            for (size_t x1 = 0; x1 < dimensions.x / 2; x1++) {
                const auto val1 = static_cast<unsigned short>(data[im(x1, y, z)]);
                EXPECT_EQ(val1, seedPoints[0].first);
            }
            // Second half of volume
            for (size_t x2 = dimensions.x / 2; x2 < dimensions.x; x2++) {
                const auto val2 = static_cast<unsigned short>(data[im(x2, y, z)]);
                EXPECT_EQ(val2, seedPoints[1].first);
            }
        }
    }
}

TEST(VolumeVoronoi, WeightedVoronoi_TwoSeedPointsWithWeights_PartitionsVolumeInTwoWeightedParts) {
    const std::vector<std::pair<uint32_t, vec3>> seedPoints = {{1, vec3{1, 2, 2}},
                                                               {2, vec3{2, 2, 2}}};
    const auto dimensions = size3_t{4, 4, 4};

    auto volumeVoronoi = util::voronoiSegmentation(
        dimensions,
        /*indexToModelMatrix*/
        mat4{vec4{1.0, 0.0, 0.0, 0.0}, vec4{0.0, 1.0, 0.0, 0.0}, vec4{0.0, 0.0, 1.0, 0.0},
             vec4{0.0, 0.0, 0.0, 1.0}},
        seedPoints, /*wrapping*/ clamp3D, /*weights*/ std::vector<float>{1.0, 2.0});

    const VolumeRAMPrecision<unsigned short>* ramtyped =
        dynamic_cast<const VolumeRAMPrecision<unsigned short>*>(
            volumeVoronoi->getRepresentation<VolumeRAM>());

    EXPECT_TRUE(ramtyped != nullptr);

    const auto data = ramtyped->getDataTyped();
    const util::IndexMapper3D im(dimensions);

    for (size_t z = 0; z < dimensions.z; z++) {
        for (size_t y = 0; y < dimensions.y; y++) {
            // First part (smaller due to smaller weight)
            for (size_t x1 = 0; x1 < dimensions.x / 4; x1++) {
                const auto val1 = static_cast<unsigned short>(data[im(x1, y, z)]);
                EXPECT_EQ(val1, seedPoints[0].first);
            }
            // Second part (larger due to larger weight)
            for (size_t x2 = dimensions.x / 4; x2 < dimensions.x; x2++) {
                const auto val2 = static_cast<unsigned short>(data[im(x2, y, z)]);
                EXPECT_EQ(val2, seedPoints[1].first);
            }
        }
    }
}

TEST(VolumeVoronoi, Voronoi_ThreeSeedPoints_PartitionsVolumeInThree) {
    const std::vector<std::pair<uint32_t, vec3>> seedPoints = {
        {1, vec3{0, 1, 1}}, {2, vec3{1, 1, 1}}, {3, vec3{2, 1, 1}}};
    const auto dimensions = size3_t{3, 3, 3};

    auto volumeVoronoi =
        util::voronoiSegmentation(dimensions,
                                  /*indexToModelMatrix*/
                                  mat4{vec4{1.0, 0.0, 0.0, 0.0}, vec4{0.0, 1.0, 0.0, 0.0},
                                       vec4{0.0, 0.0, 1.0, 0.0}, vec4{0.0, 0.0, 0.0, 1.0}},
                                  seedPoints, /*wrapping*/ clamp3D, /*weights*/ std::nullopt);

    const VolumeRAMPrecision<unsigned short>* ramtyped =
        dynamic_cast<const VolumeRAMPrecision<unsigned short>*>(
            volumeVoronoi->getRepresentation<VolumeRAM>());

    EXPECT_TRUE(ramtyped != nullptr);

    const auto data = ramtyped->getDataTyped();
    const util::IndexMapper3D im(dimensions);

    for (size_t z = 0; z < dimensions.z; z++) {
        for (size_t y = 0; y < dimensions.y; y++) {
            // First part
            for (size_t x1 = 0; x1 < dimensions.x / 3; x1++) {
                const auto val1 = static_cast<unsigned short>(data[im(x1, y, z)]);
                EXPECT_EQ(val1, seedPoints[0].first);
            }
            // Second part
            for (size_t x2 = dimensions.x / 3; x2 < 2 * dimensions.x / 3; x2++) {
                const auto val2 = static_cast<unsigned short>(data[im(x2, y, z)]);
                EXPECT_EQ(val2, seedPoints[1].first);
            }
            // Third part
            for (size_t x3 = 2 * dimensions.x / 3; x3 < dimensions.x; x3++) {
                const auto val3 = static_cast<unsigned short>(data[im(x3, y, z)]);
                EXPECT_EQ(val3, seedPoints[2].first);
            }
        }
    }
}

}  // namespace inviwo
