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

#include <inviwo/testutil/configurablegtesteventlistener.h>

namespace inviwo {

ConfigurableGTestEventListener::ConfigurableGTestEventListener(
    ::testing::TestEventListener* theEventListener)
    : eventListener{theEventListener} {
    showTestCases = true;
    showTestNames = true;
    showSuccesses = true;
    showInlineFailures = true;
    showEnvironment = true;
}
ConfigurableGTestEventListener& ConfigurableGTestEventListener::setup() {
    ::testing::TestEventListeners& listeners = testing::UnitTest::GetInstance()->listeners();
    auto default_printer = listeners.Release(listeners.default_result_printer());

    auto listener = new ConfigurableGTestEventListener(default_printer);
    listener->showEnvironment = false;
    listener->showTestCases = false;
    listener->showTestNames = false;
    listener->showSuccesses = false;
    listener->showInlineFailures = false;
    listeners.Append(listener);

    return *listener;
}
ConfigurableGTestEventListener::~ConfigurableGTestEventListener() = default;

void ConfigurableGTestEventListener::OnTestProgramStart(const ::testing::UnitTest& unit_test) {
    eventListener->OnTestProgramStart(unit_test);
}

void ConfigurableGTestEventListener::OnTestIterationStart(const ::testing::UnitTest& unit_test,
                                                          int iteration) {
    eventListener->OnTestIterationStart(unit_test, iteration);
}

void ConfigurableGTestEventListener::OnEnvironmentsSetUpStart(
    const ::testing::UnitTest& unit_test) {
    if (showEnvironment) {
        eventListener->OnEnvironmentsSetUpStart(unit_test);
    }
}

void ConfigurableGTestEventListener::OnEnvironmentsSetUpEnd(const ::testing::UnitTest& unit_test) {
    if (showEnvironment) {
        eventListener->OnEnvironmentsSetUpEnd(unit_test);
    }
}

void ConfigurableGTestEventListener::OnTestCaseStart(const ::testing::TestCase& test_case) {
    if (showTestCases) {
        eventListener->OnTestCaseStart(test_case);
    }
}

void ConfigurableGTestEventListener::OnTestStart(const ::testing::TestInfo& test_info) {
    if (showTestNames) {
        eventListener->OnTestStart(test_info);
    }
}

void ConfigurableGTestEventListener::OnTestPartResult(const ::testing::TestPartResult& result) {
    eventListener->OnTestPartResult(result);
}

void ConfigurableGTestEventListener::OnTestEnd(const ::testing::TestInfo& test_info) {
    if ((showInlineFailures && test_info.result()->Failed()) ||
        (showSuccesses && !test_info.result()->Failed())) {
        eventListener->OnTestEnd(test_info);
    }
}

void ConfigurableGTestEventListener::OnTestCaseEnd(const ::testing::TestCase& test_case) {
    if (showTestCases) {
        eventListener->OnTestCaseEnd(test_case);
    }
}

void ConfigurableGTestEventListener::OnEnvironmentsTearDownStart(
    const ::testing::UnitTest& unit_test) {
    if (showEnvironment) {
        eventListener->OnEnvironmentsTearDownStart(unit_test);
    }
}

void ConfigurableGTestEventListener::OnEnvironmentsTearDownEnd(
    const ::testing::UnitTest& unit_test) {
    if (showEnvironment) {
        eventListener->OnEnvironmentsTearDownEnd(unit_test);
    }
}

void ConfigurableGTestEventListener::OnTestIterationEnd(const ::testing::UnitTest& unit_test,
                                                        int iteration) {
    eventListener->OnTestIterationEnd(unit_test, iteration);
}

void ConfigurableGTestEventListener::OnTestProgramEnd(const ::testing::UnitTest& unit_test) {
    eventListener->OnTestProgramEnd(unit_test);
}

}  // namespace inviwo
