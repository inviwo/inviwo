/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <inviwo/core/common/modulecontainer.h>

#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/sharedlibrary.h>
#include <inviwo/core/util/fileobserver.h>

#include <fmt/format.h>
#include <fmt/std.h>

#include <ranges>

namespace inviwo {

namespace {

std::vector<ModuleContainer*> findTransitiveDependencies(const ModuleContainer& container,
                                                         std::vector<ModuleContainer>& containers) {

    std::vector<ModuleContainer*> dependencies;

    auto helper = [&](auto self, const ModuleContainer& cont) -> void {
        for (const auto& dependencyVersion : cont.dependencies()) {
            const auto& dependency = dependencyVersion.first;
            if (auto it = std::ranges::find(containers, dependency, &ModuleContainer::identifier);
                it != containers.end()) {

                if (std::ranges::find(dependencies, &*it) == dependencies.end()) {
                    dependencies.push_back(&*it);
                    self(self, *it);
                }
            } else {
                throw Exception(IVW_CONTEXT_CUSTOM("ModuleManager"), "Missing module dependency {}",
                                dependency);
            }
        }
    };

    helper(helper, container);

    return dependencies;
}

class LibFileObserver : public FileObserver {
public:
    LibFileObserver(InviwoApplication* app, const std::filesystem::path& filename,
                    std::function<void(const std::filesystem::path&)> callback)
        : FileObserver(app) {

        startFileObservation(filename);
    }
    virtual ~LibFileObserver() = default;

private:
    virtual void fileChanged(const std::filesystem::path& filename) {
        fileChangeCallback_(filename);
    }

