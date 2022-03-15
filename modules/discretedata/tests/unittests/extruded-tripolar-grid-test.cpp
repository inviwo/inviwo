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
                     const std::array<size_t, size_t(From)>& dirs, ind expectedIdx,
                     const std::vector<std::array<ind, 3>>& neighCoords,
                     const std::vector<std::array<size_t, size_t(To)>> neighDirs,
                     const std::vector<ind>& neighIndices, const std::string& name) {

    auto cell = grid.getPrimitive<From>(coord, dirs);

    EXPECT_EQ(cell.getCoordinates(), coord) << name << " coordinates being kept";
    EXPECT_EQ(cell.getDirections(), dirs) << name << " directions being kept";
    EXPECT_EQ(cell.GlobalPrimitiveIndex, expectedIdx) << name << " index as expected";

    cell = grid.getPrimitive<From>(expectedIdx);
    EXPECT_EQ(cell.getCoordinates(), coord) << name << " coordinates from index";
    EXPECT_EQ(cell.getDirections(), dirs) << name << " directions from index";

    for (auto&& neighbor : util::zip(neighCoords, neighDirs, neighIndices)) {
        auto prim = grid.getPrimitive<To>(neighbor.first(), neighbor.second());
        EXPECT_EQ(prim.GlobalPrimitiveIndex, neighbor.third())
            << name << " neigbor index as expected";
    }
    std::vector<ind> connections;
    grid.getConnections(connections, cell.GlobalPrimitiveIndex, From, To);
    std::sort(connections.begin(), connections.end());
    EXPECT_EQ(neighIndices, connections);
}

