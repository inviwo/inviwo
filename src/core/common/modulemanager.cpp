/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/inviwomodulelibraryobserver.h>
#include <inviwo/core/common/version.h>
#include <inviwo/core/resources/resourcemanager.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/sharedlibrary.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/capabilities.h>
#include <inviwo/core/network/processornetwork.h>

#include <locale>
#include <codecvt>
#include <string>
#include <functional>

#ifdef WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace inviwo {

ModuleManager::ModuleManager(InviwoApplication* app)
    : app_{app}, modules_(), clearModules_([&]() {
        // Note: be careful when changing the order of clearModules
        // as modules need to be removed before factories for example.
        ResourceManager::getPtr()->clearAllResources();
        // Need to clear the modules in reverse order since the might depend on each other.
        // The destruction order of vector is undefined.
        for (auto it = std::rbegin(modules_); it != std::rend(modules_);) {
            // Erase does not take reverse_iterator so we need to convert it
            it = std::vector<std::unique_ptr<InviwoModule>>::reverse_iterator(
                modules_.erase((++it).base()));
        }
        factoryObjects_.clear();
    }) {}

ModuleManager::~ModuleManager() = default;

bool ModuleManager::isRuntimeModuleReloadingEnabled() {
    if (auto sys = app_->getSettingsByType<SystemSettings>()) {
        return sys->runtimeModuleReloading_.get();
    }
    return false;
}

void ModuleManager::registerModules(std::vector<std::unique_ptr<InviwoModuleFactoryObject>> mfo) {
    app_->printApplicationInfo();
    factoryObjects_.insert(factoryObjects_.end(), std::make_move_iterator(mfo.begin()),
                           std::make_move_iterator(mfo.end()));

    // Topological sort to make sure that we load modules in correct order
    topologicalModuleFactoryObjectSort(std::begin(factoryObjects_), std::end(factoryObjects_));

    for (auto& obj : factoryObjects_) {
        if (getModuleByIdentifier(obj->name)) continue;  // already loaded
        app_->postProgress("Loading module: " + obj->name);

        try {
            if (checkDependencies(*obj)) {
                registerModule(obj->create(app_));
            }
        } catch (const ModuleInitException& e) {
            LogError("Failed to register module: " << obj->name << ". Reason:\n" << e.getMessage());

            std::vector<std::string> toDeregister;
            for (const auto& m : e.getModulesToDeregister()) {
                util::append(toDeregister, findDependentModules(m));
                toDeregister.push_back(toLower(m));
            }
            for (const auto& dereg : toDeregister) {
                util::erase_remove_if(modules_, [&](const std::unique_ptr<InviwoModule>& m) {
                    if (toLower(m->getIdentifier()) == dereg) {
                        LogError("Deregistering " << m->getIdentifier() << " because " << obj->name
                                                  << " failed to register");
                        return true;
                    } else {
                        return false;
                    }
                });
            }
        }
    }

    app_->postProgress("Loading Capabilities");
    for (auto& module : modules_) {
        for (auto& elem : module->getCapabilities()) {
            elem->retrieveStaticInfo();
            elem->printInfo();
        }
    }

    onModulesDidRegister_.invoke();
}

std::function<bool(const std::string&)> ModuleManager::getEnabledFilter() {
    // Load enabled modules if file "application_name-enabled-modules.txt" exists,
    // otherwise load all modules
    const auto enabledModuleFileName =
        filesystem::getFileNameWithoutExtension(filesystem::getExecutablePath()) +
        "-enabled-modules.txt";
    const auto exepath = filesystem::getFileDirectory(filesystem::getExecutablePath());
#ifdef __APPLE__
    // Executable path is inviwo.app/Content/MacOs
    std::string enabledModulesFilePath(exepath + "/../../../" + enabledModuleFileName);
#else
    std::string enabledModulesFilePath(exepath + "/" + enabledModuleFileName);
#endif
    if (filesystem::fileExists(enabledModulesFilePath)) {
        return [](const std::string&) { return true; };
    }

    std::ifstream enabledModulesFile{enabledModulesFilePath};
    std::vector<std::string> enabledModules;
    std::copy(std::istream_iterator<std::string>(enabledModulesFile),
              std::istream_iterator<std::string>(), std::back_inserter(enabledModules));
    std::for_each(std::begin(enabledModules), std::end(enabledModules), toLower);

    return [=](const std::string& file) {
        const auto name = util::stripModuleFileNameDecoration(file);
        return util::contains(enabledModules, name);
    };
}

