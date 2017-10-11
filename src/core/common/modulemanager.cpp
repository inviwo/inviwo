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
        modulesFactoryObjects_.clear();
    }) {}

ModuleManager::~ModuleManager() = default;

bool ModuleManager::isRuntimeModuleReloadingEnabled() {
    if (auto sys = app_->getSettingsByType<SystemSettings>()) {
        return sys->runtimeModuleReloading_.get();
    }
    return false;
}

void ModuleManager::registerModules(
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>> moduleFactories) {
    app_->printApplicationInfo();
    for (auto& mod : moduleFactories) {
        modulesFactoryObjects_.emplace_back(std::move(mod));
    }

    // Topological sort to make sure that we load modules in correct order
    topologicalModuleFactoryObjectSort(std::begin(modulesFactoryObjects_),
                                       std::end(modulesFactoryObjects_));

    std::vector<std::string> failed;
    auto checkdepends = [&](const auto& deps) {
        for (const auto& dep : deps) {
            auto it = util::find(failed, dep.first);
            if (it != failed.end()) return it;
        }
        return failed.end();
    };

    auto checkDepencyVersion = [&](const std::string& moduleName, const Version& depVersions) {
        auto it = util::find_if(modulesFactoryObjects_, [&](const auto& module) {
            return iCaseCmp(module->name, moduleName);
        });
        // Check if dependent module is of correct version
        return (it != modulesFactoryObjects_.end() &&
                (*it)->version.semanticVersionEqual(depVersions));
    };

    for (auto& moduleObj : modulesFactoryObjects_) {
        app_->postProgress("Loading module: " + moduleObj->name);
        if (util::contains_if(modules_, [&](const auto& module) {
                return module->getIdentifier().compare(moduleObj->name) == 0;
            })) {
            continue;
        }

        // Make sure that the module supports the current inviwo core version
        if (!Version(IVW_VERSION).semanticVersionEqual(Version(moduleObj->inviwoCoreVersion))) {
            LogError("Failed to register module: " << moduleObj->name);
            LogError("Reason: Module was built for Inviwo version "
                     << moduleObj->inviwoCoreVersion << ", current version is " << IVW_VERSION);
            util::push_back_unique(failed, toLower(moduleObj->name));
            continue;
        }
        // Check if dependency modules have correct versions.
        // Note that the module version only need to be increased
        // when changing and the inviwo core version has not changed
        // since we are ensuring the they must be built for the
        // same core version.
        std::stringstream depError;
        for (const auto& item : moduleObj->dependencies) {
            const auto& name = item.first;
            const auto& version = item.second;

            if (!checkDepencyVersion(name, version)) {
                auto it = util::find_if(modulesFactoryObjects_, [&name](const auto& module) {
                    return iCaseCmp(module->name, name);
                });
                if (it != modulesFactoryObjects_.end()) {
                    depError << "Module depends on " << name << " version " << version
                             << " but version " << (*it)->version << " was loaded" << std::endl;
                } else {
                    depError << "Module depends on " << name << " version " << version
                             << " but no such module was loaded" << std::endl;
                }
            };
        }
        if (depError.str().size() > 0) {
            LogError("Failed to register module: " << moduleObj->name);
            LogError("Reason: " << depError.str());
            util::push_back_unique(failed, toLower(moduleObj->name));
            continue;
        }
        try {
            auto it = checkdepends(moduleObj->dependencies);
            if (it == failed.end()) {
                registerModule(moduleObj->create(app_));
            } else {
                LogError("Could not register module: " + moduleObj->name +
                         " since dependency: " + *it + " failed to register");
            }
        } catch (const ModuleInitException& e) {
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
                    } else {
                        return false;
                    }
                });
                util::push_back_unique(failed, dereg);
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

