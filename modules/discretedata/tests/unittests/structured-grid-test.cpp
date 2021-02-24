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

#include <modules/discretedata/connectivity/structuredgrid.h>

namespace inviwo {
namespace discretedata {

TEST(DataSet, StructuredGrid) {
    StructuredGrid<3> grid(3, 4, 5);

    // Size.
    EXPECT_EQ(grid.numPrimitives_[0], 3);
    EXPECT_EQ(grid.numPrimitives_[1], 4);
    EXPECT_EQ(grid.numPrimitives_[2], 5);

    // Number of total primitives of each type.
    EXPECT_EQ(grid.getNumElements(GridPrimitive::Vertex), 60);
    EXPECT_EQ(grid.getNumElements(GridPrimitive::Edge), 133);
    EXPECT_EQ(grid.getNumElements(GridPrimitive::Face), 98);
    EXPECT_EQ(grid.getNumElements(GridPrimitive::Volume), 24);

    // // Indexing by direction tuples.
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Vertex>({}), 0);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Edge>({0}), 0);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Edge>({1}), 1);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Edge>({2}), 2);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Face>({0, 1}), 0);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Face>({0, 2}), 1);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Face>({1, 2}), 2);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Volume>({0, 1, 2}), 0);

    // Offsets of a certain direction tuple withing the primitive type.
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Vertex>({}), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Vertex>(0), 0);

    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>({0}), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>(0), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>({1}), 40);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>(1), 40);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>({2}), 85);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>(2), 85);

    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>({0, 1}), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>(0), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>({0, 2}), 30);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>(1), 30);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>({1, 2}), 62);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>(2), 62);

    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Volume>({0, 1, 2}), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Volume>(0), 0);

    // auto offsets = dd_util::binomialCoefficientOffsets<3>();

    // Number *of primitives in a certain direction.
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Vertex>(0), 60);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Vertex>({0}), 60);

    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>(0), 40);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>({0}), 40);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>(1), 45);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>({1}), 45);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>(2), 48);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>({2}), 48);

    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>({0, 1}), 30);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>(0), 30);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>({0, 2}), 32);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>(1), 32);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>({1, 2}), 36);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>(2), 36);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Volume>({0, 1, 2}), 24);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Volume>(0), 24);

    // Create a primitive object representing the voxel at (0, 2, 3).
    auto cell123 = grid.getPrimitive<GridPrimitive::Volume>({0, 2, 3}, {0, 1, 2});
    std::array<ind, 3> c023({0, 2, 3});
    std::array<size_t, 3> d012({0, 1, 2});
    EXPECT_EQ(cell123.getCoordinates(), c023);
    EXPECT_EQ(cell123.getDirections(), d012);
    EXPECT_EQ(cell123.GlobalPrimitiveIndex, 2 * 2 + 3 * 2 * 3);

    // Check all voxel neighbors - since there is only 2 x 3 x 4 voxels, we get 3 of those.
    std::vector<ind> neighbors;
    grid.getConnections(neighbors, cell123.GlobalPrimitiveIndex, GridPrimitive::Volume,
                        GridPrimitive::Volume);

    std::sort(neighbors.begin(), neighbors.end());
    EXPECT_EQ(neighbors.size(), 3);
    EXPECT_EQ(neighbors[0], 16);
    EXPECT_EQ(neighbors[1], 20);
    EXPECT_EQ(neighbors[2], 23);

    // Get all edges of our voxel. There should be 12.
    std::vector<ind> edges;
    grid.getConnections(edges, cell123.GlobalPrimitiveIndex, GridPrimitive::Volume,
                        GridPrimitive::Edge);
    EXPECT_EQ(edges.size(), 12);
    // X-pointing edges:
    EXPECT_EQ(edges[0], 28);
    EXPECT_EQ(edges[1], 30);
    EXPECT_EQ(edges[2], 36);
    EXPECT_EQ(edges[3], 38);
    // Y-pointing edges:
    EXPECT_EQ(edges[4], 40 + 33);
    EXPECT_EQ(edges[5], 40 + 34);
    EXPECT_EQ(edges[6], 40 + 42);
    EXPECT_EQ(edges[7], 40 + 43);
    // Z-pointing edges:
    EXPECT_EQ(edges[8], 85 + 42);
    EXPECT_EQ(edges[9], 85 + 43);
    EXPECT_EQ(edges[10], 85 + 45);
    EXPECT_EQ(edges[11], 85 + 46);

    // Check one of the edges at random for resolving back to posiiton and direction.
    auto edge124y = grid.getPrimitive<GridPrimitive::Edge>(40 + 43);
    std::array<ind, 3> c124({1, 2, 4});
    EXPECT_EQ(edge124y.getCoordinates(), c124);
    EXPECT_EQ(edge124y.getDirections()[0], 1);
}

}  // namespace discretedata
}  // namespace inviwo
