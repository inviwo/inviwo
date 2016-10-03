/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2016 Inviwo Foundation
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
#include <inviwo/core/datastructures/camerafactory.h>
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

InviwoApplication::InviwoApplication(int argc, char** argv, std::string displayName)
    : displayName_(displayName)
    , progressCallback_()
    , commandLineParser_(argc, argv)
    , pool_(0, []() {}, []() { RenderContext::getPtr()->clearContext(); })
    , queue_()
    , clearDataFormats_{[]() { DataFormatBase::cleanDataFormatBases(); }}
    , clearAllSingeltons_{[this]() { cleanupSingletons(); }}

    , cameraFactory_{util::make_unique<CameraFactory>()}
    , dataReaderFactory_{util::make_unique<DataReaderFactory>()}
    , dataWriterFactory_{util::make_unique<DataWriterFactory>()}
    , dialogFactory_{util::make_unique<DialogFactory>()}
    , meshDrawerFactory_{util::make_unique<MeshDrawerFactory>()}
    , metaDataFactory_{util::make_unique<MetaDataFactory>()}
    , outportFactory_{util::make_unique<OutportFactory>()}
    , inportFactory_{util::make_unique<InportFactory>()}
    , portInspectorFactory_{util::make_unique<PortInspectorFactory>()}
    , processorFactory_{util::make_unique<ProcessorFactory>()}
    , processorWidgetFactory_{util::make_unique<ProcessorWidgetFactory>()}
    , propertyConverterManager_{util::make_unique<PropertyConverterManager>()}
    , propertyFactory_{util::make_unique<PropertyFactory>()}
    , propertyWidgetFactory_{util::make_unique<PropertyWidgetFactory>()}
    , representationConverterFactory_{util::make_unique<RepresentationConverterFactory>()}

    , modules_()
    , clearModules_([&]() {
        ResourceManager::getPtr()->clearAllResources();
        // Need to clear the modules in reverse order since the might depend on each other.
        // The destuction order of vector is undefined.
        for (auto it = modules_.rbegin(); it != modules_.rend(); it++) {
            it->reset();
        }
        modules_.clear();
    })
    , moudleCallbackActions_()

    , processorNetwork_{util::make_unique<ProcessorNetwork>(this)}
    , processorNetworkEvaluator_{
          util::make_unique<ProcessorNetworkEvaluator>(processorNetwork_.get())} {
    if (commandLineParser_.getLogToFile()) {
        auto filename = commandLineParser_.getLogToFileFileName();
        auto dir = filesystem::getFileDirectory(filename);
        if (dir.empty() || !filesystem::directoryExists(dir)){
            filename = commandLineParser_.getOutputPath() + "/" + filename;
        }
        filelogger_ = std::make_shared<FileLogger>(filename);
        LogCentral::getPtr()->registerLogger(filelogger_);
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
        resizePool(static_cast<size_t>(sys->poolSize_.get()));

        sys->poolSize_.onChange([this, sys]() { 
            resizePool(static_cast<size_t>(sys->poolSize_.get()));
        });
    }
}

InviwoApplication::InviwoApplication() : InviwoApplication(0, nullptr, "Inviwo") {}

InviwoApplication::InviwoApplication(std::string displayName)
    : InviwoApplication(0, nullptr, displayName) {}

InviwoApplication::~InviwoApplication() {
    resizePool(0);
    portInspectorFactory_->clearCache();
    ResourceManager::getPtr()->clearAllResources();
}

