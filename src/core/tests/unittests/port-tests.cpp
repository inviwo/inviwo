/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <gmock/gmock.h>
#include <warn/pop>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/processornetworkevaluator.h>

#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/util/zip.h>

#include <inviwo/testutil/zipmatcher.h>

namespace inviwo {

namespace {

struct OutportTestProcessor : Processor {
    OutportTestProcessor(const std::string& id, int val)
        : Processor(id, id), outport{"outport"}, val{val} {
        addPort(outport);
    }

    virtual const ProcessorInfo getProcessorInfo() const override { return processorInfo_; }

    static const ProcessorInfo processorInfo_;

    virtual void process() override { outport.setData(std::make_shared<int>(val)); }

    DataOutport<int> outport;
    int val;
};

const ProcessorInfo OutportTestProcessor::processorInfo_{
    "org.inviwo.OutportTestProcessor",  // Class identifier
    "OutportTestProcessor",             // Display name
    "Testing",                          // Category
    CodeState::Stable,                  // Code state
    Tags::CPU,                          // Tags
};

struct VectorOutportTestProcessor : Processor {
    VectorOutportTestProcessor(const std::string& id, std::vector<int> val)
        : Processor(id, id), outport{"outport"}, val{val} {
        addPort(outport);
    }

    virtual const ProcessorInfo getProcessorInfo() const override { return processorInfo_; }

    static const ProcessorInfo processorInfo_;

    virtual void process() override { outport.setData(std::make_shared<std::vector<int>>(val)); }

    DataOutport<std::vector<int>> outport;
    std::vector<int> val;
};

const ProcessorInfo VectorOutportTestProcessor::processorInfo_{
    "org.inviwo.VectorOutportTestProcessor",  // Class identifier
    "VectorOutportTestProcessor",             // Display name
    "Testing",                                // Category
    CodeState::Stable,                        // Code state
    Tags::CPU,                                // Tags
};

#include <warn/push>
#include <warn/ignore/extra-semi>
#include <warn/ignore/gnu-zero-variadic-macro-arguments>

struct InportTestProcessor : Processor {
    InportTestProcessor(const std::string& id) : Processor(id, id), inport{"inport"} {
        addPort(inport);
    }
    virtual const ProcessorInfo getProcessorInfo() const override { return processorInfo_; }
    static const ProcessorInfo processorInfo_;

