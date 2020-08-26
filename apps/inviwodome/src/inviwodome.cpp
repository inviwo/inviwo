/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/core/util/stringconversion.h>

#include <inviwo/core/util/filesystem.h>

#include <inviwo/core/moduleregistration.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/network/networklock.h>

#include <modules/opengl/openglutils.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <modules/glfw/canvasglfw.h>
#include <modules/glfw/filewatcher.h>
#include <modules/glfw/glfwmodule.h>

#include <sgct/sgct.h>
#include <sgct/user.h>
#include <sgct/offscreenbuffer.h>

#include <inviwo/dome/sgctmanager.h>
#include <inviwo/dome/dumpfiles.h>
#include <inviwo/dome/workspacewatcher.h>

#include <fmt/format.h>

#include <inviwo/tracy/tracy.h>
#include <inviwo/tracy/tracyopengl.h>

void* operator new(size_t count) {
    void* ptr = malloc(count);
    // TracyAllocS(ptr, count, 10);
    TRACY_ALLOC(ptr, count);
    return ptr;
}

void operator delete(void* ptr) noexcept {
    // TracyFreeS(ptr, 10);
    TRACY_FREE(ptr);
    free(ptr);
}

enum class Command : int { Invalid = -1, Nop = 0, Load = 1, Update, ShowStats, HideStats };
auto serializeCommand(std::vector<std::byte>& data, Command cmd) -> void {
    sgct::serializeObject(data, static_cast<int>(cmd));
}
auto deserializeCommand(const std::vector<std::byte>& data, unsigned int& pos) -> Command {
    int command = -1;
    sgct::deserializeObject(data, pos, command);
    return static_cast<Command>(command);
}
auto inviwoLevel(sgct::Log::Level level) -> inviwo::LogLevel {
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
}

