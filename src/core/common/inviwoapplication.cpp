/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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
#include <inviwo/core/common/inviwocore.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/moduleaction.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/metadata/metadatafactory.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/ports/portfactory.h>
#include <inviwo/core/ports/portinspectorfactory.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/propertyconvertermanager.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <inviwo/core/resources/resourcemanager.h>
#include <inviwo/core/util/capabilities.h>
#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/settings/systemsettings.h>

namespace inviwo {
// Helper function to retriever user settings path
void getInviwoUserSettingsPath();

InviwoApplication::InviwoApplication(int argc, char** argv, std::string displayName,
                                     std::string basePath)
    : displayName_(displayName)
    , basePath_(basePath)
    , initialized_(false)
    , nonSupportedTags_()
    , progressCallback_()
    , commandLineParser_(argc, argv)
    , pool_(0)
    , queue_()
    , processorNetwork_{util::make_unique<ProcessorNetwork>()}
    , processorNetworkEvaluator_{
          util::make_unique<ProcessorNetworkEvaluator>(processorNetwork_.get())} {
    if (commandLineParser_.getLogToFile()) {
        LogCentral::getPtr()->registerLogger(
            new FileLogger(commandLineParser_.getLogToFileFileName()));
    }

    init(this);
}

InviwoApplication::InviwoApplication() : InviwoApplication(0, nullptr, "Inviwo", "") {}

InviwoApplication::InviwoApplication(std::string displayName, std::string basePath)
    : InviwoApplication(0, nullptr, displayName, basePath) {}

InviwoApplication::~InviwoApplication() {
    if (initialized_) deinitialize();

    SingletonBase::deleteAllSingeltons();
    DataFormatBase::cleanDataFormatBases();
}

void InviwoApplication::initialize(registerModuleFuncPtr regModuleFunc) {
    printApplicationInfo();
    // initialize singletons
    postProgress("Initializing singletons");

    RenderContext::init();
    ResourceManager::init();
    PickingManager::init();
    
    DataReaderFactory::init();
    dataReaderFactory_ = DataReaderFactory::getPtr();
    
    DataWriterFactory::init();
    dataWriterFactory_ = DataWriterFactory::getPtr();
    
    DialogFactory::init();
    dialogFactory_ = DialogFactory::getPtr();
    
    MeshDrawerFactory::init();
    meshDrawerFactory_ = MeshDrawerFactory::getPtr();
    
    MetaDataFactory::init();
    metaDataFactory_ = MetaDataFactory::getPtr();
    
    PortFactory::init();
    portFactory_ = PortFactory::getPtr();
    
    PortInspectorFactory::init();
    portInspectorFactory_ = PortInspectorFactory::getPtr();
    
    ProcessorFactory::init();
    processorFactory_ = ProcessorFactory::getPtr();
    
    ProcessorWidgetFactory::init();
    processorWidgetFactory_ = ProcessorWidgetFactory::getPtr();
    
    PropertyFactory::init();
    propertyFactory_ = PropertyFactory::getPtr();
    
    PropertyWidgetFactory::init();
    propertyWidgetFactory_ = PropertyWidgetFactory::getPtr();
    
    PropertyConverterManager::init();
    propertyConverterManager_ = PropertyConverterManager::getPtr();
    
    RepresentationConverterFactory::init();
    representationConverterFactory_ = RepresentationConverterFactory::getPtr();
    
    // Create and register core
    InviwoCore* ivwCore = new InviwoCore(this);
    registerModule(ivwCore);

    // Load settings from core
    auto coreSettings = ivwCore->getSettings();
    for (auto setting : coreSettings) setting->loadFromDisk();

    // Create and register other modules
    (*regModuleFunc)(this);

    for (auto& module : modules_) {
        for (auto& elem : module->getCapabilities()) {
            elem->retrieveStaticInfo();
            elem->printInfo();
        }
    }

    // Load settings from other modules
    postProgress("Loading settings...");
    auto settings = getModuleSettings(1);
    for (auto setting : settings) setting->loadFromDisk();

    auto sys = getSettingsByType<SystemSettings>();
    if (sys && !commandLineParser_.getQuitApplicationAfterStartup()) {
        pool_.setSize(static_cast<size_t>(sys->poolSize_.get()));
        sys->poolSize_.onChange(
            [this, sys]() { pool_.setSize(static_cast<size_t>(sys->poolSize_.get())); });
    }

    initialized_ = true;
}

void InviwoApplication::deinitialize() {
    pool_.setSize(0);
    processorNetworkEvaluator_.reset();
    processorNetwork_.reset();
    ResourceManager::getPtr()->clearAllResources();

    moudleCallbackActions_.clear();
    modules_.clear();

    ResourceManager::deleteInstance();
    DataReaderFactory::deleteInstance();
    DataWriterFactory::deleteInstance();
    DialogFactory::deleteInstance();
    MeshDrawerFactory::deleteInstance();
    MetaDataFactory::deleteInstance();
    PickingManager::deleteInstance();
    PortFactory::deleteInstance();
    PortInspectorFactory::deleteInstance();
    ProcessorWidgetFactory::deleteInstance();
    PropertyFactory::deleteInstance();
    PropertyWidgetFactory::deleteInstance();
    PropertyConverterManager::deleteInstance();
    RepresentationConverterFactory::deleteInstance();
    RenderContext::deleteInstance();
    
    dataReaderFactory_ = nullptr;
    dataWriterFactory_ = nullptr;
    dialogFactory_ = nullptr;
    meshDrawerFactory_ = nullptr;
    metaDataFactory_ = nullptr;
    portFactory_ = nullptr;
    portInspectorFactory_ = nullptr;
    processorFactory_ = nullptr;
    processorWidgetFactory_ = nullptr;
    propertyFactory_ = nullptr;
    propertyWidgetFactory_ = nullptr;
    propertyConverterManager_ = nullptr;
    representationConverterFactory_ = nullptr;
    
    initialized_ = false;
}

const std::string& InviwoApplication::getBasePath() const { return basePath_; }

std::string InviwoApplication::getPath(PathType pathType, const std::string& suffix,
                                       const bool& createFolder) {
    std::string result = getBasePath();

    switch (pathType) {
        case inviwo::InviwoApplication::PATH_DATA:
            result += "/data";
            break;

        case inviwo::InviwoApplication::PATH_VOLUMES:
            result += "/data/volumes";
            break;

        case inviwo::InviwoApplication::PATH_MODULES:
            result += "/modules";
            break;

        case inviwo::InviwoApplication::PATH_WORKSPACES:
            result += "/data/workspaces";
            break;

        case inviwo::InviwoApplication::PATH_PORTINSPECTORS:
            result += "/data/workspaces/portinspectors";
            break;

        case inviwo::InviwoApplication::PATH_SCRIPTS:
            result += "/data/scripts";
            break;

        case inviwo::InviwoApplication::PATH_IMAGES:
            result += "/data/images";
            break;

        case inviwo::InviwoApplication::PATH_DATABASES:
            result += "/data/databases";
            break;

        case inviwo::InviwoApplication::PATH_RESOURCES:
            result += "/resources";
            break;

        case inviwo::InviwoApplication::PATH_TRANSFERFUNCTIONS:
            result += "/data/transferfunctions";
            break;

        case inviwo::InviwoApplication::PATH_SETTINGS:
            result = filesystem::getInviwoUserSettingsPath();
            break;

        case inviwo::InviwoApplication::PATH_HELP:
            result += "/data/help";
            break;

        default:
            break;
    }

    if (createFolder) {
        filesystem::createDirectoryRecursively(result);
    }
    return result + suffix;
}

void InviwoApplication::registerModule(InviwoModule* module) {
    postProgress("Loaded module: " + module->getIdentifier());
    modules_.emplace_back(module);
}

const std::vector<std::unique_ptr<InviwoModule>>& InviwoApplication::getModules() const {
    return modules_;
}

ProcessorNetwork* InviwoApplication::getProcessorNetwork() { return processorNetwork_.get(); }

ProcessorNetworkEvaluator* InviwoApplication::getProcessorNetworkEvaluator() {
    return processorNetworkEvaluator_.get();
}

const CommandLineParser* InviwoApplication::getCommandLineParser() const {
    return &commandLineParser_;
}

void InviwoApplication::printApplicationInfo() {
    LogInfoCustom("InviwoInfo", "Inviwo Version: " << IVW_VERSION);
    LogInfoCustom("InviwoInfo", "Base Path: " << getBasePath());
    std::string config = "";
#ifdef CMAKE_GENERATOR
    config += std::string(CMAKE_GENERATOR);
#endif
#if defined(CMAKE_BUILD_TYPE)
    config += " [" + std::string(CMAKE_BUILD_TYPE) + "]";
#elif defined(CMAKE_INTDIR)
    config += " [" + std::string(CMAKE_INTDIR) + "]";
#endif

    if (config != "") LogInfoCustom("InviwoInfo", "Config: " << config);
}

void InviwoApplication::postProgress(std::string progress) {
    if (progressCallback_) progressCallback_(progress);
}

void InviwoApplication::setPostEnqueueFront(std::function<void()> func) {
    queue_.postEnqueue = std::move(func);
}

std::string InviwoApplication::getDisplayName() const { return displayName_; }

void InviwoApplication::addCallbackAction(ModuleCallbackAction* callbackAction) {
    moudleCallbackActions_.emplace_back(callbackAction);
}

std::vector<std::unique_ptr<ModuleCallbackAction>>& InviwoApplication::getCallbackActions() {
    return moudleCallbackActions_;
}

std::vector<Settings*> InviwoApplication::getModuleSettings(size_t startIdx) {
    std::vector<Settings*> allModuleSettings;

    for (size_t i = startIdx; i < modules_.size(); i++) {
        auto modSettings = modules_[i]->getSettings();
        allModuleSettings.insert(allModuleSettings.end(), modSettings.begin(), modSettings.end());
    }

    return allModuleSettings;
}

void InviwoApplication::addNonSupportedTags(const Tags t) {
    for (auto& elem : t.tags_) {
        nonSupportedTags_.addTag(elem);
    }
}

bool InviwoApplication::checkIfAllTagsAreSupported(const Tags t) const {
    return (nonSupportedTags_.getMatches(t) == 0);
}

std::locale InviwoApplication::getUILocale() const {
    return std::locale();
}

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
    processFront();
    pool_.setSize(0);  // This will wait until all tasks are done;
    processFront();
    pool_.setSize(old_size);
}

}  // namespace
