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

#ifndef IVW_MODULEMANAGER_H
#define IVW_MODULEMANAGER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/dispatcher.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/common/runtimemoduleregistration.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/common/inviwomodulelibraryobserver.h>

#include <warn/push>
#include <warn/ignore/all>
#include <set>
#include <warn/pop>

namespace inviwo {

class InviwoModule;
class ModuleCallbackAction;
class FileObserver;

class SharedLibrary;

/**
 * Manages finding, loading, unloading, reloading of Inviwo modules
 */
class IVW_CORE_API ModuleManager {
public:
    using IdSet = std::set<std::string, CaseInsensitiveCompare>;

    ModuleManager(InviwoApplication* app);
    ModuleManager(const ModuleManager& rhs) = delete;
    ModuleManager& operator=(const ModuleManager& that) = delete;
    ~ModuleManager();

    /*
     * Use as second argument in InviwoApplication::registerModules
     * See inviwo.cpp for an example.
     */
    bool isRuntimeModuleReloadingEnabled();

    /**
     * \brief Registers modules from factories and takes ownership of input module factories.
     * Module is registered if dependencies exist and they have correct version.
     */
    void registerModules(std::vector<std::unique_ptr<InviwoModuleFactoryObject>> moduleFactories);
    /**
     * \brief Load modules from dynamic library files in the specified search paths.
     *
     * Will recursively search for all dll/so/dylib/bundle files in the specified search paths.
     * The library filename must contain "inviwo-module" to be loaded.
     *
     * @note Which modules to load can be specified by creating a file
     * (application_name-enabled-modules.txt) containing the names of the modules to load.
     */
    void registerModules(RuntimeModuleLoading);

    /**
     * \brief Removes all modules not marked as protected by the application.
     *
     * Use this function with care since all modules will be destroyed.
     * 1. Network will be cleared.
     * 2. Non-protected modules will be removed.
     * 3. Loaded dynamic module libraries will be unloaded (unless marked as protected).
     *
     * @see InviwoApplication::getProtectedModuleIdentifiers
     * @see InviwoModuleLibraryObserver
     */
    void unregisterModules();

    const std::vector<std::unique_ptr<InviwoModule>>& getModules() const;
    const std::vector<std::unique_ptr<InviwoModuleFactoryObject>>& getModuleFactoryObjects() const;
    template <class T>
    T* getModuleByType() const;
    InviwoModule* getModuleByIdentifier(const std::string& identifier) const;
    std::vector<InviwoModule*> getModulesByAlias(const std::string& alias) const;
    InviwoModuleFactoryObject* getFactoryObject(const std::string& identifier) const;
    std::vector<std::string> findDependentModules(const std::string& module) const;

    /**
     * \brief Register callback for monitoring when modules have been registered.
     * Invoked in registerModules.
     */
    std::shared_ptr<std::function<void()>> onModulesDidRegister(std::function<void()> callback);
    /**
     * \brief Register callback for monitoring when modules have been registered.
     * Invoked in unregisterModules.
     */
    std::shared_ptr<std::function<void()>> onModulesWillUnregister(std::function<void()> callback);

    /**
     * \brief List of modules to keep during runtime library reloading.
     *
     * Some modules such as Core can cause errors if unloaded.
     * Append them to this list in your application to prevent them from being unloaded.
     * @return Module identifiers of modules
     */
    const IdSet& getProtectedModuleIdentifiers() const;
    bool isProtected(const std::string& module) const;
    void addProtectedIdentifier(const std::string& id);

    static std::function<bool(const std::string&)> getEnabledFilter();
    void reloadModules();

private:
    void registerModule(std::unique_ptr<InviwoModule> module);
    bool checkDependencies(const InviwoModuleFactoryObject& obj) const;
    std::vector<std::string> deregisterDependetModules(
        const std::vector<std::string>& toDeregister);
    static auto getProtectedDependencies(
        const IdSet& ptotectedIds,
        const std::vector<std::unique_ptr<InviwoModuleFactoryObject>>& modules) -> IdSet;

    InviwoApplication* app_;
    IdSet protected_;

    Dispatcher<void()> onModulesDidRegister_;     ///< Called after modules have been registered
    Dispatcher<void()> onModulesWillUnregister_;  ///< Called before modules have been unregistered

    // Observes shared libraries and reload modules when file changes.
    InviwoModuleLibraryObserver libraryObserver_;
    std::vector<std::unique_ptr<SharedLibrary>> sharedLibraries_;
    util::OnScopeExit clearLibs_;
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>> factoryObjects_;
    std::vector<std::unique_ptr<InviwoModule>> modules_;
    util::OnScopeExit clearModules_;
};

template <class T>
T* ModuleManager::getModuleByType() const {
    return getTypeFromVector<T>(modules_);
}

}  // namespace inviwo

#endif  // IVW_MODULEMANAGER_H
