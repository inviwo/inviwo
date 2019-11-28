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
#pragma once

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <memory>

namespace inviwo {

class ConfigurableGTestEventListener : public ::testing::TestEventListener {
public:
    bool showTestCases;       ///< Show the names of each test case.
    bool showTestNames;       ///< Show the names of each test.
    bool showSuccesses;       ///< Show each success.
    bool showInlineFailures;  ///< Show each failure as it occurs.
    bool showEnvironment;     ///< Show the setup of the global environment.

    explicit ConfigurableGTestEventListener(::testing::TestEventListener* theEventListener);
    virtual ~ConfigurableGTestEventListener();

    static ConfigurableGTestEventListener& setup();

    virtual void OnTestProgramStart(const ::testing::UnitTest& unit_test) override;
    virtual void OnTestIterationStart(const ::testing::UnitTest& unit_test, int iteration) override;
    virtual void OnEnvironmentsSetUpStart(const ::testing::UnitTest& unit_test) override;
    virtual void OnEnvironmentsSetUpEnd(const ::testing::UnitTest& unit_test) override;
    virtual void OnTestCaseStart(const ::testing::TestCase& test_case) override;
    virtual void OnTestStart(const ::testing::TestInfo& test_info) override;
    virtual void OnTestPartResult(const ::testing::TestPartResult& result) override;
    virtual void OnTestEnd(const ::testing::TestInfo& test_info) override;
    virtual void OnTestCaseEnd(const ::testing::TestCase& test_case) override;
    virtual void OnEnvironmentsTearDownStart(const ::testing::UnitTest& unit_test) override;
    virtual void OnEnvironmentsTearDownEnd(const ::testing::UnitTest& unit_test) override;
    virtual void OnTestIterationEnd(const ::testing::UnitTest& unit_test, int iteration) override;
    virtual void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override;

protected:
    std::unique_ptr<::testing::TestEventListener> eventListener;
};

}  // namespace inviwo