namespace {
#if WIN32
bool hasAddDllFunc() {
    // Get AddDllDirectory function.
    // This function is only available after installing KB2533623
    using addDllDirectory_func = DLL_DIRECTORY_COOKIE(WINAPI*)(PCWSTR);
    auto lpfnAdllDllDirectory = reinterpret_cast<addDllDirectory_func>(
        GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "AddDllDirectory"));
    return lpfnAdllDllDirectory != nullptr;
}

std::vector<DLL_DIRECTORY_COOKIE> addDllDirs(const std::vector<std::string>& dirs) {
    std::vector<DLL_DIRECTORY_COOKIE> addedSearchDirectories;
    // Prevent error mode dialogs from displaying.
    SetErrorMode(SEM_FAILCRITICALERRORS);
    if (hasAddDllFunc()) {
        // Add search paths to find module dll dependencies
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        for (auto searchPath : dirs) {
            searchPath = filesystem::cleanupPath(searchPath);
            replaceInString(searchPath, "/", "\\");
            const auto path = converter.from_bytes(searchPath);
            const auto dlldir = AddDllDirectory(path.c_str());
            if (dlldir) {
                addedSearchDirectories.emplace_back(dlldir);
            } else {
                LogWarnCustom("ModuleManager",
                              "Could not get AddDllDirectory for path " << searchPath);
            }
        }
    }
    return addedSearchDirectories;
}
void removeDllDirs(const std::vector<DLL_DIRECTORY_COOKIE>& dirs) {
    if (hasAddDllFunc()) {
        for (const auto& dir : dirs) {
            RemoveDllDirectory(dir);
        }
    }
}
// only consider files with dll extension
std::set<std::string> libTypes() { return {"dll"}; }
#else
// only consider files with so, dylib or bundle extension
std::set<std::string> libTypes() { return {"so", "dylib", "bundle"}; }
// dummy functions
int addDllDirs(const std::vector<std::string>&) { return 0; }
void removeDllDirs(const int&) {}
bool hasAddDllFunc() { return true; }
#endif
}  // namespace

