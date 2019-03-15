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
#include <modules/discretedata/channels/bufferchannel.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <modules/discretedata/connectivity/connectivity.h>
#include <modules/discretedata/connectivity/elementiterator.h>
#include <modules/discretedata/connectivity/connectioniterator.h>
#include <modules/discretedata/connectivity/periodicgrid.h>
#include <modules/discretedata/connectivity/euclideanmeasure.h>

namespace inviwo {
namespace discretedata {

TEST(Using, Dataset) {
    // Create a curvelinear grid.
    std::vector<ind> gridSize = {10, 1, 1};
    std::vector<bool> periodic = {true, false, false};
    std::shared_ptr<PeriodicGrid> grid =
        std::make_shared<PeriodicGrid>(GridPrimitive::Volume, gridSize, periodic);

    // Make the x dimension periodic (redundant), that is, the outer yz-planes overlap.
    grid->setPeriodic(0, true);

    // An empty dataset to hold the grid and data on that grid.
    DataSet dataset(grid);

    /*********************************************************************************
     * Create some "data" in a buffer.
     * Buffer stores data explicitly, so load data into them.
     *********************************************************************************/
    // We want to store the data on the vertices (common case), so the size has to match.
    std::vector<int> randomData(grid->getNumElements(GridPrimitive::Vertex) * 3);

    // Fill randomly.
    generate(randomData.begin(), randomData.end(), []() { return rand(); });
    auto randomBuffer =
        std::make_shared<BufferChannel<int, 3>>(randomData, "Random", GridPrimitive::Vertex);

    // Add to the dataset. There, it is kept as a const shared_ptr.
    dataset.addChannel(randomBuffer);

    /*********************************************************************************
     * Other variant of a channel: analytic data.
     * The function is evaluated when accessed.
     *********************************************************************************/
    auto sinChannel = std::make_shared<AnalyticChannel<float, 2, glm::vec2>>(
        // Some function, taking a ref to the data to fill and a linear index.
        [](glm::vec2& data, ind idx) {
            data[0] = sin(0.01f * idx);
            data[1] = cos(0.01f * idx);
        },
        // Lives on the 3D cells, not the vertices.
        grid->getNumElements(GridPrimitive::Volume), "SinCos", GridPrimitive::Volume);
    dataset.addChannel(sinChannel);

    /*********************************************************************************
     * Access data from the DataSet: by name and dimension.
     *********************************************************************************/
    // Get the data as a DataChannel: BufferChannel and AnalyticChannel behave the same.
    auto cellChannel = dataset.getChannel<float, 2>("SinCos", GridPrimitive::Volume);
    EXPECT_TRUE(cellChannel) << "Could not retrieve AnalyticChannel from DataSet.";

    auto vertexChannel = dataset.getChannel<int, 3>("Random", GridPrimitive::Vertex);
    EXPECT_TRUE(vertexChannel) << "Could not retrieve random BufferChannel from DataSet.";

    // Get the data as a Buffer of 3 int, explicitely.
    // Creates a BufferChannel from implicit data, such as Analytic Channel.
    // Returns nullptr if no channel is found, or none with <int, 3>.
    auto vertexBuffer = dataset.getAsBuffer<int, 3>("Random", GridPrimitive::Vertex);
    EXPECT_TRUE(vertexBuffer) << "Could not retrieve random BufferChannel from DataSet as Buffer.";

    /*********************************************************************************
     * Random algorithm: apply an average filter.
     *********************************************************************************/
    std::vector<float> filteredRandom(vertexChannel->getNumComponents() * vertexChannel->size());

    // Iterate through all vertices.
    for (auto vertex : dataset.grid->all(GridPrimitive::Vertex)) {
        glm::ivec3 sum(0), bufferSum(0), currentValue(0);
        ind numNeighbors = 0;

        // Iterate through all neighbors.
        // If another GridPrimitive is specified, we get all such primitives connected to the
        // current vertex.
        for (auto vertexNeighbor : vertex.connection(GridPrimitive::Vertex)) {
            // Because we have a buffer, we can directly access the data.
            // Any type of the correct size can be used.
            bufferSum += vertexBuffer->get<glm::ivec3>(vertexNeighbor);

            // Possible for every DataChannel: provide memory that is written to.
            vertexChannel->fill(currentValue, vertexNeighbor);
            sum += currentValue;

            numNeighbors++;
        }

        // Assign to the result vector.
        for (ind dim = 0; dim < 3; ++dim)
            filteredRandom[vertex * 3 + dim] = (float)sum[dim] / numNeighbors;
    }

    /*********************************************************************************
     * Grid does not have positions yet.
     * Add cylindric coordinates.
     *********************************************************************************/
    std::vector<ind> size;
    grid->getNumCells(size);

    // Our cylindric coordinates.
    auto posFunc = [&size](glm::vec3& pos, ind idx) {
        // Transform linear index to 3 index.
        ind x = idx % size[0];
        ind y = (idx / size[0]) % size[1];
        ind z = idx / (size[0] * size[1]);

        // Cylinder coordinates.
        double angle = M_PI * 2 * x / size[0];
        float radius = 5.0f + y;
        pos.x = (float)sin(angle) * radius;
        pos.y = (float)cos(angle) * radius;
        pos.z = (float)z;
    };

    // Add as AnalyticChannel.
    auto vertices = std::make_shared<AnalyticChannel<float, 3, glm::vec3>>(
        posFunc, grid->getNumElements(GridPrimitive::Vertex), "Position", GridPrimitive::Vertex);
    dataset.addChannel(vertices);

    /*********************************************************************************
     * Random algorithm: divide by volume.
     * Note: So far, only volume is available, length and area etc would be nice too.
     *********************************************************************************/
    std::vector<double> normalizedSinCos(cellChannel->size() * cellChannel->getNumComponents());

    // Iterate through all 3D cells, thus, volume.
    for (auto cell : dataset.grid->all(GridPrimitive::Volume)) {

        // Get cell volume.
        double volume = euclidean::getMeasure(*vertices, cell);

        // Get data from our [sin, cos] channel.
        glm::vec2 cellData;
        cellChannel->fill(cellData, cell);

        for (ind dim = 0; dim < 2; ++dim) normalizedSinCos[cell * 2 + dim] = cellData[dim] / volume;
    }

    /*********************************************************************************
     * Random algorithm: divide averaged random data by volume.
     * "Distribute" the cell volumes uniformly across connected vertices.
     *********************************************************************************/
    for (auto vertex : dataset.grid->all(GridPrimitive::Vertex)) {
        double totalVolume = 0;

        // Iterate through all volumes containing this vertex.
        for (auto adjacentCell : vertex.connection(GridPrimitive::Volume)) {

            // Get all neighboring vertices. Save their number.
            // We could again iterate through those now, if needed.
            ind numNeighbors = adjacentCell.connection(GridPrimitive::Vertex).size();

            double volume = euclidean::getMeasure(*vertices, adjacentCell);
            totalVolume += volume / numNeighbors;
        }

        EXPECT_TRUE(totalVolume > 0) << "Some volume was 0 or negative";

        // Divide each value
        for (ind dim = 0; dim < 3; ++dim) filteredRandom[vertex * 3 + dim] /= (float)totalVolume;
    }

    // Ad to DataSet for fun.
    auto avgBuffer = std::make_shared<BufferChannel<float, 3>>(filteredRandom, "FilteredRandom",
                                                               GridPrimitive::Vertex);
    dataset.addChannel(avgBuffer);
}

}  // namespace discretedata
}  // namespace inviwo
