/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/moduleaction.h>
#include <inviwo/core/common/inviwocommondefines.h>
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/datastructures/camera/camerafactory.h>
#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/metadata/metadatafactory.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/ports/portfactory.h>
#include <inviwo/core/ports/portinspectorfactory.h>
#include <inviwo/core/ports/portinspectormanager.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/propertyconvertermanager.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/propertypresetmanager.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <inviwo/core/rendering/datavisualizermanager.h>
#include <inviwo/core/resourcemanager/resourcemanager.h>
#include <inviwo/core/util/capabilities.h>
#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/filesystemobserver.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/buildinfo.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/core/util/filelogger.h>
#include <inviwo/core/util/timer.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/rendercontext.h>

#include <inviwo/core/resourcemanager/resourcemanagerobserver.h>

#include <chrono>
#include <fmt/std.h>

namespace inviwo {

namespace detail {
class InviwoApplicationCallbacks {
public:
    WorkspaceManager::ClearHandle networkClear;
    WorkspaceManager::SerializationHandle networkSerialization;
    WorkspaceManager::DeserializationHandle networkDeserialization;

    WorkspaceManager::ClearHandle presetsClear;
    WorkspaceManager::SerializationHandle presetsSerialization;
    WorkspaceManager::DeserializationHandle presetsDeserialization;
};
}  // namespace detail

InviwoApplication* InviwoApplication::instance_ = nullptr;

InviwoApplication::InviwoApplication(int argc, char** argv, std::string_view displayName)
    : displayName_(displayName)
    , commandLineParser_(std::make_unique<CommandLineParser>(argc, argv))
    , consoleLogger_{[&]() {
        if (commandLineParser_->getLogToConsole()) {
            auto clog = std::make_shared<ConsoleLogger>();
            LogCentral::getPtr()->registerLogger(clog);
            return clog;
        } else {
            return std::shared_ptr<ConsoleLogger>{};
        }
    }()}
    , filelogger_{[&]() {
        if (commandLineParser_->getLogToFile()) {
            auto filename = commandLineParser_->getLogToFileFileName();
            if (!filename.is_absolute()) {
                auto outputDir = commandLineParser_->getOutputPath();
                if (!outputDir.empty()) {
                    filename = outputDir / filename;
                } else {
                    filename = filesystem::getWorkingDirectory() / filename;
                }
            }
            auto flog = std::make_shared<FileLogger>(filename);
            LogCentral::getPtr()->registerLogger(flog);
            return flog;
        } else {
            return std::shared_ptr<FileLogger>{};
        }
    }()}
    , progressCallback_()
    , pool_(
          0,
          []() {
              if (RenderContext::isInitialized()) {
                  RenderContext::getPtr()->activateLocalRenderContext();
              }
          },
          []() {
              if (RenderContext::isInitialized()) {
                  RenderContext::getPtr()->clearContext();
              }
          })
    , queue_()
    , clearAllSingeltons_{[]() {
        PickingManager::deleteInstance();
        RenderContext::deleteInstance();
    }}
    , resourceManager_{std::make_unique<ResourceManager>()}
    , cameraFactory_{std::make_unique<CameraFactory>()}
    , dataReaderFactory_{std::make_unique<DataReaderFactory>()}
    , dataWriterFactory_{std::make_unique<DataWriterFactory>()}
    , dialogFactory_{std::make_unique<DialogFactory>()}
    , meshDrawerFactory_{std::make_unique<MeshDrawerFactory>()}
    , metaDataFactory_{std::make_unique<MetaDataFactory>()}
    , outportFactory_{std::make_unique<OutportFactory>()}
    , inportFactory_{std::make_unique<InportFactory>()}
    , portInspectorFactory_{std::make_unique<PortInspectorFactory>()}
    , dataVisualizerManager_{std::make_unique<DataVisualizerManager>()}
    , processorFactory_{std::make_unique<ProcessorFactory>(this)}
    , processorWidgetFactory_{std::make_unique<ProcessorWidgetFactory>()}
    , propertyConverterManager_{std::make_unique<PropertyConverterManager>()}
    , propertyFactory_{std::make_unique<PropertyFactory>()}
    , propertyWidgetFactory_{std::make_unique<PropertyWidgetFactory>()}
    , representationMetaFactory_{std::make_unique<RepresentationMetaFactory>()}
    , representationConverterMetaFactory_{std::make_unique<RepresentationConverterMetaFactory>()}
    , settingsRegistry_{}
    , systemSettings_{std::make_unique<SystemSettings>(this)}
    , moduleCallbackActions_{}
    , moduleManager_{std::make_unique<ModuleManager>(this)}
    , processorNetwork_{std::make_unique<ProcessorNetwork>(this)}
    , processorNetworkEvaluator_{std::make_unique<ProcessorNetworkEvaluator>(
          processorNetwork_.get())}
    , workspaceManager_{std::make_unique<WorkspaceManager>(this)}
    , propertyPresetManager_{std::make_unique<PropertyPresetManager>(this)}
    , portInspectorManager_{std::make_unique<PortInspectorManager>(this)}
    , callbacks_{std::make_unique<detail::InviwoApplicationCallbacks>()}
    , layerRamResizer_{nullptr} {

    registerSettings(systemSettings_.get());

    // initialize singletons
    init(this);
    RenderContext::init();
    PickingManager::init();

    resizePool(systemSettings_->poolSize_);
    systemSettings_->poolSize_.onChange([this]() { resizePool(systemSettings_->poolSize_); });

    workspaceManager_->registerFactory(getProcessorFactory());
    workspaceManager_->registerFactory(getMetaDataFactory());
    workspaceManager_->registerFactory(getPropertyFactory());
    workspaceManager_->registerFactory(getInportFactory());
    workspaceManager_->registerFactory(getOutportFactory());
    workspaceManager_->registerFactory(getCameraFactory());

    callbacks_->networkClear = workspaceManager_->onClear([&]() {
        portInspectorManager_->clear();
        processorNetwork_->clear();
    });
    callbacks_->networkSerialization = workspaceManager_->onSave([&](Serializer& s) {
        s.serialize("ProcessorNetwork", *processorNetwork_);
        s.serialize("PortInspectors", *portInspectorManager_);
    });
    callbacks_->networkDeserialization = workspaceManager_->onLoad([&](Deserializer& d) {
        d.deserialize("ProcessorNetwork", *processorNetwork_);
        d.deserialize("PortInspectors", *portInspectorManager_);
    });

    callbacks_->presetsClear =
        workspaceManager_->onClear([&]() { propertyPresetManager_->clearWorkspacePresets(); });
    callbacks_->presetsSerialization = workspaceManager_->onSave(
        [&](Serializer& s) { propertyPresetManager_->saveWorkspacePresets(s); });
    callbacks_->presetsDeserialization = workspaceManager_->onLoad(
        [&](Deserializer& d) { propertyPresetManager_->loadWorkspacePresets(d); });

    // Make sure that all data formats are initialized.
    // Should only be called in the core library, i.e. in InviwoApplication constructor.
    // Need to be done when libraries are loaded at runtime since the
    // data format may be used first in one of the loaded libraries
    // but will not be cleaned up when the module is unloaded.
    DataFormatBase::get();
}

InviwoApplication::InviwoApplication() : InviwoApplication(0, nullptr, "Inviwo") {}

InviwoApplication::InviwoApplication(std::string_view displayName)
    : InviwoApplication(0, nullptr, displayName) {}

InviwoApplication::~InviwoApplication() { resizePool(0); }

void InviwoApplication::registerModules(
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>> moduleFactories) {
    moduleManager_->registerModules(std::move(moduleFactories));
}

void InviwoApplication::registerModules(RuntimeModuleLoading token) {
    moduleManager_->registerModules(token);
}

void InviwoApplication::registerModules(RuntimeModuleLoading token,
                                        std::function<bool(std::string_view)> isEnabled) {
    moduleManager_->registerModules(token, std::move(isEnabled));
}

ModuleManager& InviwoApplication::getModuleManager() { return *moduleManager_; }

const ModuleManager& InviwoApplication::getModuleManager() const { return *moduleManager_; }

InviwoModule* InviwoApplication::getModuleByIdentifier(const std::string& identifier) const {
    return moduleManager_->getModuleByIdentifier(identifier);
}

ProcessorNetwork* InviwoApplication::getProcessorNetwork() { return processorNetwork_.get(); }

ProcessorNetworkEvaluator* InviwoApplication::getProcessorNetworkEvaluator() {
    return processorNetworkEvaluator_.get();
}

WorkspaceManager* InviwoApplication::getWorkspaceManager() { return workspaceManager_.get(); }

PropertyPresetManager* InviwoApplication::getPropertyPresetManager() {
    return propertyPresetManager_.get();
}

PortInspectorManager* InviwoApplication::getPortInspectorManager() {
    return portInspectorManager_.get();
}

DataVisualizerManager* InviwoApplication::getDataVisualizerManager() {
    return dataVisualizerManager_.get();
}

const CommandLineParser& InviwoApplication::getCommandLineParser() const {
    return *commandLineParser_;
}

CommandLineParser& InviwoApplication::getCommandLineParser() { return *commandLineParser_; }

void InviwoApplication::printApplicationInfo() {
    log::info("Inviwo Version: {}", build::version);
    if (auto buildInfo = util::getBuildInfo()) {
        log::info("Build Date: {}", buildInfo->getDate());
        for (auto [name, hash] : buildInfo->githashes) {
            log::info("Git {}  hash: {}", name, hash);
        }
    }
    log::info("Base Path: {}", filesystem::findBasePath());
    log::info("ThreadPool Worker Threads: {}", pool_.getSize());

    log::info("Config: {} [{}] {} ({})", build::generator, build::configuration, build::compiler,
              build::compilerVersion);
}

void InviwoApplication::postProgress(std::string_view progress) const {
    if (progressCallback_) progressCallback_(progress);
}

size_t InviwoApplication::getPoolSize() const { return pool_.getSize(); }

void InviwoApplication::setPostEnqueueFront(std::function<void()> func) {
    queue_.postEnqueue = std::move(func);
}

const std::string& InviwoApplication::getDisplayName() const { return displayName_; }

void InviwoApplication::addCallbackAction(ModuleCallbackAction* callbackAction) {
    moduleCallbackActions_.emplace_back(callbackAction);
}

std::vector<std::unique_ptr<ModuleCallbackAction>>& InviwoApplication::getCallbackActions() {
    return moduleCallbackActions_;
}

void InviwoApplication::registerSettings(Settings* settings) {
    settingsRegistry_.push_back(settings);
}
void InviwoApplication::unregisterSettings(Settings* settings) {
    std::erase(settingsRegistry_, settings);
}

const std::vector<Settings*>& InviwoApplication::getModuleSettings() { return settingsRegistry_; }

SystemSettings& InviwoApplication::getSystemSettings() { return *systemSettings_; }

std::vector<Capabilities*> InviwoApplication::getModuleCapabilities() {
    std::vector<Capabilities*> allModuleCapabilities;
    for (auto& inviwoModule : moduleManager_->getInviwoModules()) {
        auto modCapabilities = inviwoModule.getCapabilities();
        allModuleCapabilities.insert(allModuleCapabilities.end(), modCapabilities.begin(),
                                     modCapabilities.end());
    }
    return allModuleCapabilities;
}

void InviwoApplication::setPoolResizeWaitCallback(std::function<void(LongWait)> callback) {
    poolResizeCallback_ = callback;
}

void InviwoApplication::processEvents() {
    if (processEventsCallback_) processEventsCallback_();
}

void InviwoApplication::setProcessEventsCallback(std::function<void()> callback) {
    processEventsCallback_ = callback;
}

void InviwoApplication::resizePool(size_t newSize) {
    if (newSize == pool_.getSize()) return;

    auto start = std::chrono::system_clock::now();
    std::chrono::milliseconds timeLimit(250);
    auto timeout = [&timeLimit, &start]() {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() -
                                                                start) > timeLimit;
    };

