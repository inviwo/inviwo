/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/core/common/coremodulesharedlibrary.h>
#include <modules/python3/python3modulesharedlibrary.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

using namespace inviwo;

int main(int argc, char** argv) {

    LogCentral::init();
    auto logger = std::make_shared<ConsoleLogger>();
    LogCentral::getPtr()->setLogLevel(LogLevel::Error);
    LogCentral::getPtr()->registerLogger(logger);
    InviwoApplication app(argc, argv, "Inviwo-Unittests-Python");

    {
        std::vector<std::unique_ptr<InviwoModuleFactoryObject>> modules;
        modules.emplace_back(createInviwoCore());
        modules.emplace_back(createPython3Module());
        app.registerModules(std::move(modules));
    }

    app.processFront();

    int ret = -1;
    {

#ifdef IVW_ENABLE_MSVC_MEM_LEAK_TEST
        VLDDisable();
        ::testing::InitGoogleTest(&argc, argv);
        VLDEnable();
#else
        ::testing::InitGoogleTest(&argc, argv);
#endif
        ret = RUN_ALL_TESTS();
    }

    return ret;
}