/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#include <ext/glm/glm/glm/vec3.hpp>

namespace inviwo {
namespace discretedata {

typedef BufferChannel<float, 3> BufferFloat;

// Most basic vector that should be correctly handled for indexing.
struct TestVec3f {
    float x, y, z;
};

TEST(CreatingCopyingIndexing, DataChannels) {

    // ************************************************* \\
    // Testing Indexing in Analytic/Buffer               \\
    // ************************************************* \\
    // - Create channels with (1, 0.1*idx, 0.01*idx^2)
    // - Compare results in analytic and buffer channel
    // - Copy, assign BufferChannel and test again

    ind numElements = 3;
    std::vector<ind> size(numElements);
    DataSet dataset(GridPrimitive::Volume, size);

    void (*base)(glm::vec3&, ind) = [](glm::vec3& dest, ind idx) {
        dest[0] = 1.0f;
        dest[1] = (float)idx;
        dest[2] = (float)idx * idx;
    };

    // Setting up a buffer to test against an analytic channel.
    std::vector<float> data;
    for (int dIdx = 0; dIdx < numElements; ++dIdx) {
        data.push_back(1);
        data.push_back((float)dIdx);
        data.push_back((float)dIdx * dIdx);
    }
    BufferChannel<float, 3>* testBuffer =
        new BufferFloat(data.data(), numElements, "MonomialBuffer", GridPrimitive::Vertex);
    IntMetaData* yearTest = testBuffer->createMetaData<IntMetaData>("YearCreated");
    yearTest->set(2017);

    // Set up the same channel as analytic channel.
    AnalyticChannel<float, 3, glm::vec3>* testAnalytic = new AnalyticChannel<float, 3, glm::vec3>(
        base, numElements, "MonomialAnalytical", GridPrimitive::Vertex);

    // Copy buffer.
    BufferFloat* copyBuffer = new BufferFloat(*testBuffer);
    copyBuffer->setName("MonomialBufferCopy");

    // Add to DataSet
    dataset.addChannel(testBuffer);
    dataset.addChannel(testAnalytic);
    dataset.addChannel(copyBuffer);

    //  Test access
    auto setBuffer = dataset.getChannel("MonomialBufferCopy");

    EXPECT_EQ(setBuffer->getMetaData<IntMetaData>("YearCreated")->get(), 2017);

    // Check equality of indexing and filling.

    ind c = 0;

    // Test indexing.
    for (TestVec3f& b_it : testBuffer->all<TestVec3f>()) {
        glm::vec3 b_fill, b_get;
        testBuffer->fill(b_fill, c);
        b_get = testBuffer->get<glm::vec3>(c);

        // All methods of indexing should return the same value.
        EXPECT_EQ(b_fill.y, c);
        EXPECT_EQ(b_fill.z, c * c);
        EXPECT_EQ(b_fill.y, b_get.y);
        EXPECT_EQ(b_fill.z, b_get.z);
        EXPECT_EQ(b_fill.y, b_it.y);
        EXPECT_EQ(b_fill.z, b_it.z);
        c++;
    }

    c = 0;
    // Test indexing and explicit iterator use.
    for (auto a_it = testAnalytic->begin<glm::vec3>(); a_it != testAnalytic->end<glm::vec3>();
         ++a_it) {
        glm::vec3 a_fill, b_get;
        testAnalytic->fill(a_fill, c);
        b_get = testBuffer->get<glm::vec3>(c);

        // All methods of indexing should return the same value.
        EXPECT_EQ(a_fill.y, (*a_it).y);
        EXPECT_EQ(a_fill.z, (*a_it).z);
        EXPECT_EQ(a_fill.y, b_get.y);
        EXPECT_EQ(a_fill.z, b_get.z);
        c++;
    }

    auto datasetChannel = dataset.getChannel<float, 3>("MonomialAnalytical", GridPrimitive::Vertex);
    c = 0;

    // Test indexing and explicit iterator use.
    for (glm::vec3 a_it : datasetChannel->all<glm::vec3>()) {
        glm::vec3 a_fill;
        datasetChannel->fill(a_fill, c);

        // All methods of indexing should return the same value.
        EXPECT_EQ(a_fill.x, a_it.x);
        EXPECT_EQ(a_fill.y, a_it.y);
        EXPECT_EQ(a_fill.z, a_it.z);
        c++;
    }
}

}  // namespace discretedata
}  // namespace inviwo
