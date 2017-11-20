/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2017 Inviwo Foundation
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
#include <inviwo/core/datastructures/camerafactory.h>
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
#include <inviwo/core/util/capabilities.h>
#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/systemcapabilities.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/core/util/filelogger.h>
#include <inviwo/core/util/timer.h>
#include <inviwo/core/util/settings/systemsettings.h>

namespace inviwo {

namespace util {
/*
 * Make sure that all data formats are initialized.
 * Should only be called in the core library, i.e. in InviwoApplication constructor.
 *
 * Need to be done when libraries are loaded at runtime since the
 * data format may be used first in one of the loaded libraries
 * but will not be cleaned up when the module is unloaded.
 *
 */
void dataFormatDummyInitialization() {
#include <warn/push>
#include <warn/ignore/unused-variable>
#define DataFormatIdMacro(i) \
    { const DataFormatBase* dummyInitialization = Data##i::get(); }
#include <inviwo/core/util/formatsdefinefunc.h>
#include <warn/pop>
}

}  // namespace util

InviwoApplication::InviwoApplication(int argc, char** argv, std::string displayName)
    : displayName_(displayName)
    , progressCallback_()
    , commandLineParser_(argc, argv)
    , pool_(0, []() {}, []() { RenderContext::getPtr()->clearContext(); })
    , queue_()
    , clearAllSingeltons_{[]() {
        PickingManager::deleteInstance();
        RenderContext::deleteInstance();
    }}
    , cameraFactory_{util::make_unique<CameraFactory>()}
    , dataReaderFactory_{util::make_unique<DataReaderFactory>()}
    , dataWriterFactory_{util::make_unique<DataWriterFactory>()}
    , dialogFactory_{util::make_unique<DialogFactory>()}
    , meshDrawerFactory_{util::make_unique<MeshDrawerFactory>()}
    , metaDataFactory_{util::make_unique<MetaDataFactory>()}
    , outportFactory_{util::make_unique<OutportFactory>()}
    , inportFactory_{util::make_unique<InportFactory>()}
    , portInspectorFactory_{util::make_unique<PortInspectorFactory>()}
    , processorFactory_{util::make_unique<ProcessorFactory>(this)}
    , processorWidgetFactory_{util::make_unique<ProcessorWidgetFactory>()}
    , propertyConverterManager_{util::make_unique<PropertyConverterManager>()}
    , propertyFactory_{util::make_unique<PropertyFactory>()}
    , propertyWidgetFactory_{util::make_unique<PropertyWidgetFactory>()}
    , representationConverterMetaFactory_{util::make_unique<RepresentationConverterMetaFactory>()}
    , systemSettings_{std::make_unique<SystemSettings>(this)}
    , systemCapabilities_{std::make_unique<SystemCapabilities>()}
    , moduleManager_{this}
    , moudleCallbackActions_()
    , processorNetwork_{util::make_unique<ProcessorNetwork>(this)}
    , processorNetworkEvaluator_{util::make_unique<ProcessorNetworkEvaluator>(
          processorNetwork_.get())}
    , workspaceManager_{util::make_unique<WorkspaceManager>(this)}
    , propertyPresetManager_{util::make_unique<PropertyPresetManager>()}
    , portInspectorManager_{util::make_unique<PortInspectorManager>(this)} {

    if (commandLineParser_.getLogToConsole()) {
        consoleLogger_ = std::make_shared<ConsoleLogger>();
        LogCentral::getPtr()->registerLogger(consoleLogger_);
    }

    if (commandLineParser_.getLogToFile()) {
        auto filename = commandLineParser_.getLogToFileFileName();
        auto dir = filesystem::getFileDirectory(filename);
        if (dir.empty() || !filesystem::directoryExists(dir)) {
            if (!commandLineParser_.getOutputPath().empty()) {
                filename = commandLineParser_.getOutputPath() + "/" + filename;
            }
        }
        filelogger_ = std::make_shared<FileLogger>(filename);
        LogCentral::getPtr()->registerLogger(filelogger_);
    }

    // Keep the pool at size 0 if are quiting directly to make sure that we don't have unfinished
    // results in the worker threads
    if (!commandLineParser_.getQuitApplicationAfterStartup()) {
        resizePool(systemSettings_->poolSize_);
        systemSettings_->poolSize_.onChange([this]() { resizePool(systemSettings_->poolSize_); });
    }

    // initialize singletons
    init(this);
    RenderContext::init();
    PickingManager::init();

    workspaceManager_->registerFactory(getProcessorFactory());
    workspaceManager_->registerFactory(getMetaDataFactory());
    workspaceManager_->registerFactory(getPropertyFactory());
    workspaceManager_->registerFactory(getInportFactory());
    workspaceManager_->registerFactory(getOutportFactory());

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
    //
    // Need to be done when libraries are loaded at runtime since the
    // data format may be used first in one of the loaded libraries
    // but will not be cleaned up when the module is unloaded.
    util::dataFormatDummyInitialization();
}

InviwoApplication::InviwoApplication() : InviwoApplication(0, nullptr, "Inviwo") {}

InviwoApplication::InviwoApplication(std::string displayName)
    : InviwoApplication(0, nullptr, displayName) {}

InviwoApplication::~InviwoApplication() { resizePool(0); }

void InviwoApplication::registerModules(
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>> moduleFactories) {
    moduleManager_.registerModules(std::move(moduleFactories));
}

void InviwoApplication::registerModules(RuntimeModuleLoading token) {
    moduleManager_.registerModules(token);
}

std::string InviwoApplication::getBasePath() const { return filesystem::findBasePath(); }

std::string InviwoApplication::getPath(PathType pathType, const std::string& suffix,
                                       const bool& createFolder) {
    return filesystem::getPath(pathType, suffix, createFolder);
}

ModuleManager& InviwoApplication::getModuleManager() { return moduleManager_; }

const ModuleManager& InviwoApplication::getModuleManager() const { return moduleManager_; }

const std::vector<std::unique_ptr<InviwoModule>>& InviwoApplication::getModules() const {
    return moduleManager_.getModules();
}

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

const CommandLineParser& InviwoApplication::getCommandLineParser() const {
    return commandLineParser_;
}

CommandLineParser& InviwoApplication::getCommandLineParser() { return commandLineParser_; }

void InviwoApplication::printApplicationInfo() {
    LogInfoCustom("InviwoInfo", "Inviwo Version: " << IVW_VERSION);
    if (systemCapabilities_->getBuildTimeYear() != 0) {
        LogInfoCustom("InviwoInfo", "Build Date: " << systemCapabilities_->getBuildDateString());
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
    systemCapabilities_->printInfo();
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
    moudleCallbackActions_.emplace_back(callbackAction);
}

std::vector<std::unique_ptr<ModuleCallbackAction>>& InviwoApplication::getCallbackActions() {
    return moudleCallbackActions_;
}

std::vector<Settings*> InviwoApplication::getModuleSettings() {
    std::vector<Settings*> allModuleSettings;
    allModuleSettings.push_back(systemSettings_.get());
    for (auto& module : moduleManager_.getModules()) {
        auto modSettings = module->getSettings();
        allModuleSettings.insert(allModuleSettings.end(), modSettings.begin(), modSettings.end());
    }
    return allModuleSettings;
}

SystemSettings& InviwoApplication::getSystemSettings() { return *systemSettings_; }

std::vector<Capabilities*> InviwoApplication::getModuleCapabilities() {
    std::vector<Capabilities*> allModuleCapabilities;
    allModuleCapabilities.push_back(systemCapabilities_.get());
    for (auto& module : moduleManager_.getModules()) {
        auto modCapabilities = module->getCapabilities();
        allModuleCapabilities.insert(allModuleCapabilities.end(), modCapabilities.begin(),
                                     modCapabilities.end());
    }
    return allModuleCapabilities;
}

SystemCapabilities& InviwoApplication::getSystemCapabilities() { return *systemCapabilities_; }

void InviwoApplication::resizePool(size_t newSize) {
    if (newSize == pool_.getSize()) return;
    size_t size = pool_.trySetSize(newSize);
    while (size != newSize) {
        size = pool_.trySetSize(newSize);
        processFront();
    }
}

UsageMode InviwoApplication::getApplicationUsageMode() const {
    return systemSettings_->applicationUsageMode_;
}

void InviwoApplication::setApplicationUsageMode(UsageMode mode) {
    systemSettings_->applicationUsageMode_.set(mode);
}

std::locale InviwoApplication::getUILocale() const { return std::locale(); }

void InviwoApplication::processFront() {
    NetworkLock netlock(processorNetwork_.get());
    std::function<void()> task;
    while (true) {
        {
            std::unique_lock<std::mutex> lock{queue_.mutex};
            if (queue_.tasks.empty()) return;
            task = std::move(queue_.tasks.front());
            queue_.tasks.pop();
        }
        task();
    }
}

void InviwoApplication::setProgressCallback(std::function<void(std::string)> progressCallback) {
    progressCallback_ = progressCallback;
}

void InviwoApplication::waitForPool() {
    size_t old_size = pool_.getSize();
    resizePool(0);  // This will wait until all tasks are done;
    processFront();
    resizePool(old_size);
}

TimerThread& InviwoApplication::getTimerThread() {
    if (!timerThread_) {
        timerThread_ = util::make_unique<TimerThread>();
    }
    return *timerThread_;
}

void InviwoApplication::closeInviwoApplication() {
    LogWarn("this application have not implemented the closeInviwoApplication function");
}
void InviwoApplication::registerFileObserver(FileObserver*) {
    LogWarn("this application have not implemented the registerFileObserver function");
}
void InviwoApplication::unRegisterFileObserver(FileObserver*) {}
void InviwoApplication::startFileObservation(std::string) {
    LogWarn("this application have not implemented the startFileObservation function");
}
void InviwoApplication::stopFileObservation(std::string) {}
void InviwoApplication::playSound(Message) {
    LogWarn("this application have not implemented the playSound function");
}

namespace util {

InviwoApplication* getInviwoApplication() { return InviwoApplication::getPtr(); }

InviwoApplication* getInviwoApplication(ProcessorNetwork* network) {
    return network ? network->getApplication() : nullptr;
}

InviwoApplication* getInviwoApplication(Processor* processor) {
    return processor ? getInviwoApplication(processor->getNetwork()) : nullptr;
}

InviwoApplication* getInviwoApplication(Property* property) {
    return property ? getInviwoApplication(property->getOwner()) : nullptr;
}

InviwoApplication* getInviwoApplication(PropertyOwner* owner) {
    return owner ? owner->getInviwoApplication() : nullptr;
}

}  // namespace util

}  // namespace inviwo
