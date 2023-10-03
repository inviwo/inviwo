/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#ifdef WIN32
#include <windows.h>
#endif

#include <modules/opengl/inviwoopengl.h>
#include <modules/glfw/canvasglfw.h>

#include <inviwo/core/common/defaulttohighperformancegpu.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/logerrorcounter.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/rendercontext.h>

#include <inviwo/testutil/configurablegtesteventlistener.h>

#include <modules/opengl/openglmodule.h>
#include <modules/opengl/openglsettings.h>

#include <inviwo/sys/moduleregistration.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

using namespace inviwo;

int main(int argc, char** argv) {
    int ret = -1;
    {
        // scope for ivw app
        LogCentral logCentral;
        LogCentral::init(&logCentral);

        auto logCounter = std::make_shared<LogErrorCounter>();
        LogCentral::getPtr()->registerLogger(logCounter);

        InviwoApplication inviwoApp(argc, argv, "Inviwo-IntegrationTests");
        inviwoApp.getSystemSettings().stackTraceInException_.set(true);
        inviwoApp.setPostEnqueueFront([]() { glfwPostEmptyEvent(); });
        inviwoApp.setProgressCallback([](std::string m) {
            LogCentral::getPtr()->log("InviwoApplication", LogLevel::Info, LogAudience::User, "",
                                      "", 0, m);
        });

        // Initialize all modules
        inviwoApp.registerModules(inviwo::getModuleList());
        inviwoApp.resizePool(0);
        inviwoApp.printApplicationInfo();

        RenderContext::getPtr()->activateDefaultRenderContext();

        for (auto* settings : inviwoApp.getModuleByType<OpenGLModule>()->getSettings()) {
            if (auto glSettings = dynamic_cast<OpenGLSettings*>(settings)) {
                glSettings->debugMessages_.set(utilgl::debug::Mode::DebugSynchronous);
                glSettings->debugSeverity_.set(utilgl::debug::Severity::Medium);
            }
        }

        auto& cmdparser = inviwoApp.getCommandLineParser();

        cmdparser.processCallbacks();  // run any command line callbacks from modules.

        size_t warnCount = logCounter->getWarnCount();
        size_t errCount = logCounter->getErrorCount();

        ::testing::InitGoogleTest(&argc, argv);
        ConfigurableGTestEventListener::setup();
        ret = RUN_ALL_TESTS();

        if (ret) {
            LogErrorCustom("UnitTestsModule::runAllTests",
                           "Some unit tests did not pass, see console output for details");
        }

        size_t warnCountAfter = logCounter->getWarnCount();
        size_t errCountAfter = logCounter->getErrorCount();

        if (warnCount != warnCountAfter) {
            LogWarnCustom("UnitTestsModule::runAllTest", "The integration test runs generated "
                                                             << (warnCountAfter - warnCount)
                                                             << " warnings");
        }
        if (errCount != errCountAfter) {
            LogWarnCustom("UnitTestsModule::runAllTest", "The  integration test runs generated "
                                                             << (errCountAfter - errCount)
                                                             << " errors");
        }
    }

    glfwTerminate();
    return ret;
}