void topologicalSort(std::vector<std::unique_ptr<InviwoModuleFactoryObject>>& graph,
                      std::unordered_set<std::string>& explored, std::string i,
                      std::vector<std::string>& sorted, size_t& t) {
    auto it =
        std::find_if(std::begin(graph), std::end(graph),
                     [&](const std::unique_ptr<InviwoModuleFactoryObject>& a) {
                        // Lower case comparison
                         return a->name_.size() == i.size() &&
                                equal(i.cbegin(), i.cend(), a->name_.cbegin(),
                                      [](std::string::value_type l1, std::string::value_type r1) {
                                          return ::tolower(l1) == ::tolower(r1);
                                      });
                     });
    if (it == std::end(graph)) {
        // This dependency has not been loaded
        return;
    }
    auto nodeName = (*it)->name_;
    std::transform(nodeName.begin(), nodeName.end(), nodeName.begin(), ::tolower);
    explored.insert(nodeName);

    for (const auto& dependency : (*it)->depends_) {
        auto lowerCaseDependency = dependency;
        std::transform(lowerCaseDependency.begin(), lowerCaseDependency.end(),
                       lowerCaseDependency.begin(), ::tolower);
        if (explored.find(lowerCaseDependency) == explored.end()) {
            topologicalSort(graph, explored, lowerCaseDependency, sorted, t);
        }
    }

    --t;
    sorted[t] = i;

    return;
}
void InviwoApplication::registerModules(RegisterModuleFunc regModuleFunc) {
    printApplicationInfo();

    // Create and register other modules
    modulesFactoryObjects_ = regModuleFunc();
    // Topological sort to make sure that we load modules in correct order
    // https://en.wikipedia.org/wiki/Topological_sorting#Depth-first_search
    std::unordered_set<std::string> explored;
    auto t = modulesFactoryObjects_.size();
    std::vector<std::string> sorted(modulesFactoryObjects_.size());
    for (const auto& module : modulesFactoryObjects_) {
        auto lowerCaseName = module->name_;
        std::transform(lowerCaseName.begin(), lowerCaseName.end(), lowerCaseName.begin(),
                       ::tolower);
        if (explored.find(lowerCaseName) == explored.end()) {
            topologicalSort(modulesFactoryObjects_, explored, lowerCaseName, sorted, t);
        }
    }
    // Sort modules according to dependency graph
    std::sort(std::begin(modulesFactoryObjects_), std::end(modulesFactoryObjects_),
              [&](const std::unique_ptr<InviwoModuleFactoryObject>& a,
                  const std::unique_ptr<InviwoModuleFactoryObject>& b) {
                  auto lowerCaseNameA = a->name_;
                  std::transform(lowerCaseNameA.begin(), lowerCaseNameA.end(),
                                 lowerCaseNameA.begin(), ::tolower);
                  auto lowerCaseNameB = b->name_;
                  std::transform(lowerCaseNameB.begin(), lowerCaseNameB.end(),
                                 lowerCaseNameB.begin(), ::tolower);
                  return std::find(std::begin(sorted), std::end(sorted), lowerCaseNameA) >
                         std::find(std::begin(sorted), std::end(sorted), lowerCaseNameB);
              });

    std::vector<std::string> failed;
    auto checkdepends = [&](const std::vector<std::string>& deps) {
        for (const auto& dep : deps) {
            auto it = util::find(failed, dep);
            if (it != failed.end()) return it;
        }
        return failed.end();
    };

    auto checkVersionCompability = [](const std::string referenceVersion, const std::string toCompare) {
        istringstream refSS(referenceVersion);
        std::string refMajor, refMinor, refPatch;
        std::getline(refSS, refMajor, '.'); std::getline(refSS, refMinor, '.'); std::getline(refSS, refPatch, '.');
        istringstream toCompSS(toCompare);
        std::string toCompMajor, toCompMinor, toCompPatch;
        std::getline(toCompSS, toCompMajor, '.');
        std::getline(toCompSS, toCompMinor, '.');
        std::getline(toCompSS, toCompPatch, '.');
        // Major and minor versions need to be the same.
        if (refMajor != toCompMajor || refMinor != toCompMinor) {
            return false;
        }
        else {
            return true;
        }
    };

    auto checkDepencyVersion = [&](const std::string& dep, const std::string& depVersions) {
        std::map<std::string, std::string> incorrectDepencencyVersions;
        auto lowerCaseDep = dep;
        std::transform(lowerCaseDep.begin(), lowerCaseDep.end(),
            lowerCaseDep.begin(), ::tolower);
        // Find module
        auto it = util::find_if(modulesFactoryObjects_, [&](const std::unique_ptr<InviwoModuleFactoryObject>& module) {
            auto lowerCaseNameA = module->name_;
            std::transform(lowerCaseNameA.begin(), lowerCaseNameA.end(),
                lowerCaseNameA.begin(), ::tolower);
            return lowerCaseNameA == lowerCaseDep;
        });
        // Check if dependent module is of correct version
        if (it != modulesFactoryObjects_.end() && checkVersionCompability((*it)->version_, depVersions)) {
            return true;
        }
        else {
            return false;
        };
    };


    for (auto& moduleObj : modulesFactoryObjects_) {
        postProgress("Loading module: " + moduleObj->name_);
        // Make sure that the module supports the current inviwo core version
        if (checkVersionCompability(IVW_VERSION, moduleObj->inviwoCoreVersion_)) {
            LogError("Failed to register module: " + moduleObj->name_);
            LogError("Reason: Module was built for Inviwo version " + moduleObj->inviwoCoreVersion_ + ", current version is " + IVW_VERSION);
            util::push_back_unique(failed, toLower(moduleObj->name_));
            continue;
        }
        // Check if dependency modules have correct versions.
        // Note that the module version only need to be increased 
        // when changing and the inviwo core version has not changed
        // since we are ensuring the they must be built for the 
        // same core version. 
        auto versionIt = moduleObj->dependenciesVersion_.cbegin();
        auto anyIncorrectDependencyVersions = false;
        std::stringstream dependencyVersionError;
        for (auto dep = moduleObj->depends_.cbegin();
            dep != moduleObj->depends_.end(); ++dep, ++versionIt) {
            if (!checkDepencyVersion(*dep, *versionIt)) {                
                dependencyVersionError << "Module depends on " + *dep + " version " << *versionIt << " but another version was loaded" << std::endl;
            };
        }
        if (dependencyVersionError.str().size() > 0) {
            LogError("Failed to register module: " + moduleObj->name_);
            LogError("Reason: " + dependencyVersionError.str());
            util::push_back_unique(failed, toLower(moduleObj->name_));
            continue;
        }
        try {
            auto it = checkdepends(moduleObj->depends_);
            if (it == failed.end()) {
                registerModule(moduleObj->create(this));
            } else {
                LogError("Could not register module: " + moduleObj->name_ + " since dependency: " +
                         *it + " failed to register");
            }
        } catch (const ModuleInitException& e) {
            LogError("Failed to register module: " + moduleObj->name_);
            LogError("Reason: " + e.getMessage());
            util::push_back_unique(failed, toLower(moduleObj->name_));

            std::vector<std::string> toDeregister;
            for (const auto& m : e.getModulesToDeregister()) {
                util::append(toDeregister, findDependentModules(m));
                toDeregister.push_back(toLower(m));
            }
            for (const auto& dereg : toDeregister) {
                util::erase_remove_if(modules_, [&](const std::unique_ptr<InviwoModule>& m) {
                    if (toLower(m->getIdentifier()) == dereg) {
                        LogError("De-registering " + m->getIdentifier() + " because " +
                                 moduleObj->name_ + " failed to register");
                        return true;
                    } else {
                        return false;
                    }
                });
                util::push_back_unique(failed, dereg);
            }
        }
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

std::string InviwoApplication::getBasePath() const { return filesystem::findBasePath(); }

std::string InviwoApplication::getPath(PathType pathType, const std::string& suffix,
                                       const bool& createFolder) {
    return filesystem::getPath(pathType, suffix, createFolder);
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

InviwoModule* InviwoApplication::getModuleByIdentifier(const std::string& identifier) const {
    const auto it = std::find_if(
        modules_.begin(), modules_.end(),
        [&](const std::unique_ptr<InviwoModule>& m) { return m->getIdentifier() == identifier; });
    if (it != modules_.end()) {
        return it->get();
    } else {
        return nullptr;
    }
}

ProcessorNetwork* InviwoApplication::getProcessorNetwork() { return processorNetwork_.get(); }

ProcessorNetworkEvaluator* InviwoApplication::getProcessorNetworkEvaluator() {
    return processorNetworkEvaluator_.get();
}

const CommandLineParser& InviwoApplication::getCommandLineParser() const {
    return commandLineParser_;
}

CommandLineParser& InviwoApplication::getCommandLineParser() {
    return commandLineParser_;
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

void InviwoApplication::cleanupSingletons() { SingletonBase::deleteAllSingeltons(); }

void InviwoApplication::resizePool(size_t newSize) {
    size_t size = pool_.trySetSize(newSize);
    while (size != newSize) {
        size = pool_.trySetSize(newSize);
        processFront();
    }
}

std::vector<std::string> InviwoApplication::findDependentModules(std::string module) const {
    std::vector<std::string> dependencies;
    for (const auto& item : modulesFactoryObjects_) {
        if (util::contains(item->depends_, module)) {
           auto name = toLower(item->name_);
           auto deps = findDependentModules(name);
           util::append(dependencies, deps);
           dependencies.push_back(name);
        }
    }
    std::vector<std::string> unique;
    for(const auto& item : dependencies) {
        util::push_back_unique(unique, item);
    }
    return unique;
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

void InviwoApplication::closeInviwoApplication() {
    LogWarn("this application have not implemented the closeInviwoApplication function");
}
void InviwoApplication::registerFileObserver(FileObserver* fileObserver) {
    LogWarn("this application have not implemented the registerFileObserver function");
}
void InviwoApplication::unRegisterFileObserver(FileObserver* fileObserver) {
    LogWarn("this application have not implemented the unRegisterFileObserver function");
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

InteractionStateManager& InviwoApplication::getInteractionStateManager() {
    return interactionState_;
}

}  // namespace