    MOCK_METHOD(void, process, (), (override));
    DataInport<int> inport;
};

const ProcessorInfo InportTestProcessor::processorInfo_{
    "org.inviwo.InportTestProcessor",  // Class identifier
    "InportTestProcessor",             // Display name
    "Testing",                         // Category
    CodeState::Stable,                 // Code state
    Tags::CPU,                         // Tags
};

struct MultiInportTestProcessor : Processor {
    MultiInportTestProcessor(const std::string& id) : Processor(id, id), inport{"inport"} {
        addPort(inport);
    }
    virtual const ProcessorInfo getProcessorInfo() const override { return processorInfo_; }
    static const ProcessorInfo processorInfo_;
    MOCK_METHOD(void, process, (), (override));
    DataInport<int, 0> inport;
};

const ProcessorInfo MultiInportTestProcessor::processorInfo_{
    "org.inviwo.MultiInportTestProcessor",  // Class identifier
    "MultiInportTestProcessor",             // Display name
    "Testing",                              // Category
    CodeState::Stable,                      // Code state
    Tags::CPU,                              // Tags
};

struct FlatMultiInportTestProcessor : Processor {
    FlatMultiInportTestProcessor(const std::string& id) : Processor(id, id), inport{"inport"} {
        addPort(inport);
    }
    virtual const ProcessorInfo getProcessorInfo() const override { return processorInfo_; }
    static const ProcessorInfo processorInfo_;
    MOCK_METHOD(void, process, (), (override));
    DataInport<int, 0, true> inport;
};

const ProcessorInfo FlatMultiInportTestProcessor::processorInfo_{
    "org.inviwo.FlatMultiInportTestProcessor",  // Class identifier
    "FlatMultiInportTestProcessor",             // Display name
    "Testing",                                  // Category
    CodeState::Stable,                          // Code state
    Tags::CPU,                                  // Tags
};

#include <warn/pop>

}  // namespace

using ::testing::ElementsAre;
using ::testing::Pointee;
using ::testing::Pair;
using ::testing::Eq;
using ::testing::Matches;

TEST(PortTests, SingleInportOutport) {
    ProcessorNetwork network{InviwoApplication::getPtr()};
    ProcessorNetworkEvaluator evaluator{&network};

    auto& source1 = *static_cast<OutportTestProcessor*>(
        network.addProcessor(std::make_unique<OutportTestProcessor>("source1", 1)));
    auto& sink = *static_cast<InportTestProcessor*>(
        network.addProcessor(std::make_unique<InportTestProcessor>("sink")));

    EXPECT_CALL(sink, process).WillOnce([&]() {
        EXPECT_TRUE(sink.inport.isConnected());
        EXPECT_TRUE(sink.inport.isConnectedTo(&source1.outport));
        EXPECT_EQ(sink.inport.getMaxNumberOfConnections(), 1);
        EXPECT_EQ(sink.inport.getNumberOfConnections(), 1);
        EXPECT_EQ(sink.inport.getConnectedOutport(), &source1.outport);
        EXPECT_THAT(sink.inport.getConnectedOutports(), ElementsAre(&source1.outport));
        EXPECT_FALSE(sink.inport.isOptional());

        EXPECT_TRUE(sink.inport.hasData());
        EXPECT_TRUE(sink.inport.isReady());
        EXPECT_TRUE(sink.inport.isChanged());
        EXPECT_THAT(sink.inport.getChangedOutports(), ElementsAre(&source1.outport));

        EXPECT_THAT(sink.inport.getData(), Pointee(1));
        EXPECT_THAT(sink.inport, ElementsAre(Pointee(1)));
        EXPECT_THAT(sink.inport.getVectorData(), ElementsAre(Pointee(1)));
        EXPECT_THAT(sink.inport.getSourceVectorData(),
                    ElementsAre(Pair(&source1.outport, Pointee(1))));
        EXPECT_THAT(sink.inport.outportAndData(), ElementsAre(Pair(&source1.outport, Pointee(1))));
        EXPECT_THAT(sink.inport.changedAndData(), ElementsAre(Pair(Eq(true), Pointee(1))));
    });

    network.addConnection(&source1.outport, &sink.inport);

    EXPECT_TRUE(sink.inport.hasData());
    EXPECT_TRUE(sink.inport.isReady());
    EXPECT_FALSE(sink.inport.isChanged());
}

TEST(PortTests, MultiInportOutport) {
    ProcessorNetwork network{InviwoApplication::getPtr()};
    ProcessorNetworkEvaluator evaluator{&network};

    auto& source1 = *static_cast<OutportTestProcessor*>(
        network.addProcessor(std::make_unique<OutportTestProcessor>("source1", 1)));

    auto& source2 = *static_cast<OutportTestProcessor*>(
        network.addProcessor(std::make_unique<OutportTestProcessor>("source2", 2)));

    auto& sink = *static_cast<MultiInportTestProcessor*>(
        network.addProcessor(std::make_unique<MultiInportTestProcessor>("sink")));

    EXPECT_CALL(sink, process)
        .WillOnce([&]() {
            EXPECT_TRUE(sink.inport.isConnected());
            EXPECT_TRUE(sink.inport.isConnectedTo(&source1.outport));
            EXPECT_EQ(sink.inport.getMaxNumberOfConnections(), std::numeric_limits<size_t>::max());
            EXPECT_EQ(sink.inport.getNumberOfConnections(), 1);
            EXPECT_EQ(sink.inport.getConnectedOutport(), &source1.outport);
            EXPECT_THAT(sink.inport.getConnectedOutports(), ElementsAre(&source1.outport));
            EXPECT_FALSE(sink.inport.isOptional());

            EXPECT_TRUE(sink.inport.hasData());
            EXPECT_TRUE(sink.inport.isReady());
            EXPECT_TRUE(sink.inport.isChanged());
            EXPECT_THAT(sink.inport.getChangedOutports(), ElementsAre(&source1.outport));

            EXPECT_THAT(sink.inport.getData(), Pointee(1));
            EXPECT_THAT(sink.inport, ElementsAre(Pointee(1)));
            EXPECT_THAT(sink.inport.getVectorData(), ElementsAre(Pointee(1)));
            EXPECT_THAT(sink.inport.getSourceVectorData(),
                        ElementsAre(Pair(&source1.outport, Pointee(1))));
            EXPECT_THAT(sink.inport.outportAndData(),
                        ElementsAre(Pair(&source1.outport, Pointee(1))));
            EXPECT_THAT(sink.inport.changedAndData(), ElementsAre(Pair(Eq(true), Pointee(1))));
        })
        .WillOnce([&]() {
            EXPECT_TRUE(sink.inport.isConnected());
            EXPECT_TRUE(sink.inport.isConnectedTo(&source1.outport));
            EXPECT_TRUE(sink.inport.isConnectedTo(&source2.outport));
            EXPECT_EQ(sink.inport.getMaxNumberOfConnections(), std::numeric_limits<size_t>::max());
            EXPECT_EQ(sink.inport.getNumberOfConnections(), 2);
            EXPECT_EQ(sink.inport.getConnectedOutport(), &source1.outport);
            EXPECT_THAT(sink.inport.getConnectedOutports(),
                        ElementsAre(&source1.outport, &source2.outport));
            EXPECT_FALSE(sink.inport.isOptional());

            EXPECT_TRUE(sink.inport.hasData());
            EXPECT_TRUE(sink.inport.isReady());
            EXPECT_TRUE(sink.inport.isChanged());
            EXPECT_THAT(sink.inport.getChangedOutports(), ElementsAre(&source2.outport));

            EXPECT_THAT(sink.inport.getData(), Pointee(1));
            EXPECT_THAT(sink.inport, ElementsAre(Pointee(1), Pointee(2)));
            EXPECT_THAT(sink.inport.getVectorData(), ElementsAre(Pointee(1), Pointee(2)));
            EXPECT_THAT(sink.inport.getSourceVectorData(),
                        ElementsAre(Pair(&source1.outport, Pointee(1)),
                                    Pair(&source2.outport, Pointee(2))));
            EXPECT_THAT(sink.inport.outportAndData(),
                        ElementsAre(Pair(&source1.outport, Pointee(1)),
                                    Pair(&source2.outport, Pointee(2))));
            EXPECT_THAT(sink.inport.changedAndData(),
                        ElementsAre(Pair(Eq(false), Pointee(1)), Pair(Eq(true), Pointee(2))));

            EXPECT_THAT(util::enumerate(sink.inport),
                        ElementsAre(ZipPair(Eq(0), Pointee(1)), ZipPair(Eq(1), Pointee(2))));

            EXPECT_THAT(util::enumerate(sink.inport.changedAndData()),
                        ElementsAre(ZipPair(Eq(0), Pair(Eq(false), Pointee(1))),
                                    ZipPair(Eq(1), Pair(Eq(true), Pointee(2)))));
        });

    network.addConnection(&source1.outport, &sink.inport);
    network.addConnection(&source2.outport, &sink.inport);

    EXPECT_TRUE(sink.inport.hasData());
    EXPECT_TRUE(sink.inport.isReady());
    EXPECT_FALSE(sink.inport.isChanged());
}

TEST(PortTests, FlatMultiInportOutport) {
    ProcessorNetwork network{InviwoApplication::getPtr()};
    ProcessorNetworkEvaluator evaluator{&network};

    auto& source1 = *static_cast<OutportTestProcessor*>(
        network.addProcessor(std::make_unique<OutportTestProcessor>("source1", 1)));

    auto& source2 = *static_cast<VectorOutportTestProcessor*>(network.addProcessor(
        std::make_unique<VectorOutportTestProcessor>("source2", std::vector<int>{2, 3, 4})));

    auto& sink = *static_cast<FlatMultiInportTestProcessor*>(
        network.addProcessor(std::make_unique<FlatMultiInportTestProcessor>("sink")));

    EXPECT_CALL(sink, process)
        .WillOnce([&]() {
            EXPECT_TRUE(sink.inport.isConnected());
            EXPECT_TRUE(sink.inport.isConnectedTo(&source1.outport));
            EXPECT_EQ(sink.inport.getMaxNumberOfConnections(), std::numeric_limits<size_t>::max());
            EXPECT_EQ(sink.inport.getNumberOfConnections(), 1);
            EXPECT_EQ(sink.inport.getConnectedOutport(), &source1.outport);
            EXPECT_THAT(sink.inport.getConnectedOutports(), ElementsAre(&source1.outport));
            EXPECT_FALSE(sink.inport.isOptional());

            EXPECT_TRUE(sink.inport.hasData());
            EXPECT_TRUE(sink.inport.isReady());
            EXPECT_TRUE(sink.inport.isChanged());
            EXPECT_THAT(sink.inport.getChangedOutports(), ElementsAre(&source1.outport));

            EXPECT_THAT(sink.inport.getData(), Pointee(1));
            EXPECT_THAT(sink.inport, ElementsAre(Pointee(1)));
            EXPECT_THAT(sink.inport.getVectorData(), ElementsAre(Pointee(1)));
            EXPECT_THAT(sink.inport.getSourceVectorData(),
                        ElementsAre(Pair(&source1.outport, Pointee(1))));
            EXPECT_THAT(sink.inport.outportAndData(),
                        ElementsAre(Pair(&source1.outport, Pointee(1))));
            EXPECT_THAT(sink.inport.changedAndData(), ElementsAre(Pair(Eq(true), Pointee(1))));
        })
        .WillOnce([&]() {
            EXPECT_TRUE(sink.inport.isConnected());
            EXPECT_TRUE(sink.inport.isConnectedTo(&source1.outport));
            EXPECT_TRUE(sink.inport.isConnectedTo(&source2.outport));
            EXPECT_EQ(sink.inport.getMaxNumberOfConnections(), std::numeric_limits<size_t>::max());
            EXPECT_EQ(sink.inport.getNumberOfConnections(), 2);
            EXPECT_EQ(sink.inport.getConnectedOutport(), &source1.outport);
            EXPECT_THAT(sink.inport.getConnectedOutports(),
                        ElementsAre(&source1.outport, &source2.outport));
            EXPECT_FALSE(sink.inport.isOptional());

            EXPECT_TRUE(sink.inport.hasData());
            EXPECT_TRUE(sink.inport.isReady());
            EXPECT_TRUE(sink.inport.isChanged());
            EXPECT_THAT(sink.inport.getChangedOutports(), ElementsAre(&source2.outport));

            EXPECT_THAT(sink.inport.getData(), Pointee(1));
            EXPECT_THAT(sink.inport, ElementsAre(Pointee(1), Pointee(2), Pointee(3), Pointee(4)));
            EXPECT_THAT(sink.inport.getVectorData(),
                        ElementsAre(Pointee(1), Pointee(2), Pointee(3), Pointee(4)));
            EXPECT_THAT(
                sink.inport.getSourceVectorData(),
                ElementsAre(Pair(&source1.outport, Pointee(1)), Pair(&source2.outport, Pointee(2)),
                            Pair(&source2.outport, Pointee(3)),
                            Pair(&source2.outport, Pointee(4))));
            EXPECT_THAT(
                sink.inport.outportAndData(),
                ElementsAre(Pair(&source1.outport, Pointee(1)), Pair(&source2.outport, Pointee(2)),
                            Pair(&source2.outport, Pointee(3)),
                            Pair(&source2.outport, Pointee(4))));
            EXPECT_THAT(sink.inport.changedAndData(),
                        ElementsAre(Pair(Eq(false), Pointee(1)), Pair(Eq(true), Pointee(2)),
                                    Pair(Eq(true), Pointee(3)), Pair(Eq(true), Pointee(4))));

            EXPECT_THAT(util::enumerate(sink.inport),
                        ElementsAre(ZipPair(Eq(0), Pointee(1)), ZipPair(Eq(1), Pointee(2)),
                                    ZipPair(Eq(2), Pointee(3)), ZipPair(Eq(3), Pointee(4))));

            EXPECT_THAT(util::enumerate(sink.inport.changedAndData()),
                        ElementsAre(ZipPair(Eq(0), Pair(Eq(false), Pointee(1))),
                                    ZipPair(Eq(1), Pair(Eq(true), Pointee(2))),
                                    ZipPair(Eq(2), Pair(Eq(true), Pointee(3))),
                                    ZipPair(Eq(3), Pair(Eq(true), Pointee(4)))));
        });

    network.addConnection(&source1.outport, &sink.inport);
    network.addConnection(&source2.outport, &sink.inport);

    EXPECT_TRUE(sink.inport.hasData());
    EXPECT_TRUE(sink.inport.isReady());
    EXPECT_FALSE(sink.inport.isChanged());
}

}  // namespace inviwo
