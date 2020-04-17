/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2020 Inviwo Foundation
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

#include <inviwo/core/common/defaulttohighperformancegpu.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/core/moduleregistration.h>

#include <inviwo/sgct/sgctmodule.h>

#include <sgct/sgct.h>
#include <sgct/user.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
//#include <modules/glfw/canvasglfw.h>


int main(int argc, char** argv) {
    inviwo::LogCentral logger;
    inviwo::LogCentral::init(&logger);
    auto consoleLogger = std::make_shared<inviwo::ConsoleLogger>();
    logger.registerLogger(consoleLogger);

    sgct::Log::instance().setLogToConsole(false);
    sgct::Log::instance().setLogCallback([&logger](sgct::Log::Level level, const char* message) {
        const auto inviwoLevel = [&]() {
            switch (level) {
                case sgct::Log::Level::Error:
                    return inviwo::LogLevel::Error;
                case sgct::Log::Level::Warning:
                    return inviwo::LogLevel::Warn;
                case sgct::Log::Level::Info:
                    return inviwo::LogLevel::Info;
                case sgct::Log::Level::Debug:
                    return inviwo::LogLevel::Info;
                default:
                    return inviwo::LogLevel::Info;
            }
        }();
        logger.log("SGCT", inviwoLevel, inviwo::LogAudience::Developer, "", "", 0,
                   std::string(message));
    });

    inviwo::InviwoApplication inviwoApp("Inviwo Dome");
    inviwoApp.printApplicationInfo();
    inviwoApp.setPostEnqueueFront([]() { glfwPostEmptyEvent(); });
    inviwoApp.setProgressCallback([&logger](std::string m) {
        logger.log("InviwoApplication", inviwo::LogLevel::Info, inviwo::LogAudience::User, "", "",
                   0, m);
    });

    std::vector<std::string> arg(argv + 1, argv + argc);
    const auto config = sgct::parseArguments(arg);
    const auto cluster = sgct::loadCluster(config.configFilename);

    const auto keyboard = [](sgct::Key key, sgct::Modifier, sgct::Action action, int i) {
        if (key == sgct::Key::Esc && action == sgct::Action::Press) {
            sgct::Engine::instance().terminate();
        } else if (key == sgct::Key::L && action == sgct::Action::Press) {
            sgct::Log::Info("Key press %i", i);
        }
    };

    sgct::Engine::Callbacks callbacks;

    callbacks.initOpenGL = [&](GLFWwindow* shared) {
        //inviwo::CanvasGLFW::provideExternalContext(shared);

        // Initialize all modules
        inviwoApp.registerModules(inviwo::getModuleList());
    };

    // callbacks.initOpenGL = initOGL;
    // callbacks.preSync = preSync;
    // callbacks.encode = encode;
    // callbacks.decode = decode;
    callbacks.draw = [&](const sgct::RenderData& renderData) {
        const auto lookFrom = renderData.viewport.user().posMono();
        // LogInfoCustom("Dome", "LookFrom: " << lookFrom.x << " " << lookFrom.y << " " <<
        // lookFrom.z);
    };
    // callbacks.cleanup = cleanup;
    callbacks.keyboard = keyboard;
    callbacks.drop = [&](int nfiles, const char** files) {
        std::vector<std::string> workspaces(files, files + nfiles);

        inviwoApp.getWorkspaceManager()->load(workspaces.front(), [&](inviwo::ExceptionContext ec) {
            try {
                throw;
            } catch (const inviwo::IgnoreException& e) {
                inviwo::util::log(e.getContext(),
                                  "Incomplete network loading " + workspaces.front() + " due to " +
                                      e.getMessage(),
                                  inviwo::LogLevel::Error);
            }
        });
    };

    callbacks.mouseButton = [&](sgct::MouseButton button, sgct::Modifier modifier,
                                sgct::Action action) {
        LogInfoCustom("Dome", "Mouse: " << static_cast<int>(button));
    };

    try {
        sgct::Engine::create(cluster, callbacks, config);
    } catch (const std::runtime_error& e) {
        sgct::Log::Error("%s", e.what());
        sgct::Engine::destroy();
        return EXIT_FAILURE;
    }

    sgct::Engine::instance().render();
    sgct::Engine::destroy();
}
