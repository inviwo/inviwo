/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <inviwo/core/common/coremodulesharedlibrary.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/testutil/configurablegtesteventlistener.h>

#include <inviwo/core/datastructures/representationutil.h>
#include <inviwo/core/datastructures/representationfactorymanager.h>

#include <modules/base/basemodulesharedlibrary.h>
#include <modules/nifti/niftimodulesharedlibrary.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

int main(int argc, char** argv) {
    using namespace inviwo;
    LogCentral::init();
    auto logger = std::make_shared<ConsoleLogger>();
    LogCentral::getPtr()->setVerbosity(LogVerbosity::Info);
    LogCentral::getPtr()->registerLogger(logger);

    InviwoApplication app(argc, argv, "Inviwo-Unittests-Nifti");
    {
        std::vector<std::unique_ptr<InviwoModuleFactoryObject>> modules;
        modules.emplace_back(createInviwoCore());
        modules.emplace_back(createBaseModule());
        modules.emplace_back(createNiftiModule());

        app.registerModules(std::move(modules));
    }

    app.processFront();
    int ret = -1;
    {
        ::testing::InitGoogleTest(&argc, argv);
        ConfigurableGTestEventListener::setup();
        ret = RUN_ALL_TESTS();
    }

    return ret;
}