TEST(DataSet, ExtrudedTripolarGrid) {
    // TripolarGrid<3> grid(10, 4, 3);

    // // Size.
    // EXPECT_EQ(grid.numPrimitives_[0], 10);
    // EXPECT_EQ(grid.numPrimitives_[1], 4);
    // EXPECT_EQ(grid.numPrimitives_[2], 3);

    // // Number of primitives of each type.
    // ind numVertices = 10 * 4 * 3;

    // ind numEdgesX = 10 * 4 * 3;        // X-edge
    // ind numEdgesY = (10 * 3 + 3) * 3;  // Y-edge
    // ind numEdgesZ = 10 * 4 * 2;        // Z-edge

    // ind numFacesXY = (10 * 3 + 4) * 3;  // XY-face
    // ind numFacesXZ = 10 * 4 * 2;        // XZ-face
    // ind numFacesYZ = (10 * 3 + 3) * 2;  // YZ-face

    // ind numVoxels = (10 * 3 + 4) * 2;

    // // Number of primitives per direction and total per primitive.
    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Vertex>(0), numVertices);
    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Vertex>({0}), numVertices);
    // EXPECT_EQ(grid.getNumElements(GridPrimitive::Vertex), numVertices);

    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>(0), numEdgesX);
    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>({0}), numEdgesX);
    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>(1), numEdgesY);
    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>({1}), numEdgesY);
    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>(2), numEdgesZ);
    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>({2}), numEdgesZ);
    // EXPECT_EQ(grid.getNumElements(GridPrimitive::Edge), numEdgesX + numEdgesY + numEdgesZ);

    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>({0, 1}), numFacesXY);
    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>(0), numFacesXY);
    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>({0, 2}), numFacesXZ);
    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>(1), numFacesXZ);
    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>({1, 2}), numFacesYZ);
    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>(2), numFacesYZ);
    // EXPECT_EQ(grid.getNumElements(GridPrimitive::Face), numFacesXY + numFacesXZ + numFacesYZ);

    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Volume>({0, 1, 2}), numVoxels);
    // EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Volume>(0), numVoxels);
    // EXPECT_EQ(grid.getNumElements(GridPrimitive::Volume), numVoxels);

    // // Check the number of normal voxels.
    // const auto& numNormal = grid.numPrimitives_.PerDirectionNumNormalPrimitives;
    // EXPECT_EQ(numNormal[0], numVertices);
    // EXPECT_EQ(numNormal[1], numEdgesX);
    // EXPECT_EQ(numNormal[2], 10 * 3 * 3);
    // EXPECT_EQ(numNormal[3], numEdgesZ);
    // EXPECT_EQ(numNormal[4], 10 * 3 * 3);
    // EXPECT_EQ(numNormal[5], numFacesXZ);
    // EXPECT_EQ(numNormal[6], 10 * 3 * 2);
    // EXPECT_EQ(numNormal[7], 10 * 3 * 2);

    // // Indexing by direction tuples.
    // EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Vertex>({}), 0);
    // EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Edge>({0}), 0);
    // EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Edge>({1}), 1);
    // EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Edge>({2}), 2);
    // EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Face>({0, 1}), 0);
    // EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Face>({0, 2}), 1);
    // EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Face>({1, 2}), 2);
    // EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Volume>({0, 1, 2}), 0);

    // // Offsets of a certain direction tuple withing the primitive type.
    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Vertex>({}), 0);
    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Vertex>(0), 0);

    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>({0}), 0);
    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>(0), 0);
    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>({1}), numEdgesX);
    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>(1), numEdgesX);
    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>({2}), numEdgesX + numEdgesY);
    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>(2), numEdgesX + numEdgesY);

    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>({0, 1}), 0);
    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>(0), 0);
    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>({0, 2}), numFacesXY);
    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>(1), numFacesXY);
    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>({1, 2}), numFacesXY +
    // numFacesXZ); EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>(2), numFacesXY +
    // numFacesXZ);

    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Volume>({0, 1, 2}), 0);
    // EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Volume>(0), 0);

    // // Check number of "normal" primitives (excluding the top-wrapping flap).
    // EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[0], numVertices);
    // EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[1], numEdgesX);
    // EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[2], 10 * 3 * 3);
    // EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[3], numEdgesZ);
    // EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[4], 10 * 3 * 3);
    // EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[5], numFacesXZ);
    // EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[6], 10 * 3 * 2);
    // EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[7], 10 * 3 * 2);

    // std::array<size_t, 3> dirVox({0, 1, 2});

    // // Create a primitive object connections representing the voxel at (0, 2, 0).
    // TestConnections<GridPrimitive::Volume, GridPrimitive::Volume>(
    //     grid, {0, 2, 0}, dirVox, 20, {{0, 1, 0}, {1, 2, 0}, {9, 2, 0}, {0, 2, 1}, {0, 3, 0}},
    //     std::vector<std::array<size_t, 3>>(5, {0, 1, 2}), {10, 21, 29, 50, 60}, "Neigbors of
    //     020");

    // TestConnections<GridPrimitive::Volume, GridPrimitive::Vertex>(
    //     grid, {0, 2, 0}, dirVox, 20,
    //     {{0, 2, 0}, {1, 2, 0}, {0, 3, 0}, {1, 3, 0}, {0, 2, 1}, {1, 2, 1}, {0, 3, 1}, {1, 3, 1}},
    //     std::vector<std::array<size_t, 0>>(8), {20, 21, 30, 31, 60, 61, 70, 71}, "Vertices of
    //     020");

    // // Create a primitive object representing the voxel at (0, 3, 0),
    // // one of two most special cases.
    // EXPECT_EQ(grid.numPrimitives_.PerDirectionNumNormalPrimitives[7], 60);
    // TestConnections<GridPrimitive::Volume, GridPrimitive::Volume>(
    //     grid, {0, 3, 0}, dirVox, 60, {{0, 2, 0}, {8, 2, 0}, {9, 2, 0}, {1, 3, 0}, {0, 3, 1}},
    //     std::vector<std::array<size_t, 3>>(5, {0, 1, 2}), {20, 28, 29, 61, 64}, "Neigbors of
    //     030");

    // TestConnections<GridPrimitive::Volume, GridPrimitive::Vertex>(
    //     grid, {0, 3, 0}, dirVox, 60,
    //     {{0, 3, 0}, {1, 3, 0}, {8, 3, 0}, {9, 3, 0}, {0, 3, 1}, {1, 3, 1}, {8, 3, 1}, {9, 3, 1}},
    //     std::vector<std::array<size_t, 0>>(8), {30, 31, 38, 39, 70, 71, 78, 79}, "Vertices of
    //     030");

    // // Create a primitive object representing the voxel at (3, 3, 0),
    // // the second most special cases.
    // TestConnections<GridPrimitive::Volume, GridPrimitive::Volume>(
    //     grid, {3, 3, 0}, dirVox, 63, {{3, 2, 0}, {4, 2, 0}, {5, 2, 0}, {2, 3, 0}, {3, 3, 1}},
    //     std::vector<std::array<size_t, 3>>(5, {0, 1, 2}), {23, 24, 25, 62, 67}, "Neigbors of
    //     330");

    // TestConnections<GridPrimitive::Volume, GridPrimitive::Vertex>(
    //     grid, {3, 3, 0}, dirVox, 63,
    //     {{3, 3, 0}, {4, 3, 0}, {5, 3, 0}, {6, 3, 0}, {3, 3, 1}, {4, 3, 1}, {5, 3, 1}, {6, 3, 1}},
    //     std::vector<std::array<size_t, 0>>(8), {33, 34, 35, 36, 73, 74, 75, 76}, "Vertices of
    //     330");

    // // Come from far side of top wrap.
    // TestConnections<GridPrimitive::Volume, GridPrimitive::Volume>(
    //     grid, {6, 2, 0}, dirVox, 26, {{6, 1, 0}, {5, 2, 0}, {7, 2, 0}, {6, 2, 1}, {2, 3, 0}},
    //     std::vector<std::array<size_t, 3>>(5, {0, 1, 2}), {16, 25, 27, 56, 62}, "Neigbors of
    //     620");

}  // namespace discretedata

}  // namespace discretedata
}  // namespace inviwo