    std::function<void(const std::filesystem::path&)> fileChangeCallback_;
};

}  // namespace

ModuleContainer::ModuleContainer(std::unique_ptr<InviwoModuleFactoryObject> mfo)
    : libFile_{}
    , tmpFile_{}
    , protectedModule_{mfo->protectedModule == ProtectedModule::on}
    , protectedLibrary_{true}
    , identifier_{toLower(mfo->name)}
    , reloadCallback_{nullptr}
    , observer_{nullptr}
    , sharedLibrary_{nullptr}
    , factoryObject_{std::move(mfo)}
    , module_{nullptr} {}

ModuleContainer::ModuleContainer(const std::filesystem::path& libFile, bool runtimeReload)
    : libFile_{libFile}
    , tmpFile_{}
    , protectedModule_{false}
    , protectedLibrary_{false}
    , identifier_{}
    , reloadCallback_{nullptr}
    , observer_{nullptr}
    , sharedLibrary_{nullptr}
    , factoryObject_{nullptr}
    , module_{} {

    load(runtimeReload);
}

std::filesystem::path ModuleContainer::getTmpDir() {
    auto pid = filesystem::getCurrentProcessId();
    const auto tmp = filesystem::getInviwoUserSettingsPath() / "temporary-module-libraries" /
                     fmt::to_string(pid);
    std::filesystem::create_directories(tmp);
    return tmp;
}

bool ModuleContainer::isLoaded(const std::filesystem::path& path) {
    auto loaded = filesystem::getLoadedLibraries();
    return util::contains_if(loaded, [&](const auto& lib) {
        std::error_code ec;
        return std::filesystem::equivalent(path, lib, ec);
    });
};

void ModuleContainer::unload() {
    factoryObject_.reset();
    sharedLibrary_.reset();
    if (std::filesystem::is_regular_file(tmpFile_)) {
        std::filesystem::remove(tmpFile_);
    }
}
void ModuleContainer::load(bool runtimeReload) {
    if (runtimeReload && !isLoaded(libFile_)) {
        auto tmpFile = getTmpDir() / libFile_.filename();
        // Load a copy of the file to make sure that we can overwrite the
        // file.
        std::error_code ec;
        if (!std::filesystem::copy_file(libFile_, tmpFile,
                                        std::filesystem::copy_options::update_existing, ec)) {
            throw Exception(IVW_CONTEXT, "Unable to write temporary file {} since: {}", tmpFile,
                            ec.message());
        }
        tmpFile_ = tmpFile;
    }

    filesystem::setWorkingDirectory(filesystem::getInviwoBinDir());
    sharedLibrary_ = std::make_unique<SharedLibrary>(tmpFile_.empty() ? libFile_ : tmpFile_);

    if (auto moduleFunc = sharedLibrary_->findSymbolTyped<f_getModule>("createModule")) {
        factoryObject_.reset(moduleFunc());
    } else {
        throw Exception(
            IVW_CONTEXT,
            "Could not find 'createModule' function needed for creating the module in {}. "
            "Make sure that you have compiled the library and exported the function.",
            libFile_);
    }

    identifier_ = toLower(factoryObject_->name);
    protectedModule_ = factoryObject_->protectedModule == ProtectedModule::on;
    protectedLibrary_ = protectedModule_;
}

ModuleContainer::ModuleContainer(ModuleContainer&&) = default;
ModuleContainer& ModuleContainer::operator=(ModuleContainer&&) = default;

ModuleContainer::~ModuleContainer() {
    resetModule();
    unload();
}
const std::string& ModuleContainer::identifier() const { return identifier_; }
const std::string& ModuleContainer::name() const { return factoryObject_->name; }

void ModuleContainer::setReloadCallback(InviwoApplication* app,
                                        std::function<void(ModuleContainer&)> callback) {
    if (!libFile_.empty()) {
        observer_ = std::make_unique<LibFileObserver>(
            app, libFile_, [this](const std::filesystem::path&) { reloadCallback_(*this); });
    }
}

void ModuleContainer::createModule(InviwoApplication* app) {
    if (!module_) {
        // This is a huge hack since we can't pass this information into the InviwoModule constructor
        // without updating all the module constructors, which we want to avoid.
        // And we need the information in the constructor since we use it to register various paths
        // in the constructor, like python script and glsl code.
        app->getModuleManager().setModuleLocator([this](const InviwoModule& m) {
            auto path = filesystem::findBasePath() / "modules" / toLower(m.getIdentifier());
            path = path.lexically_normal();

            if (!std::filesystem::is_directory(path)) {
                if (std::filesystem::is_directory(factoryObject_->srcPath)) {
                    return factoryObject_->srcPath;
                } else if (sharedLibrary_) {
                    // TODO handle decide on "module layout" and handle it here
                    auto libdir = sharedLibrary_->getFilePath().parent_path();
                    return libdir.parent_path();
                }
            }
            return path;
        });

        module_ = factoryObject_->create(app);

        app->getModuleManager().setModuleLocator(nullptr);
    }
}
InviwoModule* ModuleContainer::getModule() const { return module_.get(); }
void ModuleContainer::resetModule() { module_.reset(); }

InviwoModuleFactoryObject& ModuleContainer::factoryObject() const { return *factoryObject_; }

bool ModuleContainer::dependsOn(std::string_view identifier) const {
    const auto& deps = factoryObject_->dependencies;
    return std::ranges::find(deps, identifier, [&](auto& dep) { return dep.first; }) != deps.end();
}

const std::vector<std::pair<std::string, Version>>& ModuleContainer::dependencies() const {
    return factoryObject_->dependencies;
}

void ModuleContainer::updateGraph(std::vector<ModuleContainer>& moduleContainers) {
    for (auto& cont : moduleContainers) {
        cont.transitiveDependencies = findTransitiveDependencies(cont, moduleContainers);
    }
    for (auto& cont : moduleContainers) {
        for (auto* dependency : cont.transitiveDependencies) {
            if (std::ranges::find(dependency->transitiveDependents, &cont) ==
                dependency->transitiveDependents.end()) {
                dependency->transitiveDependents.push_back(&cont);
            }
        }
    }
    for (auto& cont : moduleContainers) {
        if (cont.factoryObject_->protectedModule == ProtectedModule::on ||
            std::ranges::any_of(cont.transitiveDependents, [](ModuleContainer* c) {
                return c->factoryObject_->protectedModule == ProtectedModule::on;
            })) {
            cont.protectedModule_ = true;
        } else {
            cont.protectedModule_ = false;
        }
    }
    for (auto& cont : moduleContainers) {
        if (cont.libFile_.empty() || cont.factoryObject_->protectedModule == ProtectedModule::on ||
            std::ranges::any_of(cont.transitiveDependents, [](ModuleContainer* c) {
                return c->libFile_.empty() ||
                       c->factoryObject_->protectedModule == ProtectedModule::on;
            })) {
            cont.protectedLibrary_ = true;
        } else {
            cont.protectedLibrary_ = false;
        }
    }
}

}  // namespace inviwo
