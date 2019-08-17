/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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
#include <inviwo/core/common/version.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/sharedlibrary.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/capabilities.h>
#include <inviwo/core/network/processornetwork.h>

#include <string>
#include <functional>

namespace inviwo {

ModuleManager::ModuleManager(InviwoApplication* app)
    : app_{app}
    , protected_{}
    , onModulesDidRegister_{}
    , onModulesWillUnregister_{}
    , libraryObserver_{app}
    , sharedLibraries_{}
    , clearLibs_([&]() {
        util::reverse_erase_if(sharedLibraries_, [this](const auto& module) {
            // Figure out module identifier from file name
            auto moduleName = util::stripModuleFileNameDecoration(module->getFilePath());
            return !this->isProtected(moduleName);
        });
        // Leak the protected dlls here to avoid openglqt crash
        for (auto& lib : sharedLibraries_) lib->release();
    })
    , factoryObjects_{}
    , modules_{}
    , clearModules_([&]() {
        // Need to clear the modules in reverse order since the might depend on each other.
        // The destruction order of vector is undefined.
        util::reverse_erase(modules_);
    }) {}

ModuleManager::~ModuleManager() = default;

bool ModuleManager::isRuntimeModuleReloadingEnabled() {
    return app_->getSystemSettings().runtimeModuleReloading_;
}

void ModuleManager::registerModules(std::vector<std::unique_ptr<InviwoModuleFactoryObject>> mfo) {
    factoryObjects_.insert(factoryObjects_.end(), std::make_move_iterator(mfo.begin()),
                           std::make_move_iterator(mfo.end()));

    // Topological sort to make sure that we load modules in correct order
    topologicalModuleFactoryObjectSort(std::begin(factoryObjects_), std::end(factoryObjects_));

    for (auto& obj : factoryObjects_) {
        app_->postProgress("Loading module: " + obj->name);
        if (getModuleByIdentifier(obj->name)) continue;  // already loaded
        if (!checkDependencies(*obj)) continue;
        try {
            registerModule(obj->create(app_));
        } catch (const ModuleInitException& e) {
            auto dereg = deregisterDependetModules(e.getModulesToDeregister());
            auto err = (!dereg.empty() ? "\nUnregistered dependent modules: " +
                                             joinString(dereg.begin(), dereg.end(), ", ")
                                       : "");
            LogError("Failed to register module: " << obj->name << ". Reason:\n"
                                                   << e.getMessage() << err);
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
    if (!filesystem::fileExists(enabledModulesFilePath)) {
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

void ModuleManager::reloadModules() {
    if (isRuntimeModuleReloadingEnabled()) libraryObserver_.reloadModules();
}

void ModuleManager::registerModules(RuntimeModuleLoading) {
    // Perform the following steps
    // 1. Recursively get all library files and the folders they are in
    // 2. Filter out files with correct extension, named inviwo-module
    //    and listed in application_name-enabled-modules.txt (if it exist).
    // 3. Load libraries and see if createModule function exist.
    // 4. Start observing file if reloadLibrariesWhenChanged
    // 5. Pass module factories to registerModules

    // Find unique files and directories in specified search paths
    auto librarySearchPaths = util::getLibrarySearchPaths();
    std::set<std::string> libraryFiles;
    LibrarySearchDirs searchDirectories{librarySearchPaths};
    for (auto path : librarySearchPaths) {
        using namespace inviwo::filesystem;
        // Make sure that we have an absolute path to avoid duplicates
        path = cleanupPath(path);
        try {
            auto files = getDirectoryContentsRecursively(path, ListMode::Files);
            auto dirs = getDirectoryContentsRecursively(path, ListMode::Directories);
            libraryFiles.insert(std::make_move_iterator(files.begin()),
                                std::make_move_iterator(files.end()));
            searchDirectories.add(dirs);
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
    auto libraryTypes = SharedLibrary::libraryFileExtensions();
    // Remove unsupported files and files belonging to already loaded modules.
    util::map_erase_remove_if(libraryFiles, [&](const auto& file) {
        return libraryTypes.count(filesystem::getFileExtension(file)) == 0 ||
               (file.find("inviwo-module") == std::string::npos &&
                file.find("inviwo-core") == std::string::npos) ||
               isModuleLibraryLoaded(file) || !isEnabled(file);
    });

    const auto tmpDir = [&]() -> std::string {
        if (isRuntimeModuleReloadingEnabled() && util::hasAddLibrarySearchDirsFunction()) {
            const auto tmp =
                filesystem::getInviwoUserSettingsPath() + "/temporary-module-libraries";
            if (!filesystem::directoryExists(tmp)) {
                filesystem::createDirectoryRecursively(tmp);
            }
            searchDirectories.add({tmp});
            return tmp;
        } else {
            return "";
        }
    }();

    // Load libraries from temporary directory but observe the original file
    auto isLoaded = [loaded = util::getLoadedLibraries()](const auto& path) {
        return util::contains_if(loaded, [&](const auto& lib) { return iCaseCmp(path, lib); });
    };
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>> modules;
    for (const auto& filePath : libraryFiles) {
        const auto tmpPath = [&]() -> std::string {
            if (isRuntimeModuleReloadingEnabled() && util::hasAddLibrarySearchDirsFunction()) {
                auto dstPath = tmpDir + "/" + filesystem::getFileNameWithExtension(filePath);
                if (isLoaded(filePath)) {
                    // Already loaded modules are loaded from the application dir
                    dstPath = filePath;
                    protected_.insert(util::stripModuleFileNameDecoration(filePath));
                } else if (filesystem::fileModificationTime(filePath) !=
                           filesystem::fileModificationTime(dstPath)) {
                    // Load a copy of the file to make sure that we can overwrite the file.
                    filesystem::copyFile(filePath, dstPath);
                }
                return dstPath;
            } else {
                return filePath;
            }
        }();

        try {
            // Load library. Will throw exception if failed to load
            auto sharedLib = std::make_unique<SharedLibrary>(tmpPath);
            // Only consider libraries with Inviwo module creation function
            if (auto moduleFunc = sharedLib->findSymbolTyped<f_getModule>("createModule")) {
                // Add module factory object
                modules.emplace_back(moduleFunc());
                auto moduleName = toLower(modules.back()->name);
                if (modules.back()->protectedModule == ProtectedModule::on) {
                    protected_.insert(modules.back()->name);
                }
                sharedLibraries_.emplace_back(std::move(sharedLib));
                if (isRuntimeModuleReloadingEnabled()) {
                    libraryObserver_.observe(filePath);
                }
            } else {
                LogInfo(
                    "Could not find 'createModule' function needed for creating the module in "
                    << tmpPath
                    << ". Make sure that you have compiled the library and exported the function.");
            }
        } catch (const Exception& e) {
            // Library dependency is probably missing. We silently skip this library.
            LogInfo("Could not load library: " << filePath << " " << e.getMessage());
        }
    }

    auto dependencies = getProtectedDependencies(protected_, modules);
    protected_.insert(dependencies.begin(), dependencies.end());

    registerModules(std::move(modules));
}

void ModuleManager::unregisterModules() {
    onModulesWillUnregister_.invoke();
    app_->getProcessorNetwork()->clear();
    // Need to clear the modules in reverse order since the might depend on each other.
    // The destruction order of vector is undefined.
    util::reverse_erase_if(
        modules_, [this](const auto& m) { return !this->isProtected(m->getIdentifier()); });

    // Remove module factories
    util::reverse_erase_if(factoryObjects_,
                           [this](const auto& mfo) { return !this->isProtected(mfo->name); });

    // Modules should now have removed all allocated resources and it should be safe to unload
    // shared libraries.
    util::reverse_erase_if(sharedLibraries_, [this](const auto& module) {
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

std::vector<std::string> ModuleManager::deregisterDependetModules(
    const std::vector<std::string>& toDeregister) {
    IdSet deregister;
    for (const auto& m : toDeregister) {
        deregister.insert(m);
        auto dependents = findDependentModules(m);
        deregister.insert(dependents.begin(), dependents.end());
    }
    std::vector<std::string> deregistered;
    util::reverse_erase_if(modules_, [&](const auto& m) {
        if (deregister.count(m->getIdentifier()) != 0) {
            deregistered.push_back(m->getIdentifier());
            return true;
        } else {
            return false;
        }
    });
    return deregistered;
}

auto ModuleManager::getProtectedDependencies(
    const IdSet& ptotectedIds,
    const std::vector<std::unique_ptr<InviwoModuleFactoryObject>>& modules) -> IdSet {
    IdSet dependencies;
    std::function<void(const std::string&)> getDeps = [&](const std::string& module) {
        auto it = util::find_if(modules, [&](const auto& m) { return iCaseCmp(m->name, module); });
        if (it != modules.end()) {
            for (const auto dep : (*it)->dependencies) {
                dependencies.insert(dep.first);
                getDeps(dep.first);
            }
        }
    };
    for (auto& module : ptotectedIds) {
        getDeps(module);
    }
    return dependencies;
}

}  // namespace inviwo
