/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/util/logerrorcounter.h>
#include <inviwo/core/util/stringlogger.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

namespace {

struct LogErrorCheck {
    LogErrorCheck(const std::string& procName)
        : logCounter_{std::make_shared<LogErrorCounter>()}
        , stringLog_{std::make_shared<StringLogger>()}
        , procName_{procName} {
        LogCentral::getPtr()->registerLogger(logCounter_);
        LogCentral::getPtr()->registerLogger(stringLog_);
    }
    ~LogErrorCheck() {
        EXPECT_EQ(0, logCounter_->getErrorCount())
            << "Processor " << procName_ << " produced errors: " << stringLog_->getLog();
    }

    std::shared_ptr<LogErrorCounter> logCounter_;
    std::shared_ptr<StringLogger> stringLog_;

    const std::string procName_;
};

}  // namespace

class ProcessorCreationTests : public ::testing::TestWithParam<std::string> {
protected:
    ProcessorCreationTests()
        : network_{InviwoApplication::getPtr()->getProcessorNetwork()}
        , factory_{InviwoApplication::getPtr()->getProcessorFactory()} {};

    virtual ~ProcessorCreationTests() = default;

    virtual void SetUp() override {}
    virtual void TearDown() override {}

    ProcessorNetwork* network_;
    ProcessorFactory* factory_;
};

TEST_P(ProcessorCreationTests, ProcesorCreateAndResetAndAddToNetwork) {
    LogErrorCheck checklog(GetParam());
    auto s = factory_->create(GetParam());
    ASSERT_TRUE(s.get() != nullptr) << "Could not create processor " << GetParam();
    s->resetAllPoperties();

    const size_t sizeBefore = network_->getProcessors().size();
    auto p = s.release();
    network_->addProcessor(p);
    EXPECT_EQ(sizeBefore + 1, network_->getProcessors().size())
        << "Could not add processor " << GetParam();
    network_->removeAndDeleteProcessor(p);
    EXPECT_EQ(sizeBefore, network_->getProcessors().size())
        << "Could not remove processor " << GetParam();
}

INSTANTIATE_TEST_SUITE_P(
    RegisteredProcessors, ProcessorCreationTests,
    ::testing::ValuesIn(InviwoApplication::getPtr()->getProcessorFactory()->getKeys()));
}  // namespace inviwo