void ModuleManager::registerModules(const std::vector<std::string>& librarySearchPaths) {
    // Perform the following steps
    // 1. Recursively get all library files and the folders they are in
    // 2. Sort them according to protected modules (done by std::set).
    //    Prevents dependent modules from being loaded from temporary directory.
    //    Only necessary when libraries can be reloaded.
    // 3. Filter out files with correct extension, named inviwo-module
    //    and listed in application_name-enabled-modules.txt (if it exist).
    // 4. Load libraries and see if createModule function exist.
    // 5. Start observing file if reloadLibrariesWhenChanged
    // 6. Pass module factories to registerModules

    auto orderByProtectedModule = [&](std::string first, std::string second) -> bool {
        auto foundFirst = protected_.find(util::stripModuleFileNameDecoration(first));
        auto foundSecond = protected_.find(util::stripModuleFileNameDecoration(second));
        if (foundFirst == protected_.end() && foundSecond == protected_.end()) {
            return iCaseLess(first, second);
        } else {
            return std::distance(protected_.begin(), foundFirst) <
                   std::distance(protected_.begin(), foundSecond);
        }
    };
    // Load protected modules first, necessary when library reloading is enabled
    // since dependent modules can exist in both temporary and application directory.
    // The modules which protected modules depend on will be loaded from the application
    // directory instead of the temporary directory.
    // Note: OpenGL module will fail to load if OpenGLQt is enabled and sorting is removed.
    // Recursively found libraries:
    std::set<std::string, decltype(orderByProtectedModule)> libraryFiles(orderByProtectedModule);
    // Recursively found directories:
    std::set<std::string> libraryDirectories;

    // Find unique files and directories in specified search paths
    for (auto path : librarySearchPaths) {
        // Make sure that we have an absolute path to avoid duplicates
        path = filesystem::cleanupPath(path);
        try {
            auto files =
                filesystem::getDirectoryContentsRecursively(path, filesystem::ListMode::Files);
            auto dirs = filesystem::getDirectoryContentsRecursively(
                path, filesystem::ListMode::Directories);
            libraryFiles.insert(std::make_move_iterator(files.begin()),
                                std::make_move_iterator(files.end()));
            libraryDirectories.insert(std::make_move_iterator(dirs.begin()),
                                      std::make_move_iterator(dirs.end()));
        } catch (FileException&) {  // Invalid path, ignore it
        }
    }
    // Determines if a library is already loaded into the application
    auto isModuleLibraryLoaded = [&](const std::string path) {
        return util::contains_if(sharedLibraries_, [path](const auto& lib) {
            return lib->getFilePath().compare(path) == 0;
        });
    };
    // Load enabled modules if file "application_name-enabled-modules.txt" exists,
    // otherwise load all modules
    auto isEnabled = getEnabledFilter();
    auto libraryTypes = libTypes();
    // Remove unsupported files and files belonging to already loaded modules.
    util::map_erase_remove_if(libraryFiles, [&](const auto& file) {
        return libraryTypes.count(filesystem::getFileExtension(file)) == 0 ||
               file.find("inviwo-module") == std::string::npos || isModuleLibraryLoaded(file) ||
               !isEnabled(file);
    });

    // Libraries are copied to a temporary folder in the case of runtime re-loading.
    // libFilePairs first = original
    // libFilePairs second = temp
    // Otherwise, temp = original
    std::vector<std::pair<std::string, std::string>> libFilePairs;
    const auto tmpLibraryDir = filesystem::getInviwoUserCachePath() + "/temporary-module-libraries";
    std::vector<std::string> searchDirectories;
    if (isRuntimeModuleReloadingEnabled() && hasAddDllFunc()) {
        if (!filesystem::directoryExists(tmpLibraryDir)) {
            filesystem::createDirectoryRecursively(tmpLibraryDir);
        }
        for (const auto& filePath : libraryFiles) {
            auto dstPath = tmpLibraryDir + "/" + filesystem::getFileNameWithExtension(filePath);
            if (isProtected(util::stripModuleFileNameDecoration(filePath))) {
                // Protected modules are loaded from the application dir
                dstPath = filePath;
            } else if (filesystem::fileModificationTime(filePath) !=
                       filesystem::fileModificationTime(dstPath)) {
                // Load a copy of the file to make sure that
                // we can overwrite the file.
                filesystem::copyFile(filePath, dstPath);
            }
            libFilePairs.emplace_back(filePath, dstPath);
        }
        searchDirectories.push_back(tmpLibraryDir);
    } else {
        // Libraries will not be reloaded, we can use the original files
        for (const auto& filePath : libraryFiles) {
            libFilePairs.emplace_back(filePath, filePath);
        }
    }
    util::append(searchDirectories, librarySearchPaths, libraryDirectories);
    auto addedSearchDirectories = addDllDirs(searchDirectories);

    if (!libraryObserver_ && isRuntimeModuleReloadingEnabled()) {
        // We cannot create the observer in the constructor since
        // deriving applications will implement observer behavior
        // and they will not have been created when InviwoApplication
        // constructor is called.
        libraryObserver_ = util::make_unique<InviwoModuleLibraryObserver>();
    }

    // Load libraries from temporary directory
    // but observe the original file
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>> modules;
    for (const auto& item : libFilePairs) {
        const auto& filePath = item.first;
        const auto& tmpPath = item.second;
        try {
            // Load library. Will throw exception if failed to load
            auto sharedLib = util::make_unique<SharedLibrary>(tmpPath);
            // Only consider libraries with Inviwo module creation function
            if (auto moduleFunc = sharedLib->findSymbolTyped<f_getModule>("createModule")) {
                // Add module factory object
                modules.emplace_back(moduleFunc());
                auto moduleName = toLower(modules.back()->name);
                sharedLibraries_.emplace_back(std::move(sharedLib));
                if (isRuntimeModuleReloadingEnabled() && !libraryObserver_->isObserved(filePath)) {
                    libraryObserver_->startFileObservation(filePath);
                }
            } else {
                LogInfo(
                    "Could not find 'createModule' function needed for creating the module in "
                    << tmpPath
                    << ". Make sure that you have compiled the library and exported the function.");
            }
        } catch (Exception ex) {
            // Library dependency is probably missing.
            // We silently skip this library.
            LogInfo("Could not load library: " + filePath);
        }
    }
    // Remove added search paths
    removeDllDirs(addedSearchDirectories);

    registerModules(std::move(modules));
}

void ModuleManager::unregisterModules() {
    onModulesWillUnregister_.invoke();
    app_->getProcessorNetwork()->clear();
    ResourceManager::getPtr()->clearAllResources();
    // Need to clear the modules in reverse order since the might depend on each other.
    // The destruction order of vector is undefined.
    for (auto it = std::rbegin(modules_); it != std::rend(modules_);) {
        if (!isProtected((*it)->getIdentifier())) {
            // Erase does not take reverse_iterator so we need to convert it
            it = decltype(it)(modules_.erase((++it).base()));
        } else {
            ++it;
        }
    }

    // Remove module factories
    util::erase_remove_if(factoryObjects_,
                          [this](const auto& mfo) { return !this->isProtected(mfo->name); });

    // Modules should now have removed all allocated resources and it should be safe to unload
    // shared libraries.
    util::erase_remove_if(sharedLibraries_, [this](const auto& module) {
        // Figure out module identifier from file name
        auto moduleName = util::stripModuleFileNameDecoration(module->getFilePath());
        return !this->isProtected(moduleName);
    });
}

