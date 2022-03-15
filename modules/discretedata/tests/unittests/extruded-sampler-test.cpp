/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <modules/discretedata/channels/extendedchannel.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <modules/discretedata/connectivity/structuredgrid.h>
#include <modules/discretedata/sampling/extrudeddatasetsampler.h>
#include <modules/discretedata/sampling/celltree.h>
#include <modules/discretedata/sampling/datasetspatialsampler.h>

namespace inviwo {
namespace discretedata {

TEST(DataSet, ExtrudedSampler) {
    using FArr2 = std::array<float, 2>;
    using FArr3 = std::array<float, 3>;
    using IArr8 = std::array<ind, 8>;

    auto gridXY = std::make_shared<CurvilinearGrid<2>>(5, 3);
    auto positionsXY = std::make_shared<AnalyticChannel<double, 2, dvec2>>(
        [](dvec2& vec, ind idx) { vec = dvec2(2, 1) * (idx % 5) + dvec2(-1, 1) * (idx / 5); },
        5 * 3, "posXY");
    auto positionsZ = std::make_shared<AnalyticChannel<double, 1, double>>(
        [](double& vec, ind idx) { vec = idx * idx; }, 4, "posZ");
    ExtendedChannel<double, 3> positionsXYZ(positionsXY, positionsZ, "posXYZ");

    auto samplerXY = std::make_shared<CellTree<2>>(gridXY, positionsXY, SkewedBoxInterpolant<2>(),
                                                   FArr2{-2, 0}, FArr2{8, 6});

    auto extrudedSampler = std::make_shared<ExtrudedDataSetSampler<3>>(samplerXY, positionsZ);

    EXPECT_EQ(extrudedSampler->getInterpolant().getDimension(), 3) << "Interpolant should be 3D.";

    auto* extInterpolant =
        dynamic_cast<const ExtendedInterpolant<3>*>(&extrudedSampler->getInterpolant());
    EXPECT_TRUE(extInterpolant) << "Interpolant should be an extended interpolant.";

    EXPECT_TRUE(extInterpolant->supportsInterpolationType(InterpolationType::Linear))
        << "Base interpolant should support lienar interpolation.";

    EXPECT_TRUE(
        extrudedSampler->getInterpolant().supportsInterpolationType(InterpolationType::Linear))
        << "Interpolant should support linar interpolation.";

    EXPECT_EQ(extrudedSampler->grid_->getDimension(), GridPrimitive::Volume)
        << "Grid should be 3D.";

    EXPECT_EQ(extrudedSampler->grid_->getNumElements(GridPrimitive::Vertex), 60)
        << "Grid should have 3 * 5 * 4 vertices.";

    auto coordinates =
        std::dynamic_pointer_cast<const ExtendedChannel<double, 3>>(extrudedSampler->coordinates_);

    // {
    //     SCOPED_TRACE("Extruded Positions");
    //     EXPECT_TRUE(coordinates);
    //     std::cout << "= Channel name: " << coordinates->getName() << std::endl;

    //     std::array<ivec3, 5> testCoords = {ivec3{0, 0, 0}, ivec3{1, 1, 1}, ivec3{1, 2, 3},
    //                                        ivec3{3, 2, 1}, ivec3{1, 0, 1}};
    //     dvec3 sample(-42.0), sampleXYZ(-42.0);
    //     dvec2 sampleXY(-42.0);
    //     double sampleZ = -42.0;
    //     for (const auto& coord : testCoords) {
    //         size_t linearIndex = coord.x + coord.y * 5 + coord.z * 5 * 3;
    //         SCOPED_TRACE(
    //             fmt::format("Coord [{}, {}, {}] ({})", coord.x, coord.y, coord.z, linearIndex));

    //         positionsXY->fill(sampleXY, coord.x + coord.y * 5);
    //         positionsZ->fill(sampleZ, coord.z);
    //         positionsXYZ.fill(sampleXYZ, linearIndex);
    //         coordinates->fill(sample, linearIndex);
    //         dvec3 expected = dvec3(2, 1, 0) * double(coord.x) + dvec3(-1, 1, 0) * double(coord.y)
    //         +
    //                          dvec3(0, 0, coord.z * coord.z);

    //         EXPECT_DOUBLE_EQ(sampleXY.x, expected.x);
    //         EXPECT_DOUBLE_EQ(sampleXY.y, expected.y);
    //         EXPECT_DOUBLE_EQ(sampleZ, expected.z);

    //         EXPECT_DOUBLE_EQ(sampleXYZ.x, expected.x);
    //         EXPECT_DOUBLE_EQ(sampleXYZ.y, expected.y);
    //         EXPECT_DOUBLE_EQ(sampleXYZ.z, expected.z);

    //         EXPECT_DOUBLE_EQ(sample.x, expected.x);
    //         EXPECT_DOUBLE_EQ(sample.y, expected.y);
    //         EXPECT_DOUBLE_EQ(sample.z, expected.z);
    //     }
    // }

    // {
    //     SCOPED_TRACE("DataSet Sampler Indices");

    //     struct TestSample {
    //         TestSample(FArr3 p, IArr8 v, ind c) : position(p), vertices(v), cell(c) {}
    //         FArr3 position;
    //         IArr8 vertices;
    //         ind cell;
    //     };

    //     const std::array<TestSample, 3> testSamples = {
    //         TestSample(FArr3{0, 1, 0.5f}, IArr8{0, 1, 5, 6, 15, 16, 20, 21}, 0),
    //         TestSample(FArr3{5, 3, 2}, IArr8{17, 18, 22, 23, 32, 33, 37, 38}, 10),
    //         TestSample(FArr3{2.5f, 3, 8}, IArr8{36, 37, 41, 42, 51, 52, 56, 57}, 21)};
    //     std::vector<double> weights;
    //     std::vector<ind> vertices;

    //     for (TestSample& sample : testSamples) {
    //         SCOPED_TRACE(fmt::format("Position [{}, {}, {}]", sample.position[0],
    //                                  sample.position[1], sample.position[2]));
    //         weights.clear();
    //         vertices.clear();
    //         ind c = extrudedSampler->locateAndSampleCell(sample.position, weights, vertices,
    //                                                      InterpolationType::Linear);

    //         EXPECT_EQ(c, sample.cell);
    //         EXPECT_EQ(vertices.size(), 8);

    //         for (auto&& v : util::zip(vertices, sample.vertices)) {
    //             EXPECT_EQ(v.first(), v.second());
    //         }
    //     }
    // }

    // {
    //     SCOPED_TRACE("DataSet Sampler Weights");

    //     std::array<vec3, 4> testDataCoords = {vec3{1.5, 0.5, 1.5}, vec3{0.1, 1.9, 2.9},
    //                                           vec3{3.8, 0.2, 2.7}, vec3{2.1, 0.9, 1.5}};
    //     std::vector<double> weights;
    //     std::vector<ind> vertices;

    //     for (const vec3& dataCoords : testDataCoords) {
    //         weights.clear();
    //         vertices.clear();

    //         vec2 posXY = vec2(2, 1) * dataCoords.x + vec2(-1, 1) * dataCoords.y;
    //         float posZ = (long)dataCoords.z * (long)dataCoords.z +
    //                      (dataCoords.z - (long)dataCoords.z) * ((long)dataCoords.z * 2 + 1);

    //         SCOPED_TRACE(fmt::format("Data Coord: [{}, {}, {}] - Position: [{}, {}, {}]",
    //                                  dataCoords.x, dataCoords.y, dataCoords.z, posXY.x, posXY.y,
    //                                  posZ));
    //         ind cell = extrudedSampler->locateAndSampleCell(FArr3{posXY.x, posXY.y, posZ},
    //         weights,
    //                                                         vertices, InterpolationType::Linear);
    //         vec3 interp{dataCoords.x - (int)dataCoords.x, dataCoords.y - (int)dataCoords.y,
    //                     dataCoords.z - (int)dataCoords.z};

    //         EXPECT_GE(cell, 0) << "Not inside grid.";
    //         if (cell >= 0) {
    //             EXPECT_EQ(weights.size(), 8);
    //             float eps = 0.001;
    //             EXPECT_NEAR(weights[0], (1 - interp.x) * (1 - interp.y) * (1 - interp.z), eps);
    //             EXPECT_NEAR(weights[1], interp.x * (1 - interp.y) * (1 - interp.z), eps);
    //             EXPECT_NEAR(weights[2], (1 - interp.x) * interp.y * (1 - interp.z), eps);
    //             EXPECT_NEAR(weights[3], interp.x * interp.y * (1 - interp.z), eps);

    //             EXPECT_NEAR(weights[4], (1 - interp.x) * (1 - interp.y) * interp.z, eps);
    //             EXPECT_NEAR(weights[5], interp.x * (1 - interp.y) * interp.z, eps);
    //             EXPECT_NEAR(weights[6], (1 - interp.x) * interp.y * interp.z, eps);
    //             EXPECT_NEAR(weights[7], interp.x * interp.y * interp.z, eps);
    //         }
    //     }
    // }

    // {
    //     SCOPED_TRACE("DataSet SpatialSampler");
    //     DataSetSpatialSampler3D positionSampler(extrudedSampler, InterpolationType::Linear,
    //                                             coordinates);
    //     std::array<dvec3, 9> testCoordsInside = {
    //         dvec3{0, 1, 0.5f},    dvec3{5, 3, 2},     dvec3{2.5f, 3, 8},    // From sampler tests
    //         dvec3{6.5, 5.2, 8.8}, dvec3{4.1, 3.6, 6}, dvec3{-1, 2.1, 4.2},  // Random positions
    //         dvec3{3, 2, 1},       dvec3{1, 2, 3},     dvec3{5, 4, 4}        // On
    //         face/edge/vertex
    //     };
    //     std::array<dvec3, 6> testCoordsOutside = {dvec3{3, 1.1, 3},  dvec3{-1, 3, 2},
    //                                               dvec3{7, 5.5, 5},  dvec3{-2, 1.5, 6},
    //                                               dvec3{3, 3, -0.5}, dvec3{3, 3, 9.5}};
    //     for (dvec3& posIn : testCoordsInside) {

    //         EXPECT_TRUE(positionSampler.withinBoundsDataSpace(posIn));
    //         dvec3 sample = positionSampler.sample(posIn, CoordinateSpace::Model);

    //         SCOPED_TRACE(fmt::format("Sampling [{}, {}, {}] -> [{}, {}, {}]", posIn.x, posIn.y,
    //                                  posIn.z, sample.x, sample.y, sample.z));

    //         EXPECT_NEAR(sample.x, posIn.x, 0.001);
    //         EXPECT_NEAR(sample.y, posIn.y, 0.001);
    //         EXPECT_NEAR(sample.z, posIn.z, 0.001);
    //     }

    //     for (dvec3& posOut : testCoordsOutside) {
    //         EXPECT_FALSE(positionSampler.withinBoundsDataSpace(posOut));
    //         dvec3 sample = positionSampler.sample(posOut, CoordinateSpace::Model);
    //         EXPECT_EQ(sample.x, 0);
    //     }
    // }

    {
        SCOPED_TRACE("Extruded Grid");

        auto grid = std::dynamic_pointer_cast<const ExtrudedGrid>(extrudedSampler->grid_);
        EXPECT_TRUE(grid);

        EXPECT_EQ(grid->getNumElements(GridPrimitive::Vertex), 5 * 3 * 4);
        EXPECT_EQ(grid->getNumElements(GridPrimitive::Edge),
                  (4 * 3 * 4) + (5 * 2 * 4) + (5 * 3 * 3));
        EXPECT_EQ(grid->getNumElements(GridPrimitive::Face),
                  (4 * 2 * 4) + (4 * 3 * 3) + (5 * 2 * 3));
        EXPECT_EQ(grid->getNumElements(GridPrimitive::Volume), 4 * 2 * 3);

        struct TestConnection {
            ind fromIndex_;
            GridPrimitive from_, to_;
            std::vector<ind> toIndices_;
            bool keepOrder_ = true;
        };

        std::vector<ind> connections;
        std::array<TestConnection, 18> tests = {
            TestConnection{
                0, GridPrimitive::Volume, GridPrimitive::Vertex, {0, 1, 5, 6, 15, 16, 20, 21}},
            TestConnection{
                14, GridPrimitive::Volume, GridPrimitive::Vertex, {22, 23, 27, 28, 37, 38, 42, 43}},

            TestConnection{0, GridPrimitive::Edge, GridPrimitive::Vertex, {0, 1}},
            TestConnection{12, GridPrimitive::Edge, GridPrimitive::Vertex, {0, 5}},
            TestConnection{88 + 0, GridPrimitive::Edge, GridPrimitive::Vertex, {0, 15}},

            TestConnection{28, GridPrimitive::Edge, GridPrimitive::Vertex, {22, 23}},
            TestConnection{65, GridPrimitive::Edge, GridPrimitive::Vertex, {39, 44}},
            TestConnection{88 + 32, GridPrimitive::Edge, GridPrimitive::Vertex, {32, 47}},

            TestConnection{14, GridPrimitive::Face, GridPrimitive::Vertex, {22, 23, 27, 28}},
            TestConnection{32 + 23, GridPrimitive::Face, GridPrimitive::Vertex, {16, 17, 31, 32}},
            TestConnection{
                32 + 65, GridPrimitive::Face, GridPrimitive::Vertex, {39, 44, 54, 59}, false},

            TestConnection{1, GridPrimitive::Face, GridPrimitive::Edge, {1, 13, 14, 5}, false},
            TestConnection{
                32 + 1, GridPrimitive::Face, GridPrimitive::Edge, {1, 23, 88 + 1, 88 + 2}, false},
            TestConnection{32 + 43,
                           GridPrimitive::Face,
                           GridPrimitive::Edge,
                           {43, 65, 88 + 24, 88 + 29},
                           false},

            TestConnection{13,
                           GridPrimitive::Volume,
                           GridPrimitive::Edge,
                           {27, 31, 40, 41, 49, 53, 62, 63, 88 + 21, 88 + 22, 88 + 26, 88 + 27},
                           false},

            TestConnection{0,
                           GridPrimitive::Volume,
                           GridPrimitive::Face,
                           {0, 8, 32 + 0, 32 + 4, 32 + 12, 32 + 13},
                           false},
            TestConnection{13,
                           GridPrimitive::Volume,
                           GridPrimitive::Face,
                           {13, 21, 32 + 27, 32 + 31, 32 + 40, 32 + 41},
                           false},

            TestConnection{9, GridPrimitive::Volume, GridPrimitive::Volume, {8, 10, 13, 17}, false}

            // TestConnection{22,
            //                GridPrimitive::Vertex,
            //                GridPrimitive::Volume,
            //                {1, 2, 5, 6, 9, 10, 13, 14},
            //                false}

        };

        for (auto& test : tests) {
            connections.clear();
            grid->getConnections(connections, test.fromIndex_, test.from_, test.to_);
            if (!test.keepOrder_) {
                std::sort(connections.begin(), connections.end());
                std::sort(test.toIndices_.begin(), test.toIndices_.end());
            }

            SCOPED_TRACE(fmt::format("From {} {} to {}\n\t Expect [{}] == [{}]",
                                     primitiveName(test.from_), test.fromIndex_,
                                     primitiveName(test.to_, true), fmt::join(connections, ", "),
                                     fmt::join(test.toIndices_, ", ")));

            EXPECT_EQ(connections.size(), test.toIndices_.size());

            for (auto&& c : util::zip(connections, test.toIndices_)) {
                EXPECT_EQ(c.first(), c.second());
            }
        }
    }
}

}  // namespace discretedata
}  // namespace inviwo
