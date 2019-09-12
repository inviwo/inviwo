/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/network/networklock.h>

#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/interaction/events/resizeevent.h>

#include <functional>

namespace inviwo {

namespace {

struct TestProcessor : Processor {
    TestProcessor(const std::string& id) : Processor(id, id), inport("inport"), outport("outport") {
        addPort(inport);
        addPort(outport);
    }

    virtual const ProcessorInfo getProcessorInfo() const override { return processorInfo_; }

    static const ProcessorInfo processorInfo_;

    virtual void initializeResources() override {}
    virtual void process() override {}
    virtual void doIfNotReady() override {}

    ImageInport inport;
    ImageOutport outport;
};

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TestProcessor::processorInfo_{
    "org.inviwo.ResizeEvent.TestProcessor",  // Class identifier
    "TestProcessor",                         // Display name
    "Testing",                               // Category
    CodeState::Stable,                       // Code state
    Tags::CPU,                               // Tags
};

}  // namespace

TEST(ResizeEvent, basic) {
    ProcessorNetwork network{InviwoApplication::getPtr()};
    ProcessorNetworkEvaluator evaluator{&network};

    /*
     *     ┌───────────┐
     *     │    P0     │
     *     └─────┬─────┘
     *     ┌─────┴─────┐
     *     │    P1     │
     *     └─────┬─────┘
     *     ┌─────┴─────┐
     *     │    P2     │
     *     └───────────┘
     */

    auto proc0 = std::make_unique<TestProcessor>("p0");
    auto proc1 = std::make_unique<TestProcessor>("p1");
    auto proc2 = std::make_unique<TestProcessor>("p2");

    auto& p0 = *proc0;
    auto& p1 = *proc1;
    auto& p2 = *proc2;

    network.addProcessor(std::move(proc0));
    network.addProcessor(std::move(proc1));
    network.addProcessor(std::move(proc2));

    network.addConnection(&p0.outport, &p1.inport);
    network.addConnection(&p1.outport, &p2.inport);

    {
        ResizeEvent event{size2_t{100}};
        p2.outport.propagateEvent(&event, nullptr);
        EXPECT_EQ(p1.outport.getDimensions(), size2_t{100});
        EXPECT_EQ(p0.outport.getDimensions(), size2_t{100});
    }

    {
        ResizeEvent event{size2_t{200}};
        p2.outport.propagateEvent(&event, nullptr);
        EXPECT_EQ(p1.outport.getDimensions(), size2_t{200});
        EXPECT_EQ(p0.outport.getDimensions(), size2_t{200});
    }
}

TEST(ResizeEvent, twoSinks) {
    ProcessorNetwork network{InviwoApplication::getPtr()};
    ProcessorNetworkEvaluator evaluator{&network};

    /*
     *             ┌───────────┐
     *             │    P0     │
     *             └─────┬─────┘
     *             ┌─────┴─────┐
     *             │    P1     │
     *             └─────┬─────┘
     *           ┌───────┴───────┐
     *     ┌─────┴─────┐   ┌─────┴─────┐
     *     │    P2a    │   │    P2a    │
     *     └───────────┘   └───────────┘
     */

    auto proc0 = std::make_unique<TestProcessor>("p0");
    auto proc1 = std::make_unique<TestProcessor>("p1");
    auto proc2a = std::make_unique<TestProcessor>("p2a");
    auto proc2b = std::make_unique<TestProcessor>("p2b");

    auto& p0 = *proc0;
    auto& p1 = *proc1;
    auto& p2a = *proc2a;
    auto& p2b = *proc2b;

    network.addProcessor(std::move(proc0));
    network.addProcessor(std::move(proc1));
    network.addProcessor(std::move(proc2a));
    network.addProcessor(std::move(proc2b));

    network.addConnection(&p0.outport, &p1.inport);
    network.addConnection(&p1.outport, &p2a.inport);
    network.addConnection(&p1.outport, &p2b.inport);

    {
        ResizeEvent eventA{size2_t{100}};
        p2a.outport.propagateEvent(&eventA, nullptr);

        ResizeEvent eventB{size2_t{50}};
        p2b.outport.propagateEvent(&eventB, nullptr);

        EXPECT_EQ(p1.outport.getDimensions(), size2_t{100});
        EXPECT_EQ(p0.outport.getDimensions(), size2_t{100});
    }

    {
        ResizeEvent eventB{size2_t{150}};
        p2b.outport.propagateEvent(&eventB, nullptr);

        EXPECT_EQ(p1.outport.getDimensions(), size2_t{150});
        EXPECT_EQ(p0.outport.getDimensions(), size2_t{150});
    }

    {
        ResizeEvent eventB{size2_t{50}};
        p2b.outport.propagateEvent(&eventB, nullptr);

        EXPECT_EQ(p1.outport.getDimensions(), size2_t{100});
        EXPECT_EQ(p0.outport.getDimensions(), size2_t{100});
    }
}

TEST(ResizeEvent, dontHandleResizeEvents) {
    ProcessorNetwork network{InviwoApplication::getPtr()};
    ProcessorNetworkEvaluator evaluator{&network};

    /*
     *             ┌───────────┐
     *             │    P0     │
     *             └─────┬─────┘
     *             ┌─────┴─────┐
     *             │    P1     │
     *             └─────┬─────┘
     *           ┌───────┴───────┐
     *     ┌─────┴─────┐   ┌─────┴─────┐
     *     │    P2a    │   │    P2a    │
     *     └───────────┘   └───────────┘
     */

    auto proc0 = std::make_unique<TestProcessor>("p0");
    auto proc1 = std::make_unique<TestProcessor>("p1");
    auto proc2a = std::make_unique<TestProcessor>("p2a");
    auto proc2b = std::make_unique<TestProcessor>("p2b");

    auto& p0 = *proc0;
    auto& p1 = *proc1;
    auto& p2a = *proc2a;
    auto& p2b = *proc2b;

    network.addProcessor(std::move(proc0));
    network.addProcessor(std::move(proc1));
    network.addProcessor(std::move(proc2a));
    network.addProcessor(std::move(proc2b));

    network.addConnection(&p0.outport, &p1.inport);
    network.addConnection(&p1.outport, &p2a.inport);
    network.addConnection(&p1.outport, &p2b.inport);

    p1.outport.setHandleResizeEvents(false);

    {
        ResizeEvent eventA{size2_t{100}};
        p2a.outport.propagateEvent(&eventA, nullptr);

        ResizeEvent eventB{size2_t{50}};
        p2b.outport.propagateEvent(&eventB, nullptr);

        EXPECT_EQ(p1.outport.getDimensions(), size2_t{1}) << "Should not be resized";
        EXPECT_EQ(p0.outport.getDimensions(), size2_t{100});
    }

    {
        ResizeEvent eventB{size2_t{150}};
        p2b.outport.propagateEvent(&eventB, nullptr);

        EXPECT_EQ(p1.outport.getDimensions(), size2_t{1});
        EXPECT_EQ(p0.outport.getDimensions(), size2_t{150});
    }

    {
        ResizeEvent eventB{size2_t{50}};
        p2b.outport.propagateEvent(&eventB, nullptr);

        EXPECT_EQ(p1.outport.getDimensions(), size2_t{1});
        EXPECT_EQ(p0.outport.getDimensions(), size2_t{100});
    }
}

TEST(ResizeEvent, twoSinksDisconnect) {
    ProcessorNetwork network{InviwoApplication::getPtr()};
    ProcessorNetworkEvaluator evaluator{&network};

    /*
     *             ┌───────────┐
     *             │    P0     │
     *             └─────┬─────┘
     *             ┌─────┴─────┐
     *             │    P1     │
     *             └─────┬─────┘
     *           ┌───────┴───────┐
     *     ┌─────┴─────┐   ┌─────┴─────┐
     *     │    P2a    │   │    P2a    │
     *     └───────────┘   └───────────┘
     */

    auto proc0 = std::make_unique<TestProcessor>("p0");
    auto proc1 = std::make_unique<TestProcessor>("p1");
    auto proc2a = std::make_unique<TestProcessor>("p2a");
    auto proc2b = std::make_unique<TestProcessor>("p2b");

    auto& p0 = *proc0;
    auto& p1 = *proc1;
    auto& p2a = *proc2a;
    auto& p2b = *proc2b;

    network.addProcessor(std::move(proc0));
    network.addProcessor(std::move(proc1));
    network.addProcessor(std::move(proc2a));
    network.addProcessor(std::move(proc2b));

    network.addConnection(&p0.outport, &p1.inport);
    network.addConnection(&p1.outport, &p2a.inport);
    network.addConnection(&p1.outport, &p2b.inport);

    {
        ResizeEvent eventA{size2_t{100}};
        p2a.outport.propagateEvent(&eventA, nullptr);

        ResizeEvent eventB{size2_t{50}};
        p2b.outport.propagateEvent(&eventB, nullptr);

        EXPECT_EQ(p1.outport.getDimensions(), size2_t{100});
        EXPECT_EQ(p0.outport.getDimensions(), size2_t{100});
    }

    network.removeConnection(&p1.outport, &p2a.inport);

    {
        EXPECT_EQ(p1.outport.getDimensions(), size2_t{50});
        EXPECT_EQ(p0.outport.getDimensions(), size2_t{50});
    }

    network.addConnection(&p1.outport, &p2a.inport);

    {
        ResizeEvent eventA{size2_t{100}};
        p2a.outport.propagateEvent(&eventA, nullptr);
        EXPECT_EQ(p1.outport.getDimensions(), size2_t{100});
        EXPECT_EQ(p0.outport.getDimensions(), size2_t{100});
    }
}

}  // namespace inviwo
