/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <modules/unittests/unittestsmodule.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/common/inviwoapplication.h>
#include <modules/unittests/logerrorcounter.h>

using namespace inviwo;

namespace inviwo {
UnitTestsModule::UnitTestsModule(InviwoApplication* app) : InviwoModule(app, "UnitTests") {
    LogErrorCounter::init();
    LogCentral::getPtr()->registerLogger(LogErrorCounter::getPtr());
}

UnitTestsModule::~UnitTestsModule() {
    LogCentral::getPtr()->unregisterLogger(LogErrorCounter::getPtr());
    LogErrorCounter::deleteInstance();
}

int UnitTestsModule::runAllTests() {
    size_t warnCount = LogErrorCounter::getPtr()->getWarnCount();
    size_t errCount = LogErrorCounter::getPtr()->getErrorCount();

    int global_argc = InviwoApplication::getPtr()->getCommandLineParser()->getARGC();
    char **global_argv = InviwoApplication::getPtr()->getCommandLineParser()->getARGV();
    ::testing::InitGoogleTest(&global_argc, global_argv);
    int res = RUN_ALL_TESTS();

    if (res) {
        LogErrorCustom("UnitTestsModule::runAllTests",
                       "Some unit tests did not pass, see console output for details");
    }

    size_t warnCountAfter = LogErrorCounter::getPtr()->getWarnCount();
    size_t errCountAfter = LogErrorCounter::getPtr()->getErrorCount();

    if (warnCount != warnCountAfter) {
        LogWarnCustom("UnitTestsModule::runAllTest", "The tnittest runs generated "
                                                         << (warnCountAfter - warnCount)
                                                         << " warnings");
    }
    if (errCount != errCountAfter) {
        LogWarnCustom("UnitTestsModule::runAllTest", "The tnittest runs generated "
                                                         << (errCountAfter - errCount) << " errors");
    }

    return res;
}

}  // namespace
