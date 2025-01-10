/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <inviwo/core/common/version.h>

#include <memory>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo {

class InviwoModule;
class InviwoApplication;
class InviwoModuleFactoryObject;
class FileObserver;
class SharedLibrary;

class IVW_CORE_API ModuleContainer {
public:
    ModuleContainer(std::unique_ptr<InviwoModuleFactoryObject> mfo);
    ModuleContainer(const std::filesystem::path& libFile, bool runtimeReload);

    ModuleContainer(const ModuleContainer&) = delete;
    ModuleContainer& operator=(const ModuleContainer&) = delete;
    ModuleContainer(ModuleContainer&&);
    ModuleContainer& operator=(ModuleContainer&&);

    ~ModuleContainer();

    const std::string& identifier() const;
    const std::string& name() const;

    void createModule(InviwoApplication* app);

    InviwoModule* getModule() const;
    void resetModule();

    void load(bool runtimeReload);
    void unload();

    InviwoModuleFactoryObject& factoryObject() const;

    bool dependsOn(std::string_view identifier) const;

    const std::vector<std::pair<std::string, Version>>& dependencies() const;

    bool isProtectedModule() const { return protectedModule_; }
    bool isProtectedLibrary() const { return protectedLibrary_; }

    static void updateGraph(std::vector<ModuleContainer>& moduleContainers);

    void setReloadCallback(InviwoApplication* app, std::function<void(ModuleContainer&)> callback);

private:
    static bool isLoaded(const std::filesystem::path& file);
    static std::filesystem::path getTmpDir();

    std::filesystem::path libFile_;
    std::filesystem::path tmpFile_;

    bool protectedModule_;
    bool protectedLibrary_;

    std::string identifier_;

    std::function<void(ModuleContainer&)> reloadCallback_;

    std::unique_ptr<FileObserver> observer_;
    std::unique_ptr<SharedLibrary> sharedLibrary_;
    std::unique_ptr<InviwoModuleFactoryObject> factoryObject_;
    std::unique_ptr<InviwoModule> module_;

    std::vector<ModuleContainer*> transitiveDependencies;
    std::vector<ModuleContainer*> transitiveDependents;
};

}  // namespace inviwo
