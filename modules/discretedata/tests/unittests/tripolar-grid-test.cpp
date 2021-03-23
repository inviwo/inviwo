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
#include <inviwo/core/util/zip.h>

namespace inviwo {
namespace discretedata {

template <GridPrimitive From, GridPrimitive To>
void TestConnections(const TripolarGrid<3>& grid, const std::array<ind, 3>& coord,
                     const std::array<ind, size_t(From)>& dirs, ind expectedIdx,
                     const std::vector<std::array<ind, 3>>& neighCoords,
                     const std::vector<std::array<ind, size_t(To)>> neighDirs,
                     const std::vector<ind>& neighIndices) {

    auto cell = grid.getPrimitive<From>(coord030, dirs);

    EXPECT_EQ(cell.getCoordinates(), coord);
    EXPECT_EQ(cell.getDirections(), dirs);
    EXPECT_EQ(cell.GlobalPrimitiveIndex, expectedIdx);

    cell = grid.getPrimitive<From>(expectedIdx);
    EXPECT_EQ(cell.getCoordinates(), coord);
    EXPECT_EQ(cell.getDirections(), dirs);

    for (auto&& neighbor : util::zip(neighCoords, neighDirs, neighIndices)) {
        auto prim = grid.getPrimitive<To>(neighbor.first(), neighbor.second());
        EXPECT_EQ(prim.GlobalPrimitiveIndex, neighbor.third());
    }
    std::vector<ind> connections;
    grid.getConnections(connections, cell.GlobalPrimitiveIndex, From, To);
    std::sort(connections.begin(), connections.end());
    EXPECT_EQ(neighborIndices, connections);
}

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
    ind numFacesXZ = 10 * 4 * 2;        // XZ-face
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

    // Check number of "normal" primitives (excluding the top-wrapping flap).
    EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[0], numVertices);
    EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[1], numEdgesX);
    EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[2], 10 * 3 * 3);
    EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[3], numEdgesZ);
    EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[4], 10 * 3 * 3);
    EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[5], numFacesXZ);
    EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[6], 10 * 3 * 2);
    EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[7], 10 * 3 * 2);

    std::array<size_t, 3> dirVox({0, 1, 2});

    // Create a primitive object connections representing the voxel at (0, 2, 0).
    // {
    //     std::array<ind, 3> coord020({0, 2, 0});
    //     auto cell020 = grid.getPrimitive<GridPrimitive::Volume>(coord020, dirVox);

    //     EXPECT_EQ(cell020.getCoordinates(), coord020);
    //     EXPECT_EQ(cell020.getDirections(), dirVox);
    //     EXPECT_EQ(cell020.GlobalPrimitiveIndex, 20);
    //     std::vector<std::array<ind, 3>> neighbors{
    //         {0, 1, 0}, {1, 2, 0}, {9, 2, 0}, {0, 2, 1}, {0, 3, 0}};
    //     std::vector<ind> neighborIndices = {10, 21, 29, 50, 60};
    //     for (auto&& neighbor : util::zip(neighbors, neighborIndices)) {
    //         auto prim = grid.getPrimitive<GridPrimitive::Volume>(neighbor.first(), dirVox);
    //         EXPECT_EQ(prim.GlobalPrimitiveIndex, neighbor.second());
    //     }
    //     std::vector<ind> connections;
    //     grid.getConnections(connections, cell020.GlobalPrimitiveIndex, GridPrimitive::Volume,
    //                         GridPrimitive::Volume);
    //     std::sort(connections.begin(), connections.end());
    //     EXPECT_EQ(neighborIndices, connections);
    // }

    // Create a primitive object representing the voxel at (0, 3, 0),
    // one of two most special cases.
    {
        std::array<ind, 3> coord030({0, 3, 0});
        auto cell030 = grid.getPrimitive<GridPrimitive::Volume>(coord030, dirVox);

        EXPECT_EQ(cell030.getCoordinates(), coord030);
        EXPECT_EQ(cell030.getDirections(), dirVox);
        EXPECT_EQ(cell030.GlobalPrimitiveIndex,
                  grid.numPrimitives_.PerDirectionNumNormalPrimitives[7]);

        cell030 = grid.getPrimitive<GridPrimitive::Volume>(60);
        EXPECT_EQ(cell030.getCoordinates(), coord030);
        EXPECT_EQ(cell030.getDirections(), dirVox);

        std::vector<std::array<ind, 3>> neighbors{
            {0, 2, 0}, {8, 2, 0}, {9, 2, 0}, {1, 3, 0}, {0, 3, 1}};
        std::vector<ind> neighborIndices = {20, 28, 29, 61, 64};
        for (auto&& neighbor : util::zip(neighbors, neighborIndices)) {
            auto prim = grid.getPrimitive<GridPrimitive::Volume>(neighbor.first(), dirVox);
            EXPECT_EQ(prim.GlobalPrimitiveIndex, neighbor.second());
        }
        std::vector<ind> connections;
        grid.getConnections(connections, cell030.GlobalPrimitiveIndex, GridPrimitive::Volume,
                            GridPrimitive::Volume);
        std::sort(connections.begin(), connections.end());
        EXPECT_EQ(neighborIndices, connections);
    }

}  // namespace discretedata

}  // namespace discretedata
}  // namespace inviwo
