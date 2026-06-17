/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2026 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/network/networklock.h>

#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>

#include <functional>

namespace inviwo {

namespace {

class TestProcessor;

struct OnCallbacks {
    std::function<void(TestProcessor&)> onInit;
    std::function<void(TestProcessor&)> onProcess;
    std::function<void(TestProcessor&)> onNotReady;
};

struct TestProcessor : Processor {
    TestProcessor(std::string_view id, OnCallbacks callbacks = {})
        : Processor(id, id), inport{"in"}, outport{"out"}, callbacks{std::move(callbacks)} {}

    virtual const ProcessorInfo& getProcessorInfo() const override { return processorInfo_; }

    static const ProcessorInfo processorInfo_;

    virtual void initializeResources() override {
        if (callbacks.onInit) callbacks.onInit(*this);
    }
    virtual void process() override {
        if (callbacks.onProcess) callbacks.onProcess(*this);
    }
    virtual void doIfNotReady() override {
        if (callbacks.onNotReady) callbacks.onNotReady(*this);
    }

    DataInport<int> inport;
    DataOutport<int> outport;

    OnCallbacks callbacks;
};

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TestProcessor::processorInfo_{
    "org.inviwo.TestProcessor",  // Class identifier
    "TestProcessor",             // Display name
    "Testing",                   // Category
    CodeState::Stable,           // Code state
    Tags::CPU,                   // Tags
};

struct Instrument {
    struct Calls {
        int init = 0;
        int process = 0;
        int notReady = 0;
    };

    Instrument(TestProcessor& p) : name{p.getIdentifier()}, calls{} {

        p.callbacks.onInit = [this, f = p.callbacks.onInit](TestProcessor& p) {
            ++calls.init;
            if (f) f(p);
        };
        p.callbacks.onProcess = [this, f = p.callbacks.onProcess](TestProcessor& p) {
            ++calls.process;
            if (f) f(p);
        };
        p.callbacks.onNotReady = [this, f = p.callbacks.onNotReady](TestProcessor& p) {
            ++calls.notReady;
            if (f) f(p);
        };
    }

    void check(Calls expected) {
        SCOPED_TRACE(name);
        EXPECT_EQ(calls.init, expected.init);
        EXPECT_EQ(calls.process, expected.process);
        EXPECT_EQ(calls.notReady, expected.notReady);
    }
    void reset() {
        calls.init = 0;
        calls.process = 0;
        calls.notReady = 0;
    }
    void checkAndReset(Calls expected) {
        check(expected);
        reset();
    }

    std::string name = "";
    Calls calls;
};

}  // namespace

auto createSource(std::string_view id, OnCallbacks callbacks = {}) {
    auto source = std::make_unique<TestProcessor>(id, std::move(callbacks));
    source->addPort(source->outport);
    return source;
};

auto createSink(std::string_view id, OnCallbacks callbacks = {}) {
    auto sink = std::make_unique<TestProcessor>(id, std::move(callbacks));
    sink->addPort(sink->inport);
    return sink;
};

TEST(NetworkEvaluator, Eval) {
    ProcessorNetwork network{InviwoApplication::getPtr()};
    ProcessorNetworkEvaluator evaluator{&network};

    auto source = createSource("source", {.onProcess = [](TestProcessor& p) {
                                   p.outport.setData(std::make_shared<int>(0));
                               }});
    auto src = source.get();

    Instrument srcInst(*src);

    {
        SCOPED_TRACE("Add source");
        network.addProcessor(std::move(source));
        srcInst.checkAndReset({});
    }

    auto sink = createSink("sink");
    auto snk = sink.get();
    Instrument snkInst(*snk);

    {
        SCOPED_TRACE("Add sink");
        network.addProcessor(std::move(sink));
        snkInst.checkAndReset({.notReady = 1});
    }

    {
        SCOPED_TRACE("Add connection");
        network.addConnection(src->getOutports()[0], snk->getInports()[0]);
        srcInst.checkAndReset({.init = 1, .process = 1});
        snkInst.checkAndReset({.init = 1, .process = 1});
    }

    {
        SCOPED_TRACE("Invalid valid");
        src->invalidate(InvalidationLevel::Valid);
        srcInst.checkAndReset({});
        snkInst.checkAndReset({});
    }
    {
        SCOPED_TRACE("Invalid output");
        src->invalidate(InvalidationLevel::InvalidOutput);
        srcInst.checkAndReset({.process = 1});
        snkInst.checkAndReset({.process = 1});
    }
    {
        SCOPED_TRACE("Invalid resources");
        src->invalidate(InvalidationLevel::InvalidResources);
        srcInst.checkAndReset({.init = 1, .process = 1});
        snkInst.checkAndReset({.process = 1});
    }
    {
        SCOPED_TRACE("Locked network");
        NetworkLock lock(&network);
        src->invalidate(InvalidationLevel::InvalidOutput);
        srcInst.checkAndReset({});
        snkInst.checkAndReset({});
        EXPECT_FALSE(src->isValid());
        EXPECT_FALSE(snk->isValid());
    }
    {
        SCOPED_TRACE("Unlocked network");
        srcInst.checkAndReset({.process = 1});
        snkInst.checkAndReset({.process = 1});
        EXPECT_TRUE(src->isValid());
        EXPECT_TRUE(snk->isValid());
    }
}

TEST(NetworkEvaluator, Error) {
    ProcessorNetwork network{InviwoApplication::getPtr()};
    ProcessorNetworkEvaluator evaluator{&network};

    bool shouldThrow = false;
    auto source = createSource("source", {.onProcess = [&shouldThrow](TestProcessor& p) {
                                   p.outport.setData(std::make_shared<int>(0));
                                   if (shouldThrow) {
                                       throw Exception(SourceContext{}, "Error");
                                   }
                               }});
    auto src = source.get();

    Instrument srcInst(*src);

    {
        SCOPED_TRACE("Add source");
        network.addProcessor(std::move(source));
        srcInst.checkAndReset({});
    }

    auto sink = createSink("sink");
    auto snk = sink.get();
    Instrument snkInst(*snk);

    {
        SCOPED_TRACE("Add sink");
        network.addProcessor(std::move(sink));
        snkInst.checkAndReset({.notReady = 1});
    }

    {
        SCOPED_TRACE("Add connection");
        network.addConnection(src->getOutports()[0], snk->getInports()[0]);
        srcInst.checkAndReset({.init = 1, .process = 1});
        snkInst.checkAndReset({.init = 1, .process = 1});
    }

    {
        SCOPED_TRACE("Invalid output");
        src->invalidate(InvalidationLevel::InvalidOutput);
        srcInst.checkAndReset({.process = 1});
        snkInst.checkAndReset({.process = 1});
    }

    {
        SCOPED_TRACE("Invalid output with throw");
        unsigned int throwCount = 0;
        evaluator.setExceptionHandler(
            [&throwCount](Processor*, EvaluationType, SourceContext) { ++throwCount; });

        shouldThrow = true;
        src->invalidate(InvalidationLevel::InvalidOutput);
        EXPECT_EQ(throwCount, 1);
        srcInst.checkAndReset({.process = 1});
        snkInst.checkAndReset({.notReady = 1});
    }
}

}  // namespace inviwo
