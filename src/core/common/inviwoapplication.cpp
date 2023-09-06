/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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
#include <inviwo/core/inviwocommondefines.h>
#include <inviwo/core/datastructures/camera/camerafactory.h>
#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/metadata/metadatafactory.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/network/processornetworkevaluator.h>
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

#include <inviwo/core/resourcemanager/resourcemanagerobserver.h>

#include <chrono>

namespace inviwo {

struct AppResourceManagerObserver : ResourceManagerObserver {
    AppResourceManagerObserver(SystemSettings* settings, ResourceManager* manager)
        : settings{settings}, manager{manager} {

        manager->addObserver(this);
    }

    virtual void onResourceManagerEnableStateChanged() override {
        settings->enableResourceManager_.set(manager->isEnabled());
    }

    SystemSettings* settings = nullptr;
    ResourceManager* manager = nullptr;
};

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
          0, []() {}, []() { RenderContext::getPtr()->clearContext(); })
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
    , systemSettings_{std::make_unique<SystemSettings>(this)}
    , resourcemanagerobserver_{std::make_unique<AppResourceManagerObserver>(systemSettings_.get(),
                                                                            resourceManager_.get())}
    , moduleCallbackActions_{}
    , moduleManager_{this}
    , processorNetwork_{std::make_unique<ProcessorNetwork>(this)}
    , processorNetworkEvaluator_{std::make_unique<ProcessorNetworkEvaluator>(
          processorNetwork_.get())}
    , workspaceManager_{std::make_unique<WorkspaceManager>(this)}
    , propertyPresetManager_{std::make_unique<PropertyPresetManager>(this)}
    , portInspectorManager_{std::make_unique<PortInspectorManager>(this)}
    , layerRamResizer_{nullptr} {

    // Keep the pool at size 0 if are quiting directly to make sure that we don't have
    // unfinished results in the worker threads
    if (!commandLineParser_->getQuitApplicationAfterStartup()) {
        resizePool(systemSettings_->poolSize_);
        systemSettings_->poolSize_.onChange([this]() { resizePool(systemSettings_->poolSize_); });
    }

    resourceManager_->setEnabled(systemSettings_->enableResourceManager_.get());
    systemSettings_->enableResourceManager_.onChange(
        [this]() { resourceManager_->setEnabled(systemSettings_->enableResourceManager_.get()); });
    if (commandLineParser_->getDisableResourceManager()) {
        resourceManager_->setEnabled(false);
    }

    moduleManager_.onModulesDidRegister([this]() {
        if (resourceManager_->isEnabled() && resourceManager_->numberOfResources() > 0) {
            LogWarn(
                "Resource manager was not empty when reloading modules. The behavior of resources "
                "of data structures that has changed is undefined and may effect stability");
        }
    });

    // initialize singletons
    init(this);
    RenderContext::init();
    PickingManager::init();

    workspaceManager_->registerFactory(getProcessorFactory());
    workspaceManager_->registerFactory(getMetaDataFactory());
    workspaceManager_->registerFactory(getPropertyFactory());
    workspaceManager_->registerFactory(getInportFactory());
    workspaceManager_->registerFactory(getOutportFactory());
    workspaceManager_->registerFactory(getCameraFactory());

    networkClearHandle_ = workspaceManager_->onClear([&]() {
        portInspectorManager_->clear();
        processorNetwork_->clear();
    });
    networkSerializationHandle_ = workspaceManager_->onSave([&](Serializer& s) {
        s.serialize("ProcessorNetwork", *processorNetwork_);
        s.serialize("PortInspectors", *portInspectorManager_);
    });
    networkDeserializationHandle_ = workspaceManager_->onLoad([&](Deserializer& d) {
        d.deserialize("ProcessorNetwork", *processorNetwork_);
        d.deserialize("PortInspectors", *portInspectorManager_);
    });

    presetsClearHandle_ =
        workspaceManager_->onClear([&]() { propertyPresetManager_->clearWorkspacePresets(); });
    presetsSerializationHandle_ = workspaceManager_->onSave(
        [&](Serializer& s) { propertyPresetManager_->saveWorkspacePresets(s); });
    presetsDeserializationHandle_ = workspaceManager_->onLoad(
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
    moduleManager_.registerModules(std::move(moduleFactories));
}

void InviwoApplication::registerModules(RuntimeModuleLoading token) {
    moduleManager_.registerModules(token);
}

void InviwoApplication::registerModules(RuntimeModuleLoading token,
                                        std::function<bool(std::string_view)> isEnabled) {
    moduleManager_.registerModules(token, isEnabled);
}

std::filesystem::path InviwoApplication::getBasePath() const { return filesystem::findBasePath(); }

std::filesystem::path InviwoApplication::getPath(PathType pathType, const std::string& suffix,
                                                 const bool& createFolder) {
    return filesystem::getPath(pathType, suffix, createFolder);
}

ModuleManager& InviwoApplication::getModuleManager() { return moduleManager_; }

const ModuleManager& InviwoApplication::getModuleManager() const { return moduleManager_; }

InviwoModule* InviwoApplication::getModuleByIdentifier(const std::string& identifier) const {
    return moduleManager_.getModuleByIdentifier(identifier);
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
    LogInfoCustom("InviwoInfo", "Inviwo Version: " << build::version);
    if (auto buildInfo = util::getBuildInfo()) {
        LogInfoCustom("InviwoInfo", "Build Date: " << buildInfo->getDate());
    }
    LogInfoCustom("InviwoInfo", "Base Path: " << getBasePath());
    LogInfoCustom("InviwoInfo", "ThreadPool Worker Threads: " << pool_.getSize());

    std::string config = "";
#ifdef CMAKE_GENERATOR
    config += std::string(CMAKE_GENERATOR);
#endif
#if defined(CMAKE_BUILD_TYPE)
    config += " [" + std::string(CMAKE_BUILD_TYPE) + "]";
#elif defined(CMAKE_INTDIR)
    config += " [" + std::string(CMAKE_INTDIR) + "]";
#endif
    if (!config.empty()) {
        LogInfoCustom("InviwoInfo", "Config: " << config);
    }
}

void InviwoApplication::postProgress(std::string progress) {
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

std::vector<Settings*> InviwoApplication::getModuleSettings() {
    std::vector<Settings*> allModuleSettings;
    allModuleSettings.push_back(systemSettings_.get());
    for (auto& inviwoModule : moduleManager_.getInviwoModules()) {
        const auto& modSettings = inviwoModule.getSettings();
        allModuleSettings.insert(allModuleSettings.end(), modSettings.begin(), modSettings.end());
    }
    return allModuleSettings;
}

SystemSettings& InviwoApplication::getSystemSettings() { return *systemSettings_; }

std::vector<Capabilities*> InviwoApplication::getModuleCapabilities() {
    std::vector<Capabilities*> allModuleCapabilities;
    for (auto& inviwoModule : moduleManager_.getInviwoModules()) {
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
            auto left = size - newSize;
            LogInfo("Waiting for " << left << " background thread" << (left > 1 ? "s" : "")
                                   << " to finish");
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

std::locale InviwoApplication::getUILocale() const { return uiLocale_; }
void InviwoApplication::setUILocale(const std::locale& locale) { uiLocale_ = locale; }

void InviwoApplication::dispatchFrontAndForget(std::function<void()> fun) {
    {
        std::unique_lock<std::mutex> lock(queue_.mutex);
        queue_.tasks.push(std::move(fun));
    }
    if (queue_.postEnqueue) queue_.postEnqueue();
}

size_t InviwoApplication::processFront() {
    {
        NetworkLock netlock(processorNetwork_.get());
        std::function<void()> task;
        while (true) {
            {
                std::unique_lock<std::mutex> lock{queue_.mutex};
                if (queue_.tasks.empty()) break;
                task = std::move(queue_.tasks.front());
                queue_.tasks.pop();
            }
            task();
        }
    }
    std::unique_lock<std::mutex> lock{queue_.mutex};
    return queue_.tasks.size();
}

void InviwoApplication::setProgressCallback(std::function<void(std::string)> progressCallback) {
    progressCallback_ = progressCallback;
}

ThreadPool& InviwoApplication::getThreadPool() { return pool_; }

void InviwoApplication::waitForPool() {
    size_t old_size = pool_.getSize();
    resizePool(0);  // This will wait until all tasks are done;
    while (processFront())
        ;
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
