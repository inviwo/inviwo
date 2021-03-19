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

#include <modules/discretedata/connectivity/tripolargrid.h>

namespace inviwo {
namespace discretedata {

TEST(DataSet, TripolarGrid) {
    TripolarGrid<3> grid(10, 4, 3);

    // Size.
    EXPECT_EQ(grid.numPrimitives_[0], 10);
    EXPECT_EQ(grid.numPrimitives_[1], 4);
    EXPECT_EQ(grid.numPrimitives_[2], 3);

    // Number of primitives of each type.
    ind numVertices = 10 * 4 * 3;

    ind numEdgesX = 10 * 4 * 3;        // X-edge
    ind numEdgesY = (10 * 3 + 3) * 3;  // Y-edge
    ind numEdgesZ = 10 * 4 * 2;        // Z-edge

    ind numFacesXY = (10 * 3 + 4) * 3;  // XY-face
    ind numFacesXZ = 10 * 4 * 3;        // XZ-face
    ind numFacesYZ = (10 * 3 + 3) * 2;  // YZ-face

    ind numVoxels = (10 * 3 + 4) * 2;

    // Number of primitives per direction and total per primitive.
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Vertex>(0), numVertices);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Vertex>({0}), numVertices);
    EXPECT_EQ(grid.getNumElements(GridPrimitive::Vertex), numVertices);

    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>(0), numEdgesX);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>({0}), numEdgesX);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>(1), numEdgesY);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>({1}), numEdgesY);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>(2), numEdgesZ);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>({2}), numEdgesZ);
    EXPECT_EQ(grid.getNumElements(GridPrimitive::Edge), numEdgesX + numEdgesY + numEdgesZ);

    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>({0, 1}), numFacesXY);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>(0), numFacesXY);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>({0, 2}), numFacesXZ);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>(1), numFacesXZ);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>({1, 2}), numFacesYZ);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>(2), numFacesYZ);
    EXPECT_EQ(grid.getNumElements(GridPrimitive::Face), numFacesXY + numFacesXZ + numFacesYZ);

    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Volume>({0, 1, 2}), numVoxels);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Volume>(0), numVoxels);
    EXPECT_EQ(grid.getNumElements(GridPrimitive::Volume), numVoxels);

    // Check the number of normal voxels.
    const auto& numNormal = grid.numPrimitives_.PerDirectionNumNormalPrimitives;
    EXPECT_EQ(numNormal[0], numVertices);
    EXPECT_EQ(numNormal[1], numEdgesX);
    EXPECT_EQ(numNormal[2], 10 * 3 * 3);
    EXPECT_EQ(numNormal[3], numEdgesZ);
    EXPECT_EQ(numNormal[4], 10 * 3 * 3);
    EXPECT_EQ(numNormal[5], numFacesXZ);
    EXPECT_EQ(numNormal[6], 10 * 3 * 2);
    EXPECT_EQ(numNormal[7], 10 * 3 * 2);

    // Indexing by direction tuples.
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
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>({1}), numEdgesX);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>(1), numEdgesX);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>({2}), numEdgesX + numEdgesY);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>(2), numEdgesX + numEdgesY);

    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>({0, 1}), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>(0), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>({0, 2}), numFacesXY);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>(1), numFacesXY);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>({1, 2}), numFacesXY + numFacesXZ);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>(2), numFacesXY + numFacesXZ);

    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Volume>({0, 1, 2}), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Volume>(0), 0);

    // Create a primitive object representing the voxel at (0, 3, 0), one of two most special cases.
    std::array<ind, 3> coord030({0, 3, 0});
    std::array<size_t, 3> dirV({0, 1, 2});
    auto cell030 = grid.getPrimitive<GridPrimitive::Volume>(coord030, dirV);

    EXPECT_EQ(cell030.getCoordinates(), coord030);
    EXPECT_EQ(cell030.getDirections(), dirV);
    EXPECT_EQ(cell030.GlobalPrimitiveIndex, grid.numPrimitives_.PerDirectionNumNormalPrimitives[7]);

    // // Check all voxel neighbors - since there is only 2 x 3 x 4 voxels, we get 3 of those.
    // std::vector<ind> neighbors;
    // grid.getConnections(neighbors, cell123.GlobalPrimitiveIndex, GridPrimitive::Volume,
    //                     GridPrimitive::Volume);

    // std::sort(neighbors.begin(), neighbors.end());
    // EXPECT_EQ(neighbors.size(), 3);
    // EXPECT_EQ(neighbors[0], 16);
    // EXPECT_EQ(neighbors[1], 20);
    // EXPECT_EQ(neighbors[2], 23);

    // // Get all edges of our voxel. There should be 12.
    // std::vector<ind> edges;
    // grid.getConnections(edges, cell123.GlobalPrimitiveIndex, GridPrimitive::Volume,
    //                     GridPrimitive::Edge);
    // EXPECT_EQ(edges.size(), 12);
    // // X-pointing edges:
    // EXPECT_EQ(edges[0], 28);
    // EXPECT_EQ(edges[1], 30);
    // EXPECT_EQ(edges[2], 36);
    // EXPECT_EQ(edges[3], 38);
    // // Y-pointing edges:
    // EXPECT_EQ(edges[4], 40 + 33);
    // EXPECT_EQ(edges[5], 40 + 34);
    // EXPECT_EQ(edges[6], 40 + 42);
    // EXPECT_EQ(edges[7], 40 + 43);
    // // Z-pointing edges:
    // EXPECT_EQ(edges[8], 85 + 42);
    // EXPECT_EQ(edges[9], 85 + 43);
    // EXPECT_EQ(edges[10], 85 + 45);
    // EXPECT_EQ(edges[11], 85 + 46);

    // // Check one of the edges at random for resolving back to posiiton and direction.
    // auto edge124y = grid.getPrimitive<GridPrimitive::Edge>(40 + 43);
    // std::array<ind, 3> c124({1, 2, 4});
    // EXPECT_EQ(edge124y.getCoordinates(), c124);
    // EXPECT_EQ(edge124y.getDirections()[0], 1);
}

}  // namespace discretedata
}  // namespace inviwo