    size_t size = pool_.trySetSize(newSize);
    while (size != newSize && !timeout()) {
        processFront();
        size = pool_.trySetSize(newSize);
    }
    util::OnScopeExit cleanup{[this]() {
        if (poolResizeCallback_) {
            poolResizeCallback_(LongWait::End);
        }
    }};

    if (poolResizeCallback_) {
        poolResizeCallback_(LongWait::Start);
    }

    while (size != newSize) {
        if (timeout()) {
            const auto left = size - newSize;
            log::info("Waiting for {} background thread{} to finish", left, (left > 1 ? "s" : ""));
            timeLimit += std::chrono::milliseconds(1000);
        }

        size = pool_.trySetSize(newSize);
        processFront();
        if (poolResizeCallback_) {
            poolResizeCallback_(LongWait::Update);
        }
    }
}

LayerRamResizer* InviwoApplication::getLayerRamResizer() const { return layerRamResizer_; }
void InviwoApplication::setLayerRamResizer(LayerRamResizer* obj) { layerRamResizer_ = obj; }

void InviwoApplication::setAssertionHandler(
    std::function<void(std::string_view message, SourceContext context)> assertionHandler) {
    assertionHandler_ = std::move(assertionHandler);
}

const std::function<void(std::string_view message, SourceContext context)>&
InviwoApplication::getAssertionHandler() const {
    return assertionHandler_;
}

