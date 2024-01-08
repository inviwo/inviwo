/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/dispatcher.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/common/runtimemoduleregistration.h>
#include <inviwo/core/common/modulecontainer.h>

#include <set>
#include <vector>
#include <memory>
#include <span>
#include <ranges>
#include <functional>

namespace inviwo {

class InviwoModule;

/**
 * Manages finding, loading, unloading, reloading of Inviwo modules
 */
class IVW_CORE_API ModuleManager {
public:
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

    void registerModules(std::vector<ModuleContainer> moduleFactories);

    /**
     * \brief Load modules from dynamic library files in the specified search paths.
     *
     * Will recursively search for all dll/so/dylib/bundle files in the specified search paths.
     * The library filename must contain "inviwo-module" to be loaded.
     *
     * @note Which modules to load can be specified by creating a file
     * (application_name-enabled-modules.txt) containing the names of the modules to load.
     */
    void registerModules(RuntimeModuleLoading, std::function<bool(std::string_view)> filter =
                                                   ModuleManager::getEnabledFilter());

    std::vector<ModuleContainer> findRuntimeModules(
        std::span<const std::filesystem::path> searchPaths);

    std::vector<ModuleContainer> findRuntimeModules(
        std::span<const std::filesystem::path> searchPaths,
        std::function<bool(std::string_view)> filter);

    std::vector<ModuleContainer> findRuntimeModules(
        std::span<const std::filesystem::path> searchPaths,
        std::function<bool(std::string_view)> filter, bool runtimeReloading);

    auto getInviwoModules() {
        static constexpr auto notNull = [](ModuleContainer& cont) -> bool {
            return cont.getModule() != nullptr;
        };
        static constexpr auto moduleRef = [](ModuleContainer& cont) -> InviwoModule& {
            return *cont.getModule();
        };

        return inviwoModules_ | std::views::filter(notNull) | std::views::transform(moduleRef);
    }

    auto getInviwoModules() const {
        static constexpr auto notNull = [](const ModuleContainer& cont) -> bool {
            return cont.getModule() != nullptr;
        };
        static constexpr auto moduleRef = [](const ModuleContainer& cont) -> const InviwoModule& {
            return *cont.getModule();
        };

        return inviwoModules_ | std::views::filter(notNull) | std::views::transform(moduleRef);
    }

    auto getFactoryObjects() {
        static constexpr auto factoryObjRef =
            [](const ModuleContainer& cont) -> const InviwoModuleFactoryObject& {
            return cont.factoryObject();
        };
        return inviwoModules_ | std::views::transform(factoryObjRef);
    }

    size_t size() const { return inviwoModules_.size(); }

    template <class T>
    T* getModuleByType() const;
    InviwoModule* getModuleByIdentifier(std::string_view identifier) const;
    InviwoModule* getModuleByIndex(size_t index) const { return inviwoModules_[index].getModule(); }
    std::vector<InviwoModule*> getModulesByAlias(std::string_view alias) const;
    InviwoModuleFactoryObject* getFactoryObject(std::string_view identifier) const;
    std::vector<std::string> findDependentModules(std::string_view module) const;

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

    static std::function<bool(std::string_view)> getEnabledFilter();
    void reloadModules();

    // This is a hack to avoid having to add more arguments to the InviwoModule constructor
    void setModuleLocator(std::function<std::filesystem::path(const InviwoModule&)> moduleLocator);
    std::filesystem::path locateModule(const InviwoModule&) const;

private:
    bool checkDependencies(const InviwoModuleFactoryObject& obj) const;
    std::vector<std::string> deregisterDependentModules(
        const std::vector<std::string>& toDeregister);

    InviwoApplication* app_;

    Dispatcher<void()> onModulesDidRegister_;     ///< Called after modules have been registered
    Dispatcher<void()> onModulesWillUnregister_;  ///< Called before modules have been unregistered

    std::vector<ModuleContainer> inviwoModules_;

    std::function<std::filesystem::path(const InviwoModule&)> moduleLocator_;
};

template <class T>
T* ModuleManager::getModuleByType() const {
    for (auto& cont : inviwoModules_) {
        if (auto* m = dynamic_cast<T*>(cont.getModule())) {
            return m;
        }
    }
    return nullptr;
}

}  // namespace inviwo
