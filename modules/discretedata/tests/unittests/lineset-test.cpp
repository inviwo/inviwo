/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/discretedata/dataset.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <modules/discretedata/connectivity/lineset.h>
#include <modules/discretedata/connectivity/connectioniterator.h>

namespace inviwo {
namespace discretedata {

TEST(DataSet, LineSetConnectivity) {
#if false

    auto lines = std::make_shared<LineSet>(std::vector<ind>({5, 2, 3, 90, 1, 5}));
    DataSet dataset(lines);

    auto vertIdcs = std::make_shared<BufferChannel<int, 3>>(dataset.getGrid()->getNumElements(),
                                                            "VertexIndices", GridPrimitive::Vertex);
    auto edgeIdcs = std::make_shared<BufferChannel<int, 3>>(
        dataset.getGrid()->getNumElements(GridPrimitive::Edge), "EdgeIndices", GridPrimitive::Edge);

    // Save the total index, line and line index for each vertex and edge.
    ind idxV = 0;
    ind idxE = 0;
    for (ind l = 0; l < lines->getNumLines(); ++l) {
        ind v;
        for (v = 0; v < lines->getLengthLine(l) - 1; ++v) {
            vertIdcs->get(idxV) = {idxV, l, v};
            ++idxV;
            edgeIdcs->get(idxE) = {idxE, l, v};
            ++idxE;
        }
        vertIdcs->get(idxV) = {idxV, l, v};
        ++idxV;
    }

    dataset.addChannel(vertIdcs);
    dataset.addChannel(edgeIdcs);
    auto vertChannel = dataset.getChannel<int, 3>("VertexIndices", GridPrimitive::Vertex);
    auto edgeChannel = dataset.getChannel<int, 3>("EdgeIndices", GridPrimitive::Edge);

    EXPECT_TRUE(vertChannel) << "No vertex channel got";
    EXPECT_TRUE(edgeChannel) << "No edge channel got";

    // Check vertex neighbors.
    for (auto vertex : dataset.getGrid()->all(GridPrimitive::Vertex)) {
        ivec3 vertVal, neighVal;
        vertChannel->fill(vertVal, vertex);

        for (auto vertexNeighbor : vertex.connection(GridPrimitive::Vertex)) {
            vertChannel->fill(neighVal, vertexNeighbor);
            EXPECT_EQ(vertVal.y, neighVal.y)
                << "Neighboring points not on same line:\n\t" << vertVal << ", neigh " << neighVal;
            EXPECT_TRUE(vertVal.x == neighVal.x + 1 || vertVal.x == neighVal.x - 1)
                << "Neighboring vertices not neighboring in index:\n\t" << vertVal << ", neigh "
                << neighVal;
            EXPECT_TRUE(vertVal.z == neighVal.z + 1 || vertVal.z == neighVal.z - 1)
                << "Neighboring vertices not neighboring in line index:\n\t" << vertVal
                << ", neigh " << neighVal;
        }

        for (auto edgeNeighbor : vertex.connection(GridPrimitive::Edge)) {
            edgeChannel->fill(neighVal, edgeNeighbor);
            EXPECT_EQ(vertVal.y, neighVal.y)
                << "Neighboring edge not on same line:\n\t" << vertVal << ", edge " << neighVal;
            EXPECT_TRUE(vertVal.z == neighVal.z + 1 || vertVal.z == neighVal.z)
                << "Neighboring edge not neighboring in line index:\n\t" << vertVal << ", edge "
                << neighVal;
            EXPECT_TRUE(vertVal.x == neighVal.x + neighVal.y + 1 ||
                        vertVal.x == neighVal.x + neighVal.y)
                << "Neighboring edge does not have the right offset:\n\t" << vertVal << ", edge "
                << neighVal;
        }
    }

    // Check edge neighbors.
    for (auto vertex : dataset.getGrid()->all(GridPrimitive::Edge)) {
        ivec3 edgeVal, neighVal;
        edgeChannel->fill(edgeVal, vertex);

        for (auto vertexNeighbor : vertex.connection(GridPrimitive::Vertex)) {
            vertChannel->fill(neighVal, vertexNeighbor);
            EXPECT_EQ(edgeVal.y, neighVal.y)
                << "Neighboring point not on same line:\n\t" << edgeVal << ", neigh " << neighVal;
            EXPECT_TRUE(edgeVal.x + edgeVal.y == neighVal.x - 1 ||
                        edgeVal.x + edgeVal.y == neighVal.x)
                << "Neighboring vertex not neighboring in index:\n\t" << edgeVal << ", neigh "
                << neighVal;
            EXPECT_TRUE(edgeVal.z == neighVal.z - 1 || edgeVal.z == neighVal.z)
                << "Neighboring vertex not neighboring in line index:\n\t" << edgeVal << ", neigh "
                << neighVal;
        }

        for (auto edgeNeighbor : vertex.connection(GridPrimitive::Edge)) {
            edgeChannel->fill(neighVal, edgeNeighbor);
            EXPECT_EQ(edgeVal.y, neighVal.y)
                << "Neighboring edges not on same line:\n\t" << edgeVal << ", edge " << neighVal;
            EXPECT_TRUE(edgeVal.z == neighVal.z + 1 || edgeVal.z == neighVal.z - 1)
                << "Neighboring edges not neighboring in line index:\n\t" << edgeVal << ", edge "
                << neighVal;
            EXPECT_TRUE(edgeVal.x == neighVal.x + 1 || edgeVal.x == neighVal.x - 1)
                << "Neighboring edges does not have the right offset:\n\t" << edgeVal << ", edge "
                << neighVal;
        }
    }
#endif
}

}  // namespace discretedata
}  // namespace inviwo