std::locale InviwoApplication::getUILocale() const { return uiLocale_; }
void InviwoApplication::setUILocale(const std::locale& locale) { uiLocale_ = locale; }

void InviwoApplication::dispatchFrontAndForget(std::function<void()> fun) {
    {
        std::unique_lock<std::mutex> lock(queue_.mutex);
        queue_.tasks.emplace([f = std::move(fun)]() {
            try {
                f();
            } catch (const Exception& e) {
                log::exception(e);
            } catch (const std::exception& e) {
                log::exception(e);
            } catch (...) {
                log::exception();
            }
        });
    }
    if (queue_.postEnqueue) queue_.postEnqueue();
}

size_t InviwoApplication::processFront() {

    while (true) {
        std::function<void()> task;
        {
            const std::unique_lock<std::mutex> lock{queue_.mutex};
            if (queue_.tasks.empty()) break;
            task = std::move(queue_.tasks.front());
            queue_.tasks.pop();
        }
        task();
    }

    const std::unique_lock<std::mutex> lock{queue_.mutex};
    return queue_.tasks.size();
}

void InviwoApplication::setProgressCallback(
    std::function<void(std::string_view)> progressCallback) {
    progressCallback_ = progressCallback;
}

ThreadPool& InviwoApplication::getThreadPool() { return pool_; }

void InviwoApplication::waitForPool() {
    size_t old_size = pool_.getSize();
    resizePool(0);  // This will wait until all tasks are done;
    while (processFront()) {
    }
    resizePool(old_size);
}

TimerThread& InviwoApplication::getTimerThread() {
    if (!timerThread_) {
        timerThread_ = std::make_unique<TimerThread>();
    }
    return *timerThread_;
}

void InviwoApplication::setFileSystemObserver(std::unique_ptr<FileSystemObserver> observer) {
    fileSystemObserver_ = std::move(observer);
}
FileSystemObserver* InviwoApplication::getFileSystemObserver() const {
    return fileSystemObserver_.get();
}

}  // namespace inviwo
