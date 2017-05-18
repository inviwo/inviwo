/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/util/logerrorcounter.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>

namespace inviwo {

class ProcessorCreationTests : public ::testing::TestWithParam<std::string> {
protected:
    ProcessorCreationTests() : p(nullptr) {}

    virtual ~ProcessorCreationTests() { EXPECT_TRUE(p == nullptr); }

    virtual void SetUp() {
        isAdded_ = false;
        logCounter_ = std::make_shared<LogErrorCounter>();
        LogCentral::getPtr()->registerLogger(logCounter_);
    }

    virtual void TearDown() {
        if (isAdded_) {
            ProcessorNetwork *pn = InviwoApplication::getPtr()->getProcessorNetwork();
            size_t sizeBefore = pn->getProcessors().size();

            pn->removeAndDeleteProcessor(p);

            size_t sizeAfter = pn->getProcessors().size();
            EXPECT_EQ(sizeBefore, sizeAfter + 1);
        } else if (p) {
            delete p;
        }
        p = nullptr;
    }

    void create() {
        logCounter_->reset();

        auto s = InviwoApplication::getPtr()->getProcessorFactory()->create(GetParam());
        ASSERT_TRUE(s.get() != nullptr);

        p = dynamic_cast<Processor *>(s.get());
        ASSERT_TRUE(p != nullptr);
        s.release();
        EXPECT_EQ(0, logCounter_->getWarnCount());
        EXPECT_EQ(0, logCounter_->getErrorCount());
    }

    void resetAllPoperties() {
        logCounter_->reset();

        p->resetAllPoperties();
        EXPECT_EQ(0, logCounter_->getWarnCount());
        EXPECT_EQ(0, logCounter_->getErrorCount());
    }

    void addProcessor() {
        logCounter_->reset();

        InviwoApplication::getPtr()->getProcessorNetwork()->addProcessor(p);
        EXPECT_EQ(0, logCounter_->getWarnCount());
        EXPECT_EQ(0, logCounter_->getErrorCount());
        isAdded_ = true;
    }

    Processor *p;
    bool isAdded_;
    std::shared_ptr<LogErrorCounter> logCounter_;
};

const std::vector<std::string> getListOfProcessors() {
    std::vector<std::string> theVec;
    for (const auto& module : InviwoApplication::getPtr()->getModules()) {
        for (const auto& processor: module->getProcessors()) {
            theVec.push_back(processor->getClassIdentifier());
        }
    }
    return theVec;
}

TEST_P(ProcessorCreationTests, ProcesorCreateAndResetAndAddToNetwork) {
 /*   create();
    resetAllPoperties();
    addProcessor();*/
}

INSTANTIATE_TEST_CASE_P(RegisteredProcessors, ProcessorCreationTests,
                        ::testing::ValuesIn(getListOfProcessors()));
}
