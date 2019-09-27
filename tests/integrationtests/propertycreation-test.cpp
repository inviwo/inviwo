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

#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/util/logerrorcounter.h>
#include <inviwo/core/util/stringlogger.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

namespace {

struct LogErrorCheck {
    LogErrorCheck()
        : logCounter_{std::make_shared<LogErrorCounter>()}
        , stringLog_{std::make_shared<StringLogger>()} {
        LogCentral::getPtr()->registerLogger(logCounter_);
        LogCentral::getPtr()->registerLogger(stringLog_);
    }
    ~LogErrorCheck() { EXPECT_EQ(0, logCounter_->getErrorCount()) << stringLog_->getLog(); }

    std::shared_ptr<LogErrorCounter> logCounter_;
    std::shared_ptr<StringLogger> stringLog_;
};

}  // namespace

class PropertyCreationTests : public ::testing::TestWithParam<std::string> {
protected:
    PropertyCreationTests() : factory_{InviwoApplication::getPtr()->getPropertyFactory()} {};

    virtual ~PropertyCreationTests() = default;

    virtual void SetUp() override {}
    virtual void TearDown() override {}

    PropertyFactory* factory_;
};

TEST_P(PropertyCreationTests, Create) {
    LogErrorCheck checklog;
    auto s = factory_->create(GetParam(), "identifier", "displayname");
    ASSERT_NE(s, nullptr);

    auto c = std::unique_ptr<Property>(s->clone());
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(s->getIdentifier(), c->getIdentifier());
    EXPECT_EQ(s->getPath(), c->getPath());
    EXPECT_EQ(s->getDisplayName(), c->getDisplayName());
    EXPECT_EQ(s->getClassIdentifier(), c->getClassIdentifier());
    EXPECT_EQ(s->getSemantics(), c->getSemantics());

    // Try linking
    c->set(s.get());
    s->set(c.get());

    if (auto sComp = dynamic_cast<CompositeProperty*>(s.get())) {
        auto cComp = dynamic_cast<CompositeProperty*>(c.get());
        ASSERT_NE(cComp, nullptr);

        EXPECT_EQ(sComp->isCollapsed(), cComp->isCollapsed())
            << "Collapse state mismatch in property copy \'" << GetParam() << "\'";

        auto sProps = sComp->getPropertiesRecursive();
        auto cProps = cComp->getPropertiesRecursive();
        ASSERT_EQ(sProps.size(), cProps.size())
            << "Number of subproperties does not match for property copy \'" << GetParam() << "\'";

        for (auto&& [org, clone] : util::zip(sProps, cProps)) {
            const std::string errorMsg = " in property copy \'" + org->getClassIdentifier() +
                                         "\' (base property: \'" + GetParam() + "\')";

            EXPECT_EQ(org->getIdentifier(), clone->getIdentifier())
                << "Identifier mismatch" << errorMsg;
            EXPECT_EQ(org->getPath(), clone->getPath()) << "Property path mismatch" << errorMsg;
            EXPECT_EQ(org->getDisplayName(), clone->getDisplayName())
                << "DisplayName mismatch" << errorMsg;
            EXPECT_EQ(org->getClassIdentifier(), clone->getClassIdentifier())
                << "ClassIdentifier mismatch" << errorMsg;
            EXPECT_EQ(org->getSemantics(), clone->getSemantics())
                << "Semantics mismatch" << errorMsg;
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    RegisteredProperties, PropertyCreationTests,
    ::testing::ValuesIn(InviwoApplication::getPtr()->getPropertyFactory()->getKeys()));
}  // namespace inviwo
