/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/network/lambdanetworkvisitor.h>

#include <inviwo/sys/moduleloading.h>
#include <inviwo/py/pythonhelper.h>

#include <modules/opengl/openglutils.h>

#include <inviwo/sgct/sgctutil.h>
#include <inviwo/sgct/io/communication.h>
#include <inviwo/sgct/networksyncmanager.h>
#include <inviwo/sgct/sgctmodule.h>
#include <inviwo/sgct/datastructures/sgctcamera.h>
#include <inviwo/sgct/sgctsettings.h>

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

#include <fmt/format.h>

#include <inviwo/tracy/tracy.h>
#include <inviwo/tracy/tracyopengl.h>

#ifdef IVW_CFG_TRACY_MENORY_PROFILING
void* operator new(size_t count) {
    void* ptr = malloc(count);
    TRACY_ALLOC_S(ptr, count, 10);
    // TRACY_ALLOC(ptr, count);
    return ptr;
}
void operator delete(void* ptr) noexcept {
    TRACY_FREE_S(ptr, 10);
    // TRACY_FREE(ptr);
    free(ptr);
}
#endif

struct SGCTCallbacks {

    SGCTCallbacks(inviwo::SGCTManager& aManager, inviwo::InviwoApplication& app)
        : app{app}, manager{aManager}, drawBuffers{} {

        {
            syncServer.emplace(*app.getProcessorNetwork());
            manager.onStatChange = [this](bool show) {
                syncServer->showStats(show);
                sgct::Engine::instance().setStatsGraphVisibility(show);
            };
        }
        {
            syncClient.emplace(*app.getProcessorNetwork());
            syncClient->onStats = [](bool show) {
                sgct::Engine::instance().setStatsGraphVisibility(show);
            };
        }
    }
    SGCTCallbacks(SGCTCallbacks&&) = delete;
    SGCTCallbacks& operator=(SGCTCallbacks&&) = delete;
    SGCTCallbacks(const SGCTCallbacks&) = delete;
    SGCTCallbacks& operator=(const SGCTCallbacks&) = delete;
    ~SGCTCallbacks() = default;

    inviwo::InviwoApplication& app;
    inviwo::SGCTManager& manager;
    std::vector<GLenum> drawBuffers{};

    std::optional<inviwo::NetworkSyncServer> syncServer{};
    std::optional<inviwo::NetworkSyncClient> syncClient{};

    std::mutex commandsMutex{};
    std::vector<inviwo::SgctCommand> commands{};

    static void setCamerasToSGCT(inviwo::ProcessorNetwork& net) {
        inviwo::LambdaNetworkVisitor visitor{[](inviwo::Property& p) {
            if (auto* camera = dynamic_cast<inviwo::CameraProperty*>(&p)) {
                camera->setCamera("SGCTCamera");
            }
        }};
        net.accept(visitor);
    }

    void initOpenGL(GLFWwindow* shared) {
        // Tell GLFW that we already have a shared context;
        inviwo::CanvasGLFW::provideExternalContext(shared);

        inviwo::initializePythonModules();

        // Remove the qt opengl module
        auto filter = [](const inviwo::ModuleContainer& m) {
            return m.identifier().ends_with("qt") || m.identifier().starts_with("qt");
        };
        inviwo::util::registerModulesFiltered(app.getModuleManager(), filter,
                                              app.getSystemSettings().moduleSearchPaths_.get());

        if (syncServer) {
            syncServer->setSettings(app.getSettingsByType<inviwo::SGCTSettings>());
        }

        manager.createShader();

        if (sgct::Engine::instance().isMaster()) {
            for (const auto& win : sgct::Engine::instance().windows()) {
                manager.setupInteraction(win->windowHandle());
            }
        }
        GLint maxDrawBuffers = 8;
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
        drawBuffers.resize(static_cast<size_t>(maxDrawBuffers), GL_NONE);

        if (sgct::Engine::instance().isMaster()) {
            app.getWorkspaceManager()->load(
                inviwo::filesystem::getPath(inviwo::PathType::Workspaces, "/boron.inv"));
            setCamerasToSGCT(*app.getProcessorNetwork());
        } else {
            // Only the master node needs to have any widgets.
            auto* wf = app.getProcessorWidgetFactory();
            auto wfKeys = wf->getKeys();
            for (auto& key : wfKeys) {
                wf->unRegisterObject(wf->getFactoryObject(key));
            }
        }
    };