void ModuleManager::registerModule(std::unique_ptr<InviwoModule> module) {
    modules_.push_back(std::move(module));
}

const std::vector<std::unique_ptr<InviwoModule>>& ModuleManager::getModules() const {
    return modules_;
}

const std::vector<std::unique_ptr<InviwoModuleFactoryObject>>&
ModuleManager::getModuleFactoryObjects() const {
    return factoryObjects_;
}

InviwoModule* ModuleManager::getModuleByIdentifier(const std::string& identifier) const {
    const auto it =
        std::find_if(modules_.begin(), modules_.end(), [&](const std::unique_ptr<InviwoModule>& m) {
            return iCaseCmp(m->getIdentifier(), identifier);
        });
    if (it != modules_.end()) {
        return it->get();
    } else {
        return nullptr;
    }
}

std::vector<InviwoModule*> ModuleManager::getModulesByAlias(const std::string& alias) const {
    std::vector<InviwoModule*> res;
    for (const auto& mfo : factoryObjects_) {
        if (util::contains(mfo->aliases, alias)) {
            if (auto m = getModuleByIdentifier(mfo->name)) {
                res.push_back(m);
            }
        }
    }
    return res;
}

InviwoModuleFactoryObject* ModuleManager::getFactoryObject(const std::string& identifier) const {
    auto it = util::find_if(factoryObjects_,
                            [&](const auto& module) { return iCaseCmp(module->name, identifier); });
    // Check if dependent module is of correct version
    if (it != factoryObjects_.end()) {
        return it->get();
    } else {
        return nullptr;
    }
}

std::vector<std::string> ModuleManager::findDependentModules(const std::string& module) const {
    std::vector<std::string> dependencies;
    for (const auto& item : factoryObjects_) {
        if (util::contains_if(item->dependencies, [&](auto& dep) { return dep.first == module; })) {
            auto name = toLower(item->name);
            auto deps = findDependentModules(name);
            util::append(dependencies, deps);
            dependencies.push_back(name);
        }
    }
    std::vector<std::string> unique;
    for (const auto& item : dependencies) {
        util::push_back_unique(unique, item);
    }
    return unique;
}

std::shared_ptr<std::function<void()>> ModuleManager::onModulesDidRegister(
    std::function<void()> callback) {
    return onModulesDidRegister_.add(callback);
}

std::shared_ptr<std::function<void()>> ModuleManager::onModulesWillUnregister(
    std::function<void()> callback) {
    return onModulesWillUnregister_.add(callback);
}

const std::set<std::string, CaseInsensitiveCompare>& ModuleManager::getProtectedModuleIdentifiers()
    const {
    return protected_;
}

bool ModuleManager::isProtected(const std::string& module) const {
    return protected_.count(module) != 0;
}

void ModuleManager::addProtectedIdentifier(const std::string& id) { protected_.insert(id); }

bool ModuleManager::checkDependencies(const InviwoModuleFactoryObject& obj) const {
    std::stringstream err;

    // Make sure that the module supports the current inviwo core version
    if (!Version(IVW_VERSION).semanticVersionEqual(obj.inviwoCoreVersion)) {
        err << "\nModule was built for Inviwo version " << obj.inviwoCoreVersion
            << ", current version is " << IVW_VERSION;
    }

    // Check if dependency modules have correct versions.
    // Note that the module version only need to be increased
    // when changing and the inviwo core version has not changed
    // since we are ensuring the they must be built for the
    // same core version.
    for (const auto& dep : obj.dependencies) {
        const auto& name = dep.first;
        const auto& version = dep.second;

        if (auto depObj = getFactoryObject(name)) {
            if (!getModuleByIdentifier(depObj->name)) {
                err << "\nModule dependency: " + depObj->name + " failed to register";
            } else if (!depObj->version.semanticVersionEqual(obj.version)) {
                err << "\nModule depends on " << depObj->name << " version " << version
                    << " but version " << depObj->version << " was loaded";
            }
        } else {
            err << "\nModule depends on " << name << " version " << version
                << " but no such module was found";
        }
    }
    if (err.str().size() > 0) {
        LogError("Failed to register module: " << obj.name << ". Reason: " << err.str());
        return false;
    } else {
        return true;
    }
}

}  // namespace inviwo
