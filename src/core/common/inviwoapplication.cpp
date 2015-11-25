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

InviwoApplication::InviwoApplication(int argc, char** argv, std::string displayName,
                                     std::string basePath)
    : displayName_(displayName)
    , basePath_(basePath)
    , nonSupportedTags_()
    , progressCallback_()
    , commandLineParser_(argc, argv)
    , pool_(0)
    , queue_()

    , clearDataFormats_{[]() { DataFormatBase::cleanDataFormatBases(); }}
    , clearAllSingeltons_{[this]() { cleanupSingletons(); }}

    , dataReaderFactory_{util::make_unique<DataReaderFactory>()}
    , dataWriterFactory_{util::make_unique<DataWriterFactory>()}
    , dialogFactory_{util::make_unique<DialogFactory>()}
    , meshDrawerFactory_{util::make_unique<MeshDrawerFactory>()}
    , metaDataFactory_{util::make_unique<MetaDataFactory>()}
    , portFactory_{util::make_unique<PortFactory>()}
    , portInspectorFactory_{util::make_unique<PortInspectorFactory>()}
    , processorFactory_{util::make_unique<ProcessorFactory>()}
    , processorWidgetFactory_{util::make_unique<ProcessorWidgetFactory>()}
    , propertyConverterManager_{util::make_unique<PropertyConverterManager>()}
    , propertyFactory_{util::make_unique<PropertyFactory>()}
    , propertyWidgetFactory_{util::make_unique<PropertyWidgetFactory>()}
    , representationConverterFactory_{util::make_unique<RepresentationConverterFactory>()}

    , modules_()
    , moudleCallbackActions_()

    , processorNetwork_{util::make_unique<ProcessorNetwork>()}
    , processorNetworkEvaluator_{
          util::make_unique<ProcessorNetworkEvaluator>(processorNetwork_.get())} {
    if (commandLineParser_.getLogToFile()) {
        LogCentral::getPtr()->registerLogger(
            new FileLogger(commandLineParser_.getLogToFileFileName()));
    }

    init(this);

    // initialize singletons
    RenderContext::init();
    ResourceManager::init();
    PickingManager::init();

    // Create and register core
    auto ivwCore = util::make_unique<InviwoCore>(this);
    // Load settings from core
    auto coreSettings = ivwCore->getSettings();
    registerModule(std::move(ivwCore));
    
    for (auto setting : coreSettings) setting->loadFromDisk();
    auto sys = getSettingsByType<SystemSettings>();
    if (sys && !commandLineParser_.getQuitApplicationAfterStartup()) {
        pool_.setSize(static_cast<size_t>(sys->poolSize_.get()));
        sys->poolSize_.onChange(
            [this, sys]() { pool_.setSize(static_cast<size_t>(sys->poolSize_.get())); });
    }
}

InviwoApplication::InviwoApplication() : InviwoApplication(0, nullptr, "Inviwo", "") {}

InviwoApplication::InviwoApplication(std::string displayName, std::string basePath)
    : InviwoApplication(0, nullptr, displayName, basePath) {}

InviwoApplication::~InviwoApplication() {
    pool_.setSize(0);
    ResourceManager::getPtr()->clearAllResources();
}

void InviwoApplication::registerModules(RegisterModuleFunc regModuleFunc) {
    printApplicationInfo();

    // Create and register other modules
    modulesFactoryObjects_ = regModuleFunc();

    for (auto& moduleObj : modulesFactoryObjects_) {
        postProgress("Loading module: " + moduleObj->name_);
        registerModule(moduleObj->create(this));
    }
    
    postProgress("Loading Capabilities");
    for (auto& module : modules_) {
        for (auto& elem : module->getCapabilities()) {
            elem->retrieveStaticInfo();
            elem->printInfo();
        }
    }

    // Load settings from other modules
    postProgress("Loading Settings");
    auto settings = getModuleSettings(1);
    for (auto setting : settings) setting->loadFromDisk();
}

const std::string& InviwoApplication::getBasePath() const { return basePath_; }

std::string InviwoApplication::getPath(PathType pathType, const std::string& suffix,
                                       const bool& createFolder) {
    std::string result = getBasePath();

    switch (pathType) {
        case PathType::Data:
            result += "/data";
            break;

        case PathType::Volumes:
            result += "/data/volumes";
            break;

        case PathType::Modules:
            result += "/modules";
            break;

        case PathType::Workspaces:
            result += "/data/workspaces";
            break;

        case PathType::PortInspectors:
            result += "/data/workspaces/portinspectors";
            break;

        case PathType::Scripts:
            result += "/data/scripts";
            break;

        case PathType::Images:
            result += "/data/images";
            break;

        case PathType::Databases:
            result += "/data/databases";
            break;

        case PathType::Resources:
            result += "/resources";
            break;

        case PathType::TransferFunctions:
            result += "/data/transferfunctions";
            break;

        case PathType::Settings:
            result = filesystem::getInviwoUserSettingsPath();
            break;

        case PathType::Help:
            result += "/data/help";
            break;

        case PathType::Tests:
            result += "/tests";
            break;

        default:
            break;
    }

    if (createFolder) {
        filesystem::createDirectoryRecursively(result);
    }
    return result + suffix;
}

void InviwoApplication::registerModule(std::unique_ptr<InviwoModule> module) {
    modules_.push_back(std::move(module));
}

const std::vector<std::unique_ptr<InviwoModule>>& InviwoApplication::getModules() const {
    return modules_;
}

const std::vector<std::unique_ptr<InviwoModuleFactoryObject>>&
InviwoApplication::getModuleFactoryObjects() const {
    return modulesFactoryObjects_;
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
void InviwoApplication::cleanupSingletons() { SingletonBase::deleteAllSingeltons(); }
bool InviwoApplication::checkIfAllTagsAreSupported(const Tags t) const {
    return (nonSupportedTags_.getMatches(t) == 0);
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
    processFront();
    pool_.setSize(0);  // This will wait until all tasks are done;
    processFront();
    pool_.setSize(old_size);
}

void InviwoApplication::closeInviwoApplication() {
    LogWarn("this application have not implemented the closeInviwoApplication function");
}
void InviwoApplication::registerFileObserver(FileObserver* fileObserver) {
    LogWarn("this application have not implemented the registerFileObserver function");
}
void InviwoApplication::startFileObservation(std::string fileName) {
    LogWarn("this application have not implemented the startFileObservation function");
}
void InviwoApplication::stopFileObservation(std::string fileName) {
    LogWarn("this application have not implemented the stopFileObservation function");
}
void InviwoApplication::playSound(Message soundID) {
    LogWarn("this application have not implemented the playSound function");
}

}  // namespace