void ModuleManager::registerModules(const std::vector<std::string>& librarySearchPaths) {
    bool reloadLibrariesWhenChanged = isRuntimeModuleReloadingEnabled();

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

    std::vector<std::unique_ptr<InviwoModuleFactoryObject>> modules;

    auto protectedModules = getProtectedModuleIdentifiers();

    auto orderByProtectedModule = [&](std::string first, std::string second) -> bool {
        auto foundFirst = protectedModules.find(util::stripModuleFileNameDecoration(first));
        auto foundSecond = protectedModules.find(util::stripModuleFileNameDecoration(second));
        if (foundFirst == protectedModules.end() && foundSecond == protectedModules.end()) {
            return iCaseLess(first, second);
        } else {
            return std::distance(protectedModules.begin(), foundFirst) <
                   std::distance(protectedModules.begin(), foundSecond);
        }
    };
    // Load protected modules first, necessary when library reloading is enabled
    // since dependent modules can exist in both temporary and application directory.
    // The modules which protected modules depend on will be loaded from the application
    // directory instead of the temporary directory.
    // Note: OpenGL module will fail to load if OpenGLQt is enabled and sorting is removed.
    std::set<std::string, decltype(orderByProtectedModule)> libraryFiles(
        orderByProtectedModule);               // Recursively found libraries
    std::set<std::string> libraryDirectories;  // Recursively found directories
#if WIN32
    // only consider files with dll extension
    std::set<std::string> libraryTypes{"dll"};
    // Prevent error mode dialogs from displaying.
    SetErrorMode(SEM_FAILCRITICALERRORS);
    // Get AddDllDirectory function.
    // This function is only available after installing KB2533623
    using addDllDirectory_func = DLL_DIRECTORY_COOKIE(WINAPI*)(PCWSTR);
    addDllDirectory_func lpfnAdllDllDirectory = reinterpret_cast<addDllDirectory_func>(
        GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "AddDllDirectory"));
    // Store added dll search directories so that we can remove them
    std::vector<DLL_DIRECTORY_COOKIE> addedSearchDirectories;
    std::vector<std::string> searchDirectories;
#else
    // only consider files with so, dylib or bundle extension
    std::set<std::string> libraryTypes{"so", "dylib", "bundle"};
#endif
    // Find unique files and directories in specified search paths
    for (auto path : librarySearchPaths) {
        // Make sure that we have an absolute path to avoid duplicates
        path = filesystem::cleanupPath(path);
        try {
            auto files = getDirectoryContentsRecursively(path, filesystem::ListMode::Files);
            auto dirs = getDirectoryContentsRecursively(path, filesystem::ListMode::Directories);
            libraryFiles.insert(std::make_move_iterator(files.begin()),
                                std::make_move_iterator(files.end()));
            libraryDirectories.insert(std::make_move_iterator(dirs.begin()),
                                      std::make_move_iterator(dirs.end()));
        } catch (FileException&) {
            // Invalid path, ignore it
        }
    }
    // Determines if a library is already loaded into the application
    auto isModuleLibraryLoaded = [&](const std::string path) {
        return moduleSharedLibraries_.end() !=
               std::find_if(
                   std::begin(moduleSharedLibraries_), std::end(moduleSharedLibraries_),
                   [path](const auto& lib) { return lib->getFilePath().compare(path) == 0; });
    };
    // Load enabled modules if file "application_name-enabled-modules.txt" exists,
    // otherwise load all modules
    std::vector<std::string> enabledModules;
    auto enabledModuleFileName =
        filesystem::getFileNameWithoutExtension(filesystem::getExecutablePath()) +
        "-enabled-modules.txt";
#ifdef __APPLE__
    // Executable path is inviwo.app/Content/MacOs
    std::ifstream enabledModulesFile(filesystem::getFileDirectory(filesystem::getExecutablePath()) +
                                     "/../../../" + enabledModuleFileName);
#else
    std::ifstream enabledModulesFile(filesystem::getFileDirectory(filesystem::getExecutablePath()) +
                                     "/" + enabledModuleFileName);
