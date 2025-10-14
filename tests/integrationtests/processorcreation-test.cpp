/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/stringlogger.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglcapabilities.h>

namespace inviwo {

class ProcessorCreationTests : public ::testing::TestWithParam<std::string> {
protected:
    ProcessorCreationTests()
        : network_{InviwoApplication::getPtr()->getProcessorNetwork()}
        , factory_{InviwoApplication::getPtr()->getProcessorFactory()}
        , supportedGlVersion_{OpenGLCapabilities::getOpenGLVersion()} {};

    virtual ~ProcessorCreationTests() = default;

    virtual void SetUp() override {}
    virtual void TearDown() override {}

    ProcessorNetwork* network_;
    ProcessorFactory* factory_;
    int supportedGlVersion_;

    bool isSupported(const Tags& tags) const {
        for (const auto& tag : tags.tags_) {
            const auto str = tags.getString();
            // Look for "GLX.Y"
            if (str.size() >= 5 && str.starts_with("GL")) {
                if (str[2] >= '0' && str[2] <= '9' && str[3] == '.' && str[4] >= '0' &&
                    str[4] <= '9') {

                    const auto requiredGlVersion =
                        static_cast<int>(str[2] - '0') * 100 + static_cast<int>(str[4] - '0') * 10;

                    if (requiredGlVersion > supportedGlVersion_) return false;
                }
            }
        }
        return true;
    }
};

TEST_P(ProcessorCreationTests, ProcesorCreateAndResetAndAddToNetwork) {
    RenderContext::getPtr()->activateDefaultRenderContext();
    LGL_ERROR;

    auto logCounter = std::make_shared<LogErrorCounter>();
    auto stringLog = std::make_shared<StringLogger>();
    LogCentral::getPtr()->registerLogger(logCounter);
    LogCentral::getPtr()->registerLogger(stringLog);

    auto s = std::shared_ptr<Processor>(factory_->createShared(GetParam()));

    LGL_ERROR;

    ASSERT_TRUE(s.get() != nullptr) << "Could not create processor " << GetParam();
    s->resetAllProperties();

    LGL_ERROR;

    const size_t sizeBefore = network_->getProcessors().size();
    network_->addProcessor(s);
    EXPECT_EQ(sizeBefore + 1, network_->getProcessors().size())
        << "Could not add processor " << GetParam();

    LGL_ERROR;

    network_->removeProcessor(s.get());
    EXPECT_EQ(sizeBefore, network_->getProcessors().size())
        << "Could not remove processor " << GetParam();

    s.reset();  // make sure the processor is deleted.

    LGL_ERROR;

    if (isSupported(factory_->getFactoryObject(GetParam())->getTags())) {
        EXPECT_EQ(0, logCounter->getErrorCount())
            << "Processor " << GetParam() << " produced errors: " << stringLog->getLog();
    } else {
        EXPECT_NE(0, logCounter->getErrorCount())
            << "Processor " << GetParam() << " not supported expected an error message";
    }
}

INSTANTIATE_TEST_SUITE_P(
    RegisteredProcessors, ProcessorCreationTests,
    ::testing::ValuesIn(InviwoApplication::getPtr()->getProcessorFactory()->getKeys()),
    [](const testing::TestParamInfo<std::string>& info) {
        auto name = fmt::format("{:04}_{}", info.index, info.param);
        std::replace_if(name.begin(), name.end(), [](char c) { return !std::isalnum(c); }, '_');
        return name;
    });

}  // namespace inviwo
