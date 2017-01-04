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
#include <inviwo/core/common/inviwomodulelibraryobserver.h>
#include <inviwo/core/common/moduleaction.h>
#include <inviwo/core/common/version.h>
#include <inviwo/core/datastructures/camerafactory.h>
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
#include <inviwo/core/util/sharedlibrary.h>
#include <inviwo/core/util/systemcapabilities.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/utilities.h>

#include <locale>
#include <codecvt>
#include <string>
namespace inviwo {

InviwoApplication::InviwoApplication(int argc, char** argv, std::string displayName)
    : displayName_(displayName)
    , binaryPath_(filesystem::getFileDirectory(argv[0]))
    , progressCallback_()
    , commandLineParser_(argc, argv)
    , pool_(0, []() {}, []() { RenderContext::getPtr()->clearContext(); })
    , queue_()
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
    , representationConverterMetaFactory_{util::make_unique<RepresentationConverterMetaFactory>()}

    , modules_()
    , clearModules_([&]() {
        // Note: be careful when changing the order of clearModules
        // as modules need to be removed before factories for example.
        ResourceManager::getPtr()->clearAllResources();
        // Need to clear the modules in reverse order since the might depend on each other.
        // The destruction order of vector is undefined.
        for (auto it = std::rbegin(modules_); it != std::rend(modules_);) {
            // Erase does not take reverse_iterator so we need to convert it
            it = std::vector<std::unique_ptr<InviwoModule>>::reverse_iterator(modules_.erase((++it).base()));
        }
        modulesFactoryObjects_.clear();
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
    registerModule(std::move(ivwCore));

    auto sys = getSettingsByType<SystemSettings>();
    if (sys && !commandLineParser_.getQuitApplicationAfterStartup()) {
        resizePool(static_cast<size_t>(sys->poolSize_.get()));

        sys->poolSize_.onChange([this, sys]() { 
            resizePool(static_cast<size_t>(sys->poolSize_.get()));
        });
    }
    // Make sure that all data formats are initialized in this library.
    // Need to be done when libraries are loaded at runtime since the
    // data format may be used first in one of the loaded libraries
    // but will not be cleaned up when the module is unloaded.
#define DataFormatIdMacro(i) {const DataFormatBase* dummyInitialization =  Data##i::get();}
#include <inviwo/core/util/formatsdefinefunc.h>
}

InviwoApplication::InviwoApplication() : InviwoApplication(0, nullptr, "Inviwo") {}

InviwoApplication::InviwoApplication(std::string displayName)
    : InviwoApplication(0, nullptr, displayName) {}

InviwoApplication::~InviwoApplication() {
    resizePool(0);
    portInspectorFactory_->clearCache();
    ResourceManager::getPtr()->clearAllResources();
}

void InviwoApplication::unregisterModules() {
    onModulesWillUnregister_.invoke();
    getProcessorNetwork()->clear();
    ResourceManager::getPtr()->clearAllResources();
    // Need to clear the modules in reverse order since the might depend on each other.
    // The destruction order of vector is undefined.
    auto protectedModules = getProtectedModuleIdentifiers();
    for (auto it = std::rbegin(modules_); it != std::rend(modules_);) {
        if (protectedModules.find((*it)->getIdentifier()) == protectedModules.end()) {
            // Erase does not take reverse_iterator so we need to convert it
            it = decltype(it)(modules_.erase((++it).base()));
        } else {
            ++it;
        }
    }

    // Remove module factories
    util::erase_remove_if(modulesFactoryObjects_,
        [&](const auto& module) { return protectedModules.find(module->name) == protectedModules.end(); });
    
    // Modules should now have removed all allocated resources and it should be safe to unload shared libraries. 
    util::erase_remove_if(moduleSharedLibraries_,
        [&](const auto& module) {
        // Figure out module identifier from file name
        auto moduleName = util::stripModuleFileNameDecoration(module->getFilePath());

        return std::find_if(std::begin(protectedModules), std::end(protectedModules),
            [moduleName](const auto& protectedModule) {
            return toLower(protectedModule).compare(moduleName) == 0;
        }) == protectedModules.end();
    });

}

void InviwoApplication::registerModules(std::vector<std::unique_ptr<InviwoModuleFactoryObject>> moduleFactories) {
    printApplicationInfo();
    for(auto& mod: moduleFactories) {
        modulesFactoryObjects_.emplace_back(std::move(mod));
    }
    
    // Topological sort to make sure that we load modules in correct order
    topologicalModuleFactoryObjectSort(std::begin(modulesFactoryObjects_), std::end(modulesFactoryObjects_));


    std::vector<std::string> failed;
    auto checkdepends = [&](const std::vector<std::string>& deps) {
        for (const auto& dep : deps) {
            auto it = util::find(failed, dep);
            if (it != failed.end()) return it;
        }
        return failed.end();
    };

    auto checkDepencyVersion = [&](const std::string& moduleName, const std::string& depVersions) {
        std::map<std::string, std::string> incorrectDepencencyVersions;
        auto lowerCaseDep = toLower(moduleName);
        // Find module
        auto it = util::find_if(modulesFactoryObjects_, [&](const std::unique_ptr<InviwoModuleFactoryObject>& module) {
            return toLower(module->name) == lowerCaseDep;
        });
        // Check if dependent module is of correct version
        if (it != modulesFactoryObjects_.end() && Version((*it)->version) == Version(depVersions)) {
            return true;
        }
        else {
            return false;
        };
    };

    for (auto& moduleObj : modulesFactoryObjects_) {
        if (util::contains_if(modules_, [&](const auto& module) { return module->getIdentifier().compare(moduleObj->name) == 0; })) {
            continue;
        }
        postProgress("Loading module: " + moduleObj->name);
        // Make sure that the module supports the current inviwo core version
        if (Version(IVW_VERSION) != Version(moduleObj->inviwoCoreVersion)) {
            LogError("Failed to register module: " + moduleObj->name);
            LogError("Reason: Module was built for Inviwo version " + moduleObj->inviwoCoreVersion + ", current version is " + IVW_VERSION);
            util::push_back_unique(failed, toLower(moduleObj->name));
            continue;
        }
        // Check if dependency modules have correct versions.
        // Note that the module version only need to be increased 
        // when changing and the inviwo core version has not changed
        // since we are ensuring the they must be built for the 
        // same core version. 
        auto versionIt = moduleObj->dependenciesVersion.cbegin();
        std::stringstream dependencyVersionError;
        for (auto dep = moduleObj->dependencies.cbegin();
            dep != moduleObj->dependencies.end(); ++dep, ++versionIt) {
            if (!checkDepencyVersion(*dep, *versionIt)) {
                // Find module
                auto name = *dep;
                auto dependencyIt = util::find_if(modulesFactoryObjects_, [name](const std::unique_ptr<InviwoModuleFactoryObject>& module) {
                    return toLower(module->name) == name;
                });
                if (dependencyIt != modulesFactoryObjects_.end()) {
                    dependencyVersionError << "Module depends on " + *dep + " version " << *versionIt << " but version " << (*dependencyIt)->version << " was loaded" << std::endl;
                }
                else {
                    dependencyVersionError << "Module depends on " + *dep + " version " << *versionIt << " but no such module was loaded" << std::endl;
                }
            };
        }
        if (dependencyVersionError.str().size() > 0) {
            LogError("Failed to register module: " + moduleObj->name);
            LogError("Reason: " + dependencyVersionError.str());
            util::push_back_unique(failed, toLower(moduleObj->name));
            continue;
        }
        try {
            auto it = checkdepends(moduleObj->dependencies);
            if (it == failed.end()) {
                registerModule(moduleObj->create(this));
                LogInfo("Registered module: " + moduleObj->name);
            }
            else {
                LogError("Could not register module: " + moduleObj->name + " since dependency: " +
                    *it + " failed to register");
            }
        }
        catch (const ModuleInitException& e) {
            LogError("Failed to register module: " + moduleObj->name);
            LogError("Reason: " + e.getMessage());
            util::push_back_unique(failed, toLower(moduleObj->name));

            std::vector<std::string> toDeregister;
            for (const auto& m : e.getModulesToDeregister()) {
                util::append(toDeregister, findDependentModules(m));
                toDeregister.push_back(toLower(m));
            }
            for (const auto& dereg : toDeregister) {
                util::erase_remove_if(modules_, [&](const std::unique_ptr<InviwoModule>& m) {
                    if (toLower(m->getIdentifier()) == dereg) {
                        LogError("De-registering " + m->getIdentifier() + " because " +
                            moduleObj->name + " failed to register");
                        return true;
                    }
                    else {
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

    onModulesDidRegister_.invoke();
}

void InviwoApplication::registerModules(
    const std::vector<std::string>& librarySearchPaths, bool reloadLibrariesWhenChanged) {
    // Perform the following steps
    // 1. Recursively get all library files and the folders they are in
    // 2. Sort them according to protected modules (done by std::set).
    //    Prevents dependent modules from being loaded from temporary directory.
    //    Only necessary when libraries can be reloaded.
    // 3. Load libraries and see if createModule function exist.
    // 4. Start observing file if reloadLibrariesWhenChanged
    // 5. Pass module factories to registerModules

    std::vector<std::unique_ptr<InviwoModuleFactoryObject>> modules;

    auto protectedModules = getProtectedModuleIdentifiers();
    // Returns the iterator to the protected module, or end() if not found
    auto findProtectedModule = [&protectedModules](const std::string& moduleIdentifier) {
        return std::find_if(
            std::begin(protectedModules), std::end(protectedModules),
            [moduleIdentifier](const auto& protectedModule) {
            return toLower(protectedModule).compare(toLower(moduleIdentifier)) == 0;
        });
    };
    auto orderByProtectedModule = [&](std::string first, std::string second)->bool {
        auto foundFirst = findProtectedModule(util::stripModuleFileNameDecoration(first));
        auto foundSecond = findProtectedModule(util::stripModuleFileNameDecoration(second));
        if (foundFirst == protectedModules.end() && foundSecond == protectedModules.end()) {
            return first < second;
        }
        else {
            return std::distance(protectedModules.begin(), foundFirst) <
                std::distance(protectedModules.begin(), foundSecond);
        }
    };
    // Load protected modules first, necessary when library reloading is enabled 
    // since dependent modules can exist in both temporary and application directory.
    // The modules which protected modules depend on will be loaded from the application 
    // directory instead of the temporary directory.
    // Note: OpenGL module will fail to load if OpenGLQt is enabled and sorting is removed.
    std::set<std::string, decltype(orderByProtectedModule)> files(orderByProtectedModule); // Recursively found libraries
    std::set<std::string> libraryDirectories; // Recursively found directories
#if WIN32
    // only consider files with dll extension
    std::set<std::string> libraryTypes{ "dll" }; 
    // Prevent error mode dialogs from displaying.
    SetErrorMode(SEM_FAILCRITICALERRORS);
    // Get AddDllDirectory function.
    // This function is only available after installing KB2533623
    typedef DLL_DIRECTORY_COOKIE(WINAPI * addDllDirectory_func)(PCWSTR);
    addDllDirectory_func lpfnAdllDllDirectory = (addDllDirectory_func)GetProcAddress(
        GetModuleHandle(TEXT("kernel32.dll")), "AddDllDirectory");
    // Store added dll search directories so that we can remove them 
    std::vector<DLL_DIRECTORY_COOKIE> addedSearchDirectories;
#else
    // only consider files with so, dylib or bundle extension
    std::set<std::string> libraryTypes{ "so", "dylib", "bundle" }; 
#endif
    // Find unique files and directories in specified search paths
    for (auto path : librarySearchPaths) {
        auto filesInPath = getDirectoryContentsRecursively(path, filesystem::ListMode::Files);
        auto directories = getDirectoryContentsRecursively(path, filesystem::ListMode::Directories);
        files.insert(
            std::make_move_iterator(filesInPath.begin()),
            std::make_move_iterator(filesInPath.end())
        );
        libraryDirectories.insert(
            std::make_move_iterator(directories.begin()),
            std::make_move_iterator(directories.end())
        );
    }
    // Determines if a library is already loaded into the application
    auto isModuleLibraryLoaded = [&](const std::string path) {
        return moduleSharedLibraries_.end() !=
            std::find_if(
                std::begin(moduleSharedLibraries_),
                std::end(moduleSharedLibraries_),
                [path](const auto& lib) {
            return lib->getFilePath().compare(path) == 0;
        });
    };
    // Remove unsupported files and files belonging to already loaded modules.
    // Erase-remove idiom can't be used with std::set so we have to loop
    for (auto it = files.begin(); it != files.end();) {
        if (libraryTypes.find(filesystem::getFileExtension(*it)) == libraryTypes.end() ||
            it->find("inviwo-module") == std::string::npos ||
            isModuleLibraryLoaded(*it)) {
            files.erase(it++);
        } else {
            ++it;
        }
    }

    // Libraries are copied to a temporary folder in the case of runtime re-loading.
    // Otherwise, tmpSharedLibraryFiles == originalLibraryFiles
    std::vector<std::string> tmpSharedLibraryFiles;
    std::vector<std::string> originalLibraryFiles;
    std::string tmpLibraryDir =
        filesystem::getInviwoApplicationPath() + "/temporary-module-libraries";

    if (reloadLibrariesWhenChanged
#if WIN32
        // AdllDllDirectory must be supported on windows
        && lpfnAdllDllDirectory
#endif
        ) {

        if (!filesystem::directoryExists(tmpLibraryDir)) {
            filesystem::createDirectoryRecursively(tmpLibraryDir);
        }
        for (const auto& filePath : files) {

            std::string dstPath =
                tmpLibraryDir + "/" + filesystem::getFileNameWithExtension(filePath);
            if (protectedModules.end() !=
                findProtectedModule(util::stripModuleFileNameDecoration(filePath))) {
                // Protected modules are loaded from the application dir
                dstPath = filePath;
            } else {
                if (filesystem::fileModificationTime(filePath) !=
                    filesystem::fileModificationTime(dstPath)) {
                    // Load a copy of the file to make sure that
                    // we can overwrite the file.
                    filesystem::copyFile(filePath, dstPath);
                }
            }

            tmpSharedLibraryFiles.emplace_back(dstPath);
            originalLibraryFiles.emplace_back(filePath);
        }
#if WIN32
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        addedSearchDirectories.emplace_back(
            AddDllDirectory(converter.from_bytes(tmpLibraryDir).c_str()));
#endif
    } else {
        // Libraries will not be reloaded, we can use the original files
        std::copy(std::begin(files), std::end(files), std::back_inserter(originalLibraryFiles));
        std::copy(std::begin(files), std::end(files), std::back_inserter(tmpSharedLibraryFiles));
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

#if WIN32
        if (lpfnAdllDllDirectory) {
            // Add search paths to find module dll dependencies
            for (auto searchPath : librarySearchPaths) {
                addedSearchDirectories.emplace_back(
                    AddDllDirectory(converter.from_bytes(searchPath).c_str()));
            }
            // Also add recursively found directories
            for (auto searchPath : libraryDirectories) {
                addedSearchDirectories.emplace_back(
                    AddDllDirectory(converter.from_bytes(searchPath).c_str()));
            }
        }

#endif
    }
    // Load libraries from temporary directory 
    // but observe the original file
    auto filePathIt = originalLibraryFiles.cbegin();
    for (auto tmpFilePathIt = tmpSharedLibraryFiles.cbegin();
         tmpFilePathIt != tmpSharedLibraryFiles.end(); ++tmpFilePathIt, ++filePathIt) {
        auto filePath = *filePathIt;
        auto tmpPath = *tmpFilePathIt;
        try {
            // Load library. Will throw exception if failed to load
            auto sharedLib =
                std::unique_ptr<SharedLibrary>(new SharedLibrary(tmpPath));
            // Only consider libraries with Inviwo module creation function
            if (auto moduleFunc = (f_getModule)sharedLib->findSymbol("createModule")) {
                // Add module factory object
                modules.emplace_back(moduleFunc());
                auto moduleName = toLower(modules.back()->name);
                moduleSharedLibraries_.emplace_back(std::move(sharedLib));
                if (reloadLibrariesWhenChanged) {
                    // Start observing file 
                    if (!moduleLibraryObserver_) {
                        // We cannot create the observer in the constructor since
                        // deriving applications will implement observer behavior
                        // and they will not have been created when InviwoApplication
                        // constructor is called.
                        moduleLibraryObserver_ =
                            std::unique_ptr<InviwoModuleLibraryObserver>(new InviwoModuleLibraryObserver());
                    }
                    if (!moduleLibraryObserver_->isObserved(filePath)) {
                        moduleLibraryObserver_->startFileObservation(filePath);
                    }
                }
            }
        } catch (Exception ex) {
            // Library dependency is probably missing.
            // We silently skip this library. 
            LogInfo("Could not load library: " + filePath);
        }
    }
#if WIN32
    // Remove added search paths 
    for (const auto& dir : addedSearchDirectories) {
        RemoveDllDirectory(dir);
    }
#endif
    for (const auto& mod : modules) {
        LogInfo("Found module: " + mod->name);
    }
    registerModules(std::move(modules));
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

std::set<std::string> InviwoApplication::getProtectedModuleIdentifiers() const {
    // Core:      Statically linked and should not be unloaded
    return std::set<std::string>{"Core"};
}

void InviwoApplication::printApplicationInfo() {
    auto caps = this->getModuleByType<InviwoCore>()->getCapabilities();
    
    LogInfoCustom("InviwoInfo", "Inviwo Version: " << IVW_VERSION);
    if (auto syscap = getTypeFromVector<SystemCapabilities>(caps)) {
        if (syscap->getBuildTimeYear() != 0) {
            LogInfoCustom("InviwoInfo", "Build Date: " << syscap->getBuildDateString());
        }
    }
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

const std::string& InviwoApplication::getDisplayName() const { return displayName_; }

const std::string& InviwoApplication::getBinaryPath() const { return binaryPath_; }

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

void InviwoApplication::cleanupSingletons() {
    PickingManager::deleteInstance();
    ResourceManager::deleteInstance();
    RenderContext::deleteInstance();
}

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
        if (util::contains(item->dependencies, module)) {
           auto name = toLower(item->name);
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

std::shared_ptr<std::function<void()>> InviwoApplication::onModulesDidRegister(std::function<void()> callback) {
    return onModulesDidRegister_.add(callback);
}

std::shared_ptr<std::function<void()>> InviwoApplication::onModulesWillUnregister(std::function<void()> callback) {
    return onModulesWillUnregister_.add(callback);
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