int main(int argc, char** argv) {
#ifdef WIN32
    SetUnhandledExceptionFilter(inviwo::generateMiniDump);
#endif  // WIN32

    try {
        inviwo::LogCentral logger;
        inviwo::LogCentral::init(&logger);
        auto consoleLogger = std::make_shared<inviwo::ConsoleLogger>();
        logger.registerLogger(consoleLogger);

        sgct::Log::instance().setLogToConsole(false);
        sgct::Log::instance().setLogCallback(
            [&logger](sgct::Log::Level level, std::string_view message) {
                logger.log("SGCT", inviwoLevel(level), inviwo::LogAudience::Developer, "", "", 0,
                           message);
            });

        inviwo::util::OnScopeExit closeEngine{[]() { sgct::Engine::destroy(); }};

        inviwo::InviwoApplication app("Inviwo");
        app.printApplicationInfo();
        app.setProgressCallback([&logger](std::string m) {
            logger.log("InviwoApplication", inviwo::LogLevel::Info, inviwo::LogAudience::User, "",
                       "", 0, m);
        });
        app.setFileSystemObserver(std::make_unique<inviwo::FileWatcher>(&app));

        std::vector<std::string> arg(argv + 1, argv + argc);
        const auto config = sgct::parseArguments(arg);
        const auto cluster = sgct::loadCluster(config.configFilename);

        inviwo::SgctManager state(app);

        sgct::Engine::Callbacks callbacks;
        std::string pathToLoad;
        std::string serializedProps;
        std::optional<bool> showStats;
        inviwo::WorkspaceWatcher wo(&app, pathToLoad);

        // Keep the network looked all the time, only unlock in the draw call
        app.getProcessorNetwork()->lock();

        std::vector<GLenum> drawBuffers;

        callbacks.initOpenGL = [&](GLFWwindow* shared) {
            // Tell GLFW that we already have a shared context;
            inviwo::CanvasGLFW::provideExternalContext(shared);

            // Initialize all modules
            app.registerModules(inviwo::getModuleList());

            // app.getModuleByType<inviwo::GLFWModule>()->setWaitForOpenGL(false);

            state.createShader();

            if (sgct::Engine::instance().isMaster()) {
                for (auto& win : sgct::Engine::instance().windows()) {
                    state.setupUpInteraction(win->windowHandle());
                }
                pathToLoad = app.getPath(inviwo::PathType::Workspaces, "/boron.inv");
            }

            GLint maxDrawBuffers = 8;
            glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
            drawBuffers.resize(static_cast<size_t>(maxDrawBuffers), GL_NONE);
        };

        callbacks.preSync = [&]() {
            TRACY_ZONE_SCOPED_NC("Process Front", 0xAA0000);

            // We check the front queue before every frame. No need for setPostEnqueueFront
            app.processFront();
        };

        callbacks.encode = [&]() -> std::vector<std::byte> {
            TRACY_ZONE_SCOPED_NC("Encode", 0xAA0000);

            std::vector<std::byte> data;

            if (!pathToLoad.empty()) {
                serializeCommand(data, Command::Load);

                const auto relpath = inviwo::filesystem::getRelativePath(
                    inviwo::filesystem::findBasePath(), pathToLoad);
                sgct::serializeObject(data, relpath);
            }

            if (!state.modifiedProperties.empty()) {
                serializeCommand(data, Command::Update);
                std::stringstream os;
                state.getUpdates(os);
                sgct::serializeObject(data, std::move(os).str());

                state.modifiedProperties.clear();
            }

            if (state.showStats) {
                serializeCommand(data, *state.showStats ? Command::ShowStats : Command::HideStats);
            }

            serializeCommand(data, Command::Nop);
            return data;
        };

        callbacks.decode = [&](const std::vector<std::byte>& data, unsigned int pos) {
            TRACY_ZONE_SCOPED_NC("Decode", 0xAA0000);

            while (true) {
                switch (deserializeCommand(data, pos)) {
                    case Command::Load: {
                        std::string relpath;
                        sgct::deserializeObject(data, pos, relpath);
                        pathToLoad = inviwo::filesystem::findBasePath() + "/" + relpath;
                        break;
                    }
                    case Command::Update: {
                        sgct::deserializeObject(data, pos, serializedProps);
                        break;
                    }
                    case Command::ShowStats: {
                        state.showStats = true;
                        break;
                    }
                    case Command::HideStats: {
                        state.showStats = false;
                        break;
                    }
                    case Command::Invalid: {
                        LogErrorCustom("", "Decode error");
                        return;
                    }
                    case Command::Nop:
                        return;
                    default:
                        return;
                }
            }
        };

        callbacks.postSyncPreDraw = [&]() {
            TRACY_ZONE_SCOPED_NC("Post Sync Pre Draw", 0xAA0000);

            if (!pathToLoad.empty()) {
                wo.stopAllObservation();
                state.loadWorkspace(pathToLoad);
                wo.startFileObservation(pathToLoad);
                pathToLoad.clear();
            }

            if (!serializedProps.empty()) {
                TRACY_ZONE_SCOPED_NC("Apply Updates", 0xAA0000);
                TRACY_ZONE_VALUE(static_cast<int64_t>(serializedProps.size()));
                TRACY_PLOT("Serialized Data Size", static_cast<int64_t>(serializedProps.size()));
                std::stringstream ss(serializedProps);
                state.applyUpdate(ss);
                serializedProps.clear();
            }

            if (state.showStats) {
                sgct::Engine::instance().setStatsGraphVisibility(*state.showStats);
                state.showStats = std::nullopt;
            }
        };

        callbacks.draw = [&](const sgct::RenderData& renderData) {
            TRACY_ZONE_SCOPED_NC("Draw", 0xAAAA00);
            TRACY_GPU_ZONE_C("Draw", 0xAAAA00);

            // Save State
            inviwo::utilgl::Viewport view;
            GLint sgctFBO;
            {
                TRACY_ZONE_SCOPED_NC("Save State", 0xAA66000);
                TRACY_GPU_ZONE_C("Save State", 0xAA66000);
                view.get();

                glGetIntegerv(GL_FRAMEBUFFER_BINDING, &sgctFBO);

                std::fill(drawBuffers.begin(), drawBuffers.end(), GL_NONE);
                for (int i = 0; i < drawBuffers.size(); ++i) {
                    GLint value;
                    glGetIntegerv(GL_DRAW_BUFFER0 + i, &value);
                    drawBuffers[i] = static_cast<GLenum>(value);
                }
            }
            // Do inviwo stuff
            {
                TRACY_ZONE_SCOPED_NC("Eval", 0xAA66000);
                TRACY_GPU_ZONE_C("Eval", 0xAA66000);
                state.evaluate(renderData);
            }
            // Restore state
            {
                TRACY_ZONE_SCOPED_NC("Restore State", 0xAA66000);
                TRACY_GPU_ZONE_C("Restore State", 0xAA66000);
                renderData.window.makeSharedContextCurrent();
                view.set();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindFramebuffer(GL_FRAMEBUFFER, sgctFBO);
                glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
            }

            // Copy inviwo output
            state.copy();
        };

        callbacks.postDraw = [&]() {
            TRACY_ZONE_SCOPED_NC("Post Draw", 0xAA0000);
            TRACY_GPU_ZONE_C("Post Draw", 0xAA0000);

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClearDepth(1.0f);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
        };

        callbacks.drop = [&](int nfiles, const char** files) {
            if (sgct::Engine::instance().isMaster() && nfiles > 0) {
                pathToLoad = inviwo::filesystem::cleanupPath(std::string(files[0]));
            }
        };

        LogInfoCustom("Dome", "Start Engine");

        try {
            sgct::Engine::create(cluster, callbacks, config);
        } catch (const std::runtime_error& e) {
            sgct::Log::Error(e.what());
            sgct::Engine::destroy();
            return EXIT_FAILURE;
        }

        sgct::Engine::instance().render();

    } catch (const std::exception& e) {
        std::cout << e.what();
    } catch (...) {
        std::cout << "some other error";
    }
}