#endif

    std::copy(std::istream_iterator<std::string>(enabledModulesFile),
              std::istream_iterator<std::string>(), std::back_inserter(enabledModules));
    std::for_each(std::begin(enabledModules), std::end(enabledModules), toLower);
    auto isModuleEnabled = [&](const std::string& path) {
        const auto name = util::stripModuleFileNameDecoration(path);
        return enabledModules.empty() || enabledModules.end() != util::find(enabledModules, name);
    };
    // Remove unsupported files and files belonging to already loaded modules.
    util::map_erase_remove_if(libraryFiles, [&](const auto& file) {
        return libraryTypes.find(filesystem::getFileExtension(file)) == libraryTypes.end() ||
               file.find("inviwo-module") == std::string::npos || isModuleLibraryLoaded(file) ||
               !isModuleEnabled(file);
    });

    // Libraries are copied to a temporary folder in the case of runtime re-loading.
    // Otherwise, tmpSharedLibraryFiles == originalLibraryFiles
    std::vector<std::string> tmpSharedLibraryFiles;
    std::vector<std::string> originalLibraryFiles;
    std::string tmpLibraryDir =
        filesystem::getInviwoUserCachePath() + "/temporary-module-libraries";

    if (reloadLibrariesWhenChanged
#if WIN32
        // AdllDllDirectory must be supported on windows
        && lpfnAdllDllDirectory
#endif
    ) {

        if (!filesystem::directoryExists(tmpLibraryDir)) {
            filesystem::createDirectoryRecursively(tmpLibraryDir);
        }
        for (const auto& filePath : libraryFiles) {

            std::string dstPath =
                tmpLibraryDir + "/" + filesystem::getFileNameWithExtension(filePath);
            if (protectedModules.end() !=
                protectedModules.find(util::stripModuleFileNameDecoration(filePath))) {
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
        searchDirectories.push_back(tmpLibraryDir);
#endif
    } else {
        // Libraries will not be reloaded, we can use the original files
        std::copy(std::begin(libraryFiles), std::end(libraryFiles),
                  std::back_inserter(originalLibraryFiles));
        std::copy(std::begin(libraryFiles), std::end(libraryFiles),
                  std::back_inserter(tmpSharedLibraryFiles));
    }

#if WIN32
    searchDirectories.insert(searchDirectories.end(), librarySearchPaths.begin(),
                             librarySearchPaths.end());
    searchDirectories.insert(searchDirectories.end(), libraryDirectories.begin(),
                             libraryDirectories.end());

    if (lpfnAdllDllDirectory) {
        // Add search paths to find module dll dependencies
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        for (auto searchPath : searchDirectories) {
            searchPath = filesystem::cleanupPath(searchPath);
            replaceInString(searchPath, "/", "\\");
            const auto path = converter.from_bytes(searchPath);
            const auto dlldir = AddDllDirectory(path.c_str());
            if (dlldir) {
                addedSearchDirectories.emplace_back(dlldir);
            } else {
                LogWarn("Could not get AddDllDirectory for path " << searchPath);
            }
        }
    }
#endif

    // Load libraries from temporary directory
    // but observe the original file
    for (auto&& item : util::zip(originalLibraryFiles, tmpSharedLibraryFiles)) {
        auto filePath = get<0>(item);
        auto tmpPath = get<1>(item);
        try {
            // Load library. Will throw exception if failed to load
            auto sharedLib = util::make_unique<SharedLibrary>(tmpPath);
            // Only consider libraries with Inviwo module creation function
            if (auto moduleFunc =
                    reinterpret_cast<f_getModule>(sharedLib->findSymbol("createModule"))) {
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
                        moduleLibraryObserver_ = util::make_unique<InviwoModuleLibraryObserver>();
                    }
                    if (!moduleLibraryObserver_->isObserved(filePath)) {
                        moduleLibraryObserver_->startFileObservation(filePath);
                    }
                }
            } else {
                LogInfo(
                    "Could not find 'createModule' function needed for creating the module in " +
                    tmpPath +
                    ". Make sure that you have compiled the library and exported the function.");
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
    registerModules(std::move(modules));
}

void ModuleManager::unregisterModules() {
    onModulesWillUnregister_.invoke();
    app_->getProcessorNetwork()->clear();
    ResourceManager::getPtr()->clearAllResources();
    // Need to clear the modules in reverse order since the might depend on each other.
    // The destruction order of vector is undefined.
    auto protectedModules = getProtectedModuleIdentifiers();

    for (auto it = std::rbegin(modules_); it != std::rend(modules_);) {
        if (protectedModules.count((*it)->getIdentifier()) == 0) {
            // Erase does not take reverse_iterator so we need to convert it
            it = decltype(it)(modules_.erase((++it).base()));
        } else {
            ++it;
        }
    }

    // Remove module factories
    util::erase_remove_if(modulesFactoryObjects_, [&](const auto& module) {
        return protectedModules.count(module->name) == 0;
    });

    // Modules should now have removed all allocated resources and it should be safe to unload
    // shared libraries.
    util::erase_remove_if(moduleSharedLibraries_, [&](const auto& module) {
        // Figure out module identifier from file name
        auto moduleName = util::stripModuleFileNameDecoration(module->getFilePath());
        return protectedModules.count(moduleName) == 0;
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
    return modulesFactoryObjects_;
}

InviwoModule* ModuleManager::getModuleByIdentifier(const std::string& identifier) const {
    const auto it = std::find_if(
        modules_.begin(), modules_.end(),
        [&](const std::unique_ptr<InviwoModule>& m) { return m->getIdentifier() == identifier; });
    if (it != modules_.end()) {
        return it->get();
    } else {
        return nullptr;
    }
}

std::vector<InviwoModule*> ModuleManager::getModulesByAlias(const std::string& alias) const {
    std::vector<InviwoModule*> res;
    for (const auto& mfo : modulesFactoryObjects_) {
        if (util::contains(mfo->aliases, alias)) {
            if (auto m = getModuleByIdentifier(mfo->name)) {
                res.push_back(m);
            }
        }
    }
    return res;
}

std::vector<std::string> ModuleManager::findDependentModules(const std::string& module) const {
    std::vector<std::string> dependencies;
    for (const auto& item : modulesFactoryObjects_) {
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
    return protectedIdentifiers_;
}

void ModuleManager::addProtectedIdentifier(const std::string& id) {
    protectedIdentifiers_.insert(id);
}

}  // namespace inviwo
