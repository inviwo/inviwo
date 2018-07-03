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

typedef glm::vec3 Vec3f;
TEST(DataSet, ChannelInsertRemoveEdit) {

    // ************************************************* \\
    // Testing Handling of Data Sets                     \\
    // ************************************************* \\
    // - Create several channels
    // - Add and remove them
    // - Rename them
    DataSet set;

    auto monomeVert = std::make_shared<AnalyticChannel<Vec3f, float, 3>>(
        [](Vec3f& a, ind idx) {
            a[0] = 0.0f;
            a[1] = (float)idx;
            a[2] = (float)(idx * idx);
        },
        100, "Monome", GridPrimitive::Vertex);
    auto monomeFace = std::make_shared<AnalyticChannel<Vec3f, float, 3>>(
        [](Vec3f& a, ind idx) {
            a[0] = 0.0f;
            a[1] = (float)idx;
            a[2] = (float)(idx * idx);
        },
        100, "Monome", GridPrimitive::Face);
    auto identityVert = std::make_shared<AnalyticChannel<Vec3f, float, 3>>(
        [](Vec3f& a, ind idx) {
            a[0] = (float)idx;
            a[1] = (float)idx;
            a[2] = (float)idx;
        },
        100, "Identity", GridPrimitive::Vertex);

    set.Channels.addChannel(monomeVert);
    set.Channels.addChannel(monomeFace);
    set.Channels.addChannel(identityVert);

    // auto mV_comp = set.getChannel("Monome", GridPrimitive::Vertex);
    // EXPECT_EQ(mV_comp.get(), mV.get());
    // TODO:
    // - Convert to buffer
    // - Assign to other GridPrimitives
    // - Check size with connectivity
}

}  // namespace
}