    void preSync() {
        TRACY_ZONE_SCOPED_NC("Process Front", 0xAA0000);

        // We check the front queue before every frame. No need for setPostEnqueueFront
        app.processFront();
    };

    auto encode() -> std::vector<std::byte> {
        if (syncServer) {
            return syncServer->getEncodedCommandsAndClear();
        } else {
            return {};
        }
    }

    void decode(const std::vector<std::byte>& bytes) {
        const std::scoped_lock lock{commandsMutex};
        inviwo::util::decode(bytes, commands);
    }

    void postSyncPreDraw() {
        if (syncClient) {
            const std::scoped_lock lock{commandsMutex};
            syncClient->applyCommands(commands);
            commands.clear();
        }
    };

    void draw(const sgct::RenderData& renderData) {
        TRACY_ZONE_SCOPED_NC("Draw", 0xAAAA00);
        TRACY_GPU_ZONE_C("Draw", 0xAAAA00);

        // Save State that SGCT needs since inviwo might mess with this.
        inviwo::utilgl::Viewport view;
        GLint sgctFBO = 0;
        {
            TRACY_ZONE_SCOPED_NC("Save State", 0xAA66000);
            TRACY_GPU_ZONE_C("Save State", 0xAA66000);
            view.get();

            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &sgctFBO);

            std::fill(drawBuffers.begin(), drawBuffers.end(), GL_NONE);
            for (size_t i = 0; i < drawBuffers.size(); ++i) {
                GLint value = 0;
                glGetIntegerv(GL_DRAW_BUFFER0 + static_cast<unsigned int>(i), &value);
                drawBuffers[i] = static_cast<GLenum>(value);
            }
        }
        // Do inviwo stuff
        {
            TRACY_ZONE_SCOPED_NC("Eval", 0xAA66000);
            TRACY_GPU_ZONE_C("Eval", 0xAA66000);
            manager.evaluate(renderData);
        }
        // Restore state
        {
            TRACY_ZONE_SCOPED_NC("Restore State", 0xAA66000);
            TRACY_GPU_ZONE_C("Restore State", 0xAA66000);
            sgct::Window::makeSharedContextCurrent();
            view.set();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, sgctFBO);
            glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
        }

        // Copy inviwo output to sgct
        manager.copy();
    };

    // be consistent with the other callbacks
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void postDraw() {
        TRACY_ZONE_SCOPED_NC("Post Draw", 0xAA0000);
        TRACY_GPU_ZONE_C("Post Draw", 0xAA0000);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    };

    void drop(const std::vector<std::string_view>& files) {
        if (sgct::Engine::instance().isMaster() && !files.empty()) {
            auto path = std::filesystem::canonical(files[0]);
            app.getWorkspaceManager()->clear();
            app.getWorkspaceManager()->load(path);
            setCamerasToSGCT(*app.getProcessorNetwork());
        }
    };

    // NOLINTBEGIN(cppcoreguidelines-missing-std-forward)
    template <typename Fun>
    static auto tryWrapper(Fun&& function) {
        return [fun = std::forward<Fun>(function)](auto&&... args) {
            try {
                fun(std::forward<decltype(args)>(args)...);
            } catch (const inviwo::Exception& e) {
                inviwo::util::log(e.getContext(), e.getMessage(), inviwo::LogLevel::Error);
            } catch (const std::exception& e) {
                LogErrorCustom("main", e.what());
            } catch (...) {
                LogErrorCustom("main", "some unkown error");
            }
        };
    }

    template <typename Fun>
    static auto tryWrapperRet(Fun&& function) {
        return [fun = std::forward<Fun>(function)](auto&&... args) {
            try {
                return fun(std::forward<decltype(args)>(args)...);
            } catch (const inviwo::Exception& e) {
                inviwo::util::log(e.getContext(), e.getMessage(), inviwo::LogLevel::Error);
                return decltype(fun(std::forward<decltype(args)>(args)...)){};
            } catch (const std::exception& e) {
                LogErrorCustom("main", e.what());
                return decltype(fun(std::forward<decltype(args)>(args)...)){};
            } catch (...) {
                LogErrorCustom("main", "some unkown error");
                return decltype(fun(std::forward<decltype(args)>(args)...)){};
            }
        };
    }
    // NOLINTEND(cppcoreguidelines-missing-std-forward)

    void configureSGCTCallbacks(sgct::Engine::Callbacks& callbacks) {
        callbacks.initOpenGL = tryWrapper([this](GLFWwindow* shared) { initOpenGL(shared); });
        callbacks.preSync = tryWrapper([this]() { preSync(); });
        callbacks.encode = tryWrapperRet([this]() -> std::vector<std::byte> { return encode(); });
        callbacks.decode = tryWrapper([this](const std::vector<std::byte>& data) { decode(data); });
        callbacks.postSyncPreDraw = tryWrapper([this]() { postSyncPreDraw(); });
        callbacks.draw =
            tryWrapper([this](const sgct::RenderData& renderData) { draw(renderData); });
        callbacks.postDraw = tryWrapper([this]() { postDraw(); });
        callbacks.drop =
            tryWrapper([&](const std::vector<std::string_view>& files) { drop(files); });
    }
};

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
                logger.log("SGCT", inviwo::util::sgctToInviwo(level),
                           inviwo::LogAudience::Developer, "", "", 0, message);
            });

        const inviwo::util::OnScopeExit closeEngine{[]() {
            LogInfoCustom("Dome", "Stop Engine");
            // Stop the engine after we clear the inviwo app.
            // The app will need the glfw context to be active to join
            // any background threads with a context.
            sgct::Engine::destroy();
        }};
        inviwo::InviwoApplication app("Inviwo");
        app.printApplicationInfo();
        app.setProgressCallback([&logger](std::string_view m) {
            logger.log("InviwoApplication", inviwo::LogLevel::Info, inviwo::LogAudience::User, "",
                       "", 0, m);
        });
        app.setFileSystemObserver(std::make_unique<inviwo::FileWatcher>(&app));

        std::vector<std::string> arg(argv + 1, argv + argc);
        const auto config = sgct::parseArguments(arg);
        const auto cluster = sgct::loadCluster(config.configFilename);
        if (!cluster.success) {
            return -1;
        }

        inviwo::SGCTManager state(app);
        SGCTCallbacks conf(state, app);

        const inviwo::util::OnScopeExit clearNetwork{
            [&app]() { app.getWorkspaceManager()->clear(); }};

        // Keep the network locked to prevent evaluating it all the time, only unlock it in the draw
        // call in SGCTManager::evaluate
        app.getProcessorNetwork()->lock();

        sgct::Engine::Callbacks callbacks;
        conf.configureSGCTCallbacks(callbacks);

        LogInfoCustom("Dome", "Start Engine");

        sgct::Engine::create(cluster, callbacks, config);
        sgct::Engine::instance().exec();

    } catch (const inviwo::Exception& e) {
        inviwo::util::log(e.getContext(), e.getMessage(), inviwo::LogLevel::Error);
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        LogErrorCustom("main", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        LogErrorCustom("main", "some other error");
        return EXIT_FAILURE;
    }
}
