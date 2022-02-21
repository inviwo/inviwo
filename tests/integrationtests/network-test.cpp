/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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

#include <inviwo/core/network/processornetwork.h>
#include <modules/base/processors/volumesource.h>
#include <modules/base/processors/cubeproxygeometryprocessor.h>
#include <modules/base/processors/volumeslice.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo {

class NetworkTest : public ::testing::Test {
public:
    NetworkTest() : network(InviwoApplication::getPtr()){};

protected:
    virtual void SetUp() {
        auto p1 = network.emplaceProcessor<VolumeSource>();
        p1->setIdentifier("volumeSource");

        auto p2 = network.emplaceProcessor<CubeProxyGeometry>();
        p2->setIdentifier("cubeProxyGeometry");

        auto p3 = network.emplaceProcessor<VolumeSlice>();
        p3->setIdentifier("volumeSlice");

        network.addConnection(p1->getOutport("data"), p2->getInport("volume"));
        network.addConnection(p1->getOutport("data"), p3->getInport("inputVolume"));
    }

    // virtual void TearDown() {}

    ProcessorNetwork network;
};

TEST_F(NetworkTest, Inititiate) {
    ASSERT_EQ(3, network.getProcessors().size());
    ASSERT_EQ(2, network.getConnections().size());
    ASSERT_EQ(0, network.getLinks().size());
}

TEST_F(NetworkTest, NetworkGetPropery) {
    std::string_view path = "volumeSource.filename";
    const Property* prop = network.getProperty(path);
    ASSERT_TRUE(prop != nullptr);
}

TEST_F(NetworkTest, NetworkGetSubPropery) {
    std::string_view path = "volumeSource.Information.dimensions";
    const Property* prop = network.getProperty(path);
    ASSERT_TRUE(prop != nullptr);
}

TEST_F(NetworkTest, NetworkGetProcessorByIdentifier) {
    const Processor* p = network.getProcessorByIdentifier("volumeSource");
    ASSERT_TRUE(p != nullptr);
}

TEST_F(NetworkTest, NetworkGetProcessorByType) {
    const std::vector<VolumeSlice*> ps = network.getProcessorsByType<VolumeSlice>();
    ASSERT_EQ(1, ps.size());
    ASSERT_TRUE(ps[0] != nullptr);
}

TEST_F(NetworkTest, ProcessorGetPropertyByIdentifier) {
    const Processor* p = network.getProcessorByIdentifier("volumeSource");
    ASSERT_TRUE(p != nullptr);

    const Property* prop = p->getPropertyByIdentifier("filename");
    ASSERT_TRUE(prop != nullptr);
}

TEST_F(NetworkTest, ProcessorGetPropertyByType) {
    const Processor* p = network.getProcessorByIdentifier("volumeSource");
    ASSERT_TRUE(p != nullptr);

    const std::vector<FloatVec3Property*> props = p->getPropertiesByType<FloatVec3Property>(true);
    ASSERT_EQ(5, props.size());

    ASSERT_TRUE(props[0] != nullptr);
}

TEST_F(NetworkTest, ProcessorGetProperty) {
    const Processor* p = network.getProcessorByIdentifier("volumeSource");
    ASSERT_TRUE(p != nullptr);

    std::string_view path = "Information.dimensions";
    const Property* prop = p->getPropertyByPath(path);
    ASSERT_TRUE(prop != nullptr);
}

}  // namespace inviwo
