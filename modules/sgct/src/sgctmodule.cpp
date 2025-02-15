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

#include <inviwo/sgct/sgctmodule.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/sgct/datastructures/sgctcamera.h>
#include <inviwo/sgct/sgctutil.h>
#include <inviwo/sgct/io/communication.h>
#include <inviwo/sgct/networksyncmanager.h>
#include <inviwo/sgct/sgctsettings.h>

#include <thread>
#include <atomic>

#include <sgct/sgct.h>

namespace inviwo {

class SGCTWrapper {
public:
    SGCTWrapper(ProcessorNetwork& net, std::string_view configFile, SGCTSettings* settings)
        : syncServer_{net, settings}
        , terminate_{false}
        , runner_{&SGCTWrapper::run, this, configFile} {

        if (settings) {
            showSGCTStatisticsOverlayCallbackHandle_ =
                settings->showSGCTStatisticsOverlay.onChangeScoped([this, settings]() {
                    syncServer_.showStats(settings->showSGCTStatisticsOverlay.get());
                });
        }
    }

    SGCTWrapper(const SGCTWrapper&) = delete;
    SGCTWrapper& operator=(const SGCTWrapper&) = delete;

    SGCTWrapper(SGCTWrapper&&) = delete;
    SGCTWrapper& operator=(SGCTWrapper&&) = delete;

    ~SGCTWrapper() {
        terminate_ = true;
        runner_.join();
    }

private:
    void run(std::string_view configFile) {
        sgct::Log::instance().setLogToConsole(false);
        sgct::Log::instance().setLogCallback([](sgct::Log::Level level, std::string_view message) {
            LogCentral::getPtr()->log("SGCT", util::sgctToInviwo(level),
                                      inviwo::LogAudience::Developer, "", "", 0, message);
        });

        const sgct::Configuration config{
            .configFilename = std::string{configFile},
            .logLevel = sgct::Log::Level::Debug,
        };
        const auto cluster = sgct::loadCluster(config.configFilename);

        const sgct::Engine::Callbacks callbacks{.preSync =
                                                    [this]() mutable {
                                                        if (terminate_) {
                                                            sgct::Engine::instance().terminate();
                                                        }
                                                    },
                                                .encode = [this]() -> std::vector<std::byte> {
                                                    return syncServer_.getEncodedCommandsAndClear();
                                                }

        };

        try {
            LogInfoCustom("SGCTWrapper", "Start Engine");
            const inviwo::util::OnScopeExit closeEngine{[]() { sgct::Engine::destroy(); }};
            sgct::Engine::create(cluster, callbacks, config);

            for (const auto& win : sgct::Engine::instance().windows()) {
                win->setRenderWhileHidden(true);
                win->setVisible(false);
            }

            sgct::Engine::instance().exec();
        } catch (const std::runtime_error& e) {
            sgct::Log::Error(e.what());
        }
    }

    NetworkSyncServer syncServer_;
    std::atomic<bool> terminate_;
    std::thread runner_;
    std::shared_ptr<std::function<void()>> showSGCTStatisticsOverlayCallbackHandle_;
};

SGCTModule::SGCTModule(InviwoApplication* app)
    : InviwoModule(app, "sgct")
    , settings{}
    , configFileArg_{"",    "config", "Path to the SGCT configuration file to use",
                     false, "path",   "SGCT json config file"}
    , argHolder_{app, configFileArg_,
                 [this, app]() {
                     sgctWrapper_ = std::make_unique<SGCTWrapper>(
                         *app->getProcessorNetwork(), configFileArg_.getValue(), &settings);
                 },
                 10} {

    registerCamera<SGCTCamera>(SGCTCamera::classIdentifier);
    registerSettings(&settings);
}

SGCTModule::~SGCTModule() = default;

}  // namespace inviwo
