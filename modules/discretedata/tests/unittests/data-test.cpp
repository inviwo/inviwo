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
#include <iostream>

#include "discretedata/dataset.h"
#include "discretedata/channels/bufferchannel.h"
#include "discretedata/channels/analyticchannel.h"

namespace inviwo {
namespace dd {

typedef BufferChannel<float> BufferFloat;
TEST(CreatingCopyingIndexing, DataChannels) {
    // Testing tests.
    EXPECT_EQ(42, -~41);

    // ************************************************* \\
    // Testing Indexing in Analytic/Buffer               \\
    // ************************************************* \\
    // - Create channels with (1, 0.1*idx, 0.01*idx^2)
    // - Compare results in analytic and buffer channel
    // - Copy, assign BufferChannel and test again

    DataSet dataset;
    size_t numElements = 10;
    void (*base)(float*, size_t) = [](float* dest, size_t idx) {
        dest[0] = 1.0f;
        dest[1] = 0.1f * idx;
        dest[2] = dest[1] * dest[1];
    };

    // Setting up a buffer to test against an analytic channel.
    std::vector<float> data;
    for (int dIdx = 0; dIdx < numElements; ++dIdx) {
        data.push_back(1);
        data.push_back(0.1f * dIdx);
        data.push_back(0.01f * dIdx * dIdx);
    }
    BufferChannel<float>* testBuffer =
        new BufferFloat(data, 3, "MonomialBuffer", GridPrimitive::Vertex);
    IntMetaData* yearTest = testBuffer->createMetaData<IntMetaData>("YearCreated");
    yearTest->set(2017);

    // Set up the same channel as analytic channel.
    AnalyticChannelFloat* testAnalytic =
        new AnalyticChannelFloat(base, numElements, 3, "MonomialAnalytical", GridPrimitive::Vertex);

    // Copy buffer.
    BufferFloat* copyBuffer = new BufferFloat(*testBuffer);
    copyBuffer->setName("MonomialBufferCopy");

    // Add to DataSet
    dataset.Channels.addChannel(testBuffer);
    dataset.Channels.addChannel(testAnalytic);
    dataset.Channels.addChannel(copyBuffer);

    //  Test access
    auto setBuffer = dataset.Channels.getChannel("MonomialBufferCopy");

    EXPECT_EQ(setBuffer->getMetaData<IntMetaData>("YearCreated")->get(), 2017);

    // Check equality of indexing and filling.

    // Setup for fill function.
    float* bufferData = new float[3 * numElements];
    float* analyticData = new float[3 * numElements];

    // For one element: compare channels and fill data.
    {
        int dIdx = 3;

        testBuffer->fill(bufferData + 3 * dIdx, dIdx);

        testAnalytic->fill(analyticData + 3 * dIdx, dIdx);

        // Check equality in a dimension.
        {
            int dim = 1;

            EXPECT_EQ(testBuffer->get(dIdx)[dim], copyBuffer->get(dIdx)[dim]);
            EXPECT_EQ(testBuffer->get(dIdx)[dim], bufferData[dIdx * 3 + dim]);
            EXPECT_EQ(testBuffer->get(dIdx)[dim], analyticData[dIdx * 3 + dim]);
        }
    }

    // The other data should be deleted when the dataset deconstructs.
}

}  // namespace
}
