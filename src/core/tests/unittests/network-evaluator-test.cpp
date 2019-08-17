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

#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>

#include <functional>

namespace inviwo {

namespace {

struct TestProcessor : Processor {
    TestProcessor(const std::string& id) : Processor(id, id) {}

    virtual const ProcessorInfo getProcessorInfo() const override { return processorInfo_; }

    static const ProcessorInfo processorInfo_;

    virtual void initializeResources() override {
        if (onInitializeResources) onInitializeResources(*this);
    }
    virtual void process() override {
        if (onProcess) onProcess(*this);
    }
    virtual void doIfNotReady() override {
        if (onDoIfNotReady) onDoIfNotReady(*this);
    }

    std::function<void(TestProcessor&)> onInitializeResources;
    std::function<void(TestProcessor&)> onProcess;
    std::function<void(TestProcessor&)> onDoIfNotReady;
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
    Instrument(TestProcessor& p) {
        name = p.getIdentifier();
        p.onInitializeResources = [this](TestProcessor&) { ++initRes; };
        p.onProcess = [this](TestProcessor&) { ++process; };
        p.onDoIfNotReady = [this](TestProcessor&) { ++notReady; };
    }

    void check(int exprectedInitRes, int exprectedProcess, int exprectedNotReady) {
        SCOPED_TRACE(name);
        EXPECT_EQ(initRes, exprectedInitRes);
        EXPECT_EQ(process, exprectedProcess);
        EXPECT_EQ(notReady, exprectedNotReady);
    }
    void reset() {
        initRes = 0;
        process = 0;
        notReady = 0;
    }
    void checkAndReset(int exprectedInitRes, int exprectedProcess, int exprectedNotReady) {
        check(exprectedInitRes, exprectedProcess, exprectedNotReady);
        reset();
    }

    std::string name = "";
    int initRes = 0;
    int process = 0;
    int notReady = 0;
};

}  // namespace

const auto createA = []() {
    auto at = std::make_unique<TestProcessor>("a");
    at->addPort(std::make_unique<DataOutport<int>>("out"));
    return at;
};

const auto createB = []() {
    auto bt = std::make_unique<TestProcessor>("b");
    bt->addPort(std::make_unique<DataInport<int>>("in"));
    return bt;
};

TEST(NetworkEvaluator, Eval) {
    ProcessorNetwork network{InviwoApplication::getPtr()};
    ProcessorNetworkEvaluator evaluator{&network};

    auto at = createA();
    auto a = at.get();
    Instrument ai(*a);

    a->onProcess = [func = a->onProcess](TestProcessor& p) {
        func(p);
        static_cast<DataOutport<int>*>(p.getOutports()[0])->setData(std::make_shared<int>(0));
    };

    {
        SCOPED_TRACE("Add a");
        network.addProcessor(std::move(at));
        ai.checkAndReset(0, 0, 0);
    }

    auto bt = createB();
    auto b = bt.get();
    Instrument bi(*b);

    {
        SCOPED_TRACE("Add b");
        network.addProcessor(std::move(bt));
        bi.checkAndReset(0, 0, 1);
    }

    {
        SCOPED_TRACE("Add connection");
        network.addConnection(a->getOutports()[0], b->getInports()[0]);
        ai.checkAndReset(1, 1, 0);
        bi.checkAndReset(1, 1, 0);
    }

    {
        SCOPED_TRACE("Invalid valid");
        a->invalidate(InvalidationLevel::Valid);
        ai.checkAndReset(0, 0, 0);
        bi.checkAndReset(0, 0, 0);
    }
    {
        SCOPED_TRACE("Invalid output");
        a->invalidate(InvalidationLevel::InvalidOutput);
        ai.checkAndReset(0, 1, 0);
        bi.checkAndReset(0, 1, 0);
    }
    {
        SCOPED_TRACE("Invalid resources");
        a->invalidate(InvalidationLevel::InvalidResources);
        ai.checkAndReset(1, 1, 0);
        bi.checkAndReset(0, 1, 0);
    }
    {
        SCOPED_TRACE("Locked network");
        NetworkLock lock(&network);
        a->invalidate(InvalidationLevel::InvalidOutput);
        ai.checkAndReset(0, 0, 0);
        bi.checkAndReset(0, 0, 0);
        EXPECT_FALSE(a->isValid());
        EXPECT_FALSE(b->isValid());
    }
    {
        SCOPED_TRACE("Unlocked network");
        ai.checkAndReset(0, 1, 0);
        bi.checkAndReset(0, 1, 0);
        EXPECT_TRUE(a->isValid());
        EXPECT_TRUE(b->isValid());
    }
}

TEST(NetworkEvaluator, Error) {
    ProcessorNetwork network{InviwoApplication::getPtr()};
    ProcessorNetworkEvaluator evaluator{&network};

    auto at = createA();
    auto a = at.get();
    Instrument ai(*a);

    bool shouldThrow = false;
    a->onProcess = [func = a->onProcess, &shouldThrow](TestProcessor& p) {
        func(p);
        static_cast<DataOutport<int>*>(p.getOutports()[0])->setData(std::make_shared<int>(0));
        if (shouldThrow) {
            throw Exception("Error", IVW_CONTEXT_CUSTOM("TestProcessor"));
        }
    };

    {
        SCOPED_TRACE("Add a");
        network.addProcessor(std::move(at));
        ai.checkAndReset(0, 0, 0);
    }

    auto bt = createB();
    auto b = bt.get();
    Instrument bi(*b);

    {
        SCOPED_TRACE("Add b");
        network.addProcessor(std::move(bt));
        bi.checkAndReset(0, 0, 1);
    }

    {
        SCOPED_TRACE("Add connection");
        network.addConnection(a->getOutports()[0], b->getInports()[0]);
        ai.checkAndReset(1, 1, 0);
        bi.checkAndReset(1, 1, 0);
    }

    {
        SCOPED_TRACE("Invalid output");
        a->invalidate(InvalidationLevel::InvalidOutput);
        ai.checkAndReset(0, 1, 0);
        bi.checkAndReset(0, 1, 0);
    }

    {
        SCOPED_TRACE("Invalid output with throw");
        unsigned int throwCount = 0;
        evaluator.setExceptionHandler(
            [&throwCount](Processor*, EvaluationType, ExceptionContext) { ++throwCount; });

        shouldThrow = true;
        a->invalidate(InvalidationLevel::InvalidOutput);
        EXPECT_EQ(throwCount, 1);
        ai.checkAndReset(0, 1, 0);
        bi.checkAndReset(0, 1, 0);
    }
}

}  // namespace inviwo
