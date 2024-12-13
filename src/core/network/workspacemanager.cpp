/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/core/network/workspacemanager.h>

#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/network/processornetwork.h>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/std.h>

#include <memory_resource>

namespace inviwo {

class NetworkModified final : public ProcessorNetworkObserver {
public:
    NetworkModified(WorkspaceManager* manager) : manager_{manager} {};

protected:
    // Overrides for ProcessorNetworkObserver
    virtual void onProcessorNetworkChange() override { manager_->setModified(); }
    virtual void onProcessorNetworkDidAddProcessor(Processor*) override { manager_->setModified(); }
    virtual void onProcessorNetworkDidAddConnection(const PortConnection&) override {
        manager_->setModified();
    }
    virtual void onProcessorNetworkDidAddLink(const PropertyLink&) override {
        manager_->setModified();
    }
    virtual void onProcessorNetworkDidRemoveProcessor(Processor*) override {
        manager_->setModified();
    }
    virtual void onProcessorNetworkDidRemoveConnection(const PortConnection&) override {
        manager_->setModified();
    }
    virtual void onProcessorNetworkDidRemoveLink(const PropertyLink&) override {
        manager_->setModified();
    }

private:
    WorkspaceManager* manager_;
};

class WorkspaceConverter : public VersionConverter {
public:
    WorkspaceConverter(int from) : VersionConverter(), from_(from) {}

    virtual bool convert(TxElement* root) override {

        switch (from_) {
            case 0:
            case 1:
                return bundleProcessorNetwork(root);

            default:
                return false;  // No changes
        }
    }

private:
    bool bundleProcessorNetwork(TxElement* root) {

        // create
        TxElement newNode("ProcessorNetwork");

        // temp list
        std::vector<TxElement*> toBeDeleted;

        std::vector<std::string> toMove = {"ProcessorNetworkVersion", "Processors", "Connections",
                                           "PropertyLinks"};

        for (TiXmlElement* child = root->FirstChildElement(); child;
             child = child->NextSiblingElement()) {
            if (std::find(toMove.begin(), toMove.end(), child->Value()) != toMove.end()) {
                newNode.LinkEndChild(child->Clone());
                toBeDeleted.push_back(child);
            }
        }

        for (auto& elem : toBeDeleted) {
            root->RemoveChild(elem);
        }

        // insert new node
        root->InsertEndChild(newNode);

        return true;
    }

    int from_;
};

struct ErrorHandle {
    ErrorHandle(const InviwoSetupInfo& info, const std::filesystem::path& filename)
        : info_(info), filename_(filename) {}

    ~ErrorHandle() {
        if (!messages.empty()) {
            LogNetworkError("There were errors while loading workspace: " << filename_);
            for (auto& message : messages) {
                LogNetworkError(message);
            }
        }
    }

    void operator()(ExceptionContext c) {
        try {
            throw;
        } catch (SerializationException& error) {
            auto key = error.getKey();
            if (key == "Processor") {
                if (auto moduleInfo = info_.getModuleForProcessor(error.getType())) {
                    messages.push_back(fmt::format("{} ({})\nProcessor was in module: \"{}\".",
                                                   error.getMessage(), error.getContext(),
                                                   moduleInfo->name));
                } else {
                    messages.push_back(
                        fmt::format("{} ({})", error.getMessage(), error.getContext()));
                }
            } else {
                messages.push_back(fmt::format("{} ({})", error.getMessage(), error.getContext()));
            }
        } catch (Exception& error) {
            messages.push_back("" + fmt::format("Deserialization error: {} ({})",
                                                error.getMessage(), error.getContext()));
        }
    }

    std::vector<std::string> messages;
    const InviwoSetupInfo& info_;
    std::filesystem::path filename_;
};

WorkspaceManager::WorkspaceManager(InviwoApplication* app)
    : app_(app), modified_{false}, networkModified_{std::make_unique<NetworkModified>(this)} {

    app_->getProcessorNetwork()->addObserver(networkModified_.get());
}

WorkspaceManager::~WorkspaceManager() = default;

void WorkspaceManager::clear() {
    RenderContext::getPtr()->activateDefaultRenderContext();
    clears_.invoke();

    setModified(false);
}

void WorkspaceManager::save(std::ostream& stream, const std::filesystem::path& refPath,
                            const ExceptionHandler& exceptionHandler, WorkspaceSaveMode mode) {

    std::pmr::monotonic_buffer_resource mbr{1024 * 32};

    Serializer serializer(refPath, SerializeConstants::InviwoWorkspace, &mbr);
    serializer.setWorkspaceSaveMode(mode);

    if (mode != WorkspaceSaveMode::Undo) {
        InviwoSetupInfo info(*app_, *app_->getProcessorNetwork(), &mbr);
        serializer.serialize("InviwoSetup", info);
    }

    serializers_.invoke(serializer, exceptionHandler, mode);
    serializer.writeFile(stream, true);

    if (mode != WorkspaceSaveMode::Undo) {
        setModified(false);
    }
}

void WorkspaceManager::save(std::pmr::string& xml, const std::filesystem::path& refPath,
                            const ExceptionHandler& exceptionHandler, WorkspaceSaveMode mode) {

    std::pmr::monotonic_buffer_resource mbr{1024 * 32};

    Serializer serializer(refPath, SerializeConstants::InviwoWorkspace, &mbr);
    serializer.setWorkspaceSaveMode(mode);

    if (mode != WorkspaceSaveMode::Undo) {
        const InviwoSetupInfo info(*app_, *app_->getProcessorNetwork(), &mbr);
        serializer.serialize("InviwoSetup", info);
    }

    serializers_.invoke(serializer, exceptionHandler, mode);
    const bool indentXml = mode != WorkspaceSaveMode::Undo;
    serializer.write(xml, indentXml);

    if (mode != WorkspaceSaveMode::Undo) {
        setModified(false);
    }
}

void WorkspaceManager::load(std::istream& stream, const std::filesystem::path& refPath,
                            const ExceptionHandler& exceptionHandler, WorkspaceSaveMode mode) {
    RenderContext::getPtr()->activateDefaultRenderContext();

    std::pmr::monotonic_buffer_resource mbr{1024 * 128};

    auto [deserializer, info] =
        createWorkspaceDeserializerAndInfo(stream, refPath, LogCentral::getPtr(), &mbr);
    const DeserializationErrorHandle<ErrorHandle> errorHandle(deserializer, info, refPath);
    deserializers_.invoke(deserializer, exceptionHandler, mode);

    if (mode != WorkspaceSaveMode::Undo) {
        setModified(false);
    }
}

void WorkspaceManager::load(const std::pmr::string& xml, const std::filesystem::path& refPath,
                            const ExceptionHandler& exceptionHandler, WorkspaceSaveMode mode) {
    RenderContext::getPtr()->activateDefaultRenderContext();

    std::pmr::monotonic_buffer_resource mbr{1024 * 128};

    auto [deserializer, info] =
        createWorkspaceDeserializerAndInfo(xml, refPath, LogCentral::getPtr(), &mbr);
    const DeserializationErrorHandle<ErrorHandle> errorHandle(deserializer, info, refPath);
    deserializers_.invoke(deserializer, exceptionHandler, mode);

    if (mode != WorkspaceSaveMode::Undo) {
        setModified(false);
    }
}

void WorkspaceManager::save(const std::filesystem::path& path,
                            const ExceptionHandler& exceptionHandler, WorkspaceSaveMode mode) {
    auto ostream = std::ofstream(path);
    if (ostream.is_open()) {
        save(ostream, path, exceptionHandler, mode);
    } else {
        throw AbortException(IVW_CONTEXT, "Could not open workspace file: {}", path);
    }
}

void WorkspaceManager::load(const std::filesystem::path& path,
                            const ExceptionHandler& exceptionHandler, WorkspaceSaveMode mode) {

    FILE* file = filesystem::fopen(path, "rb");
    if (!file) {
        throw AbortException(IVW_CONTEXT, "Could not open workspace file: {}", path);
    }
    const util::OnScopeExit closeFile{[file]() { std::fclose(file); }};

    const long length = [&]() {
        std::fseek(file, 0, SEEK_END);
        const auto len = std::ftell(file);
        std::fseek(file, 0, SEEK_SET);
        return len;
    }();
    std::pmr::string data(length, '0');
    if (std::fread(data.data(), length, 1, file) != 1) {
        throw AbortException(IVW_CONTEXT, "Could not read workspace file: {}", path);
    }
    load(data, path, exceptionHandler, mode);
}

void WorkspaceManager::registerFactory(FactoryBase* factory) {
    registeredFactories_.push_back(factory);
}

Deserializer WorkspaceManager::createWorkspaceDeserializer(
    std::istream& stream, const std::filesystem::path& refPath, Logger* logger,
    std::pmr::polymorphic_allocator<std::byte> alloc) const {

    return createWorkspaceDeserializerAndInfo(stream, refPath, logger, alloc).first;
}

std::pair<Deserializer, InviwoSetupInfo> WorkspaceManager::createWorkspaceDeserializerAndInfo(
    std::istream& stream, const std::filesystem::path& refPath, Logger* logger,
    std::pmr::polymorphic_allocator<std::byte> alloc) const {

    std::pair<Deserializer, InviwoSetupInfo> result{
        std::piecewise_construct,
        std::forward_as_tuple(stream, refPath, SerializeConstants::InviwoWorkspace, alloc),
        std::forward_as_tuple(alloc)};
    auto& [deserializer, info] = result;

    configureWorkspaceDeserializerAndInfo(deserializer, info, logger);

    return result;
}

std::pair<Deserializer, InviwoSetupInfo> WorkspaceManager::createWorkspaceDeserializerAndInfo(
    const std::pmr::string& xml, const std::filesystem::path& refPath, Logger* logger,
    std::pmr::polymorphic_allocator<std::byte> alloc) const {

    std::pair<Deserializer, InviwoSetupInfo> result{
        std::piecewise_construct,
        std::forward_as_tuple(xml, refPath, SerializeConstants::InviwoWorkspace, alloc),
        std::forward_as_tuple(alloc)};
    auto& [deserializer, info] = result;

    configureWorkspaceDeserializerAndInfo(deserializer, info, logger);

    return result;
}

void WorkspaceManager::configureWorkspaceDeserializerAndInfo(Deserializer& deserializer,
                                                             InviwoSetupInfo& info,
                                                             Logger* logger) const {

    deserializer.setLogger(logger);
    for (const auto& factory : registeredFactories_) {
        deserializer.registerFactory(factory);
    }

    if (SerializeConstants::InviwoWorkspaceVersion != deserializer.getVersion()) {
        WorkspaceConverter converter(deserializer.getVersion());
        deserializer.convertVersion(&converter);
    }

    deserializer.deserialize("InviwoSetup", info);

    for (const auto& inviwoModule : app_->getModuleManager().getInviwoModules()) {
        if (auto moduleInfo = info.getModuleInfo(inviwoModule.getIdentifier())) {
            if (moduleInfo->version < inviwoModule.getVersion()) {
                auto converter = inviwoModule.getConverter(moduleInfo->version);
                deserializer.convertVersion(converter.get());
                LogNetworkSpecial(
                    (&deserializer), LogLevel::Warn,
                    fmt::format(
                        "Loading old workspace ({}) Module version: {}. Updating to version: {}.",
                        deserializer.getFileName(), inviwoModule.getIdentifier(),
                        moduleInfo->version, inviwoModule.getVersion()));
            }
        }
    }
}

WorkspaceManager::ClearHandle WorkspaceManager::onClear(const ClearCallback& callback) {
    return clears_.add(callback);
}

WorkspaceManager::SerializationHandle WorkspaceManager::onSave(
    const SerializationCallback& callback, WorkspaceSaveModes modes) {
    return serializers_.add([callback, modes](Serializer& s,
                                              const ExceptionHandler& exceptionHandler,
                                              WorkspaceSaveMode mode) {
        if (modes.count(mode)) {
            try {
                callback(s);
            } catch (Exception& e) {
                exceptionHandler(e.getContext());
            } catch (...) {
                exceptionHandler(IVW_CONTEXT_CUSTOM("WorkspaceManager"));
            }
        }
    });
}

WorkspaceManager::DeserializationHandle WorkspaceManager::onLoad(
    const DeserializationCallback& callback, WorkspaceSaveModes modes) {
    return deserializers_.add([callback, modes](Deserializer& d,
                                                const ExceptionHandler& exceptionHandler,
                                                WorkspaceSaveMode mode) {
        if (modes.count(mode)) {
            try {
                callback(d);
            } catch (Exception& e) {
                exceptionHandler(e.getContext());
            } catch (...) {
                exceptionHandler(IVW_CONTEXT_CUSTOM("WorkspaceManager"));
            }
        }
    });
}

void WorkspaceManager::setModified() { setModified(true); }

void WorkspaceManager::setModified(bool modified) {
    if (modified_ != modified) {
        modified_ = modified;
        modifiedChangedDispatcher_.invoke(modified_);
        modifiedDispatcher_.invoke(modified_);
    }
    modifiedDispatcher_.invoke(modified_);
}

bool WorkspaceManager::isModified() const { return modified_; }

WorkspaceManager::ModifiedChangedHandle WorkspaceManager::onModifiedChanged(
    const ModifiedChangedCallback& callback) {
    return modifiedChangedDispatcher_.add(callback);
}

WorkspaceManager::ModifiedHandle WorkspaceManager::onModified(const ModifiedCallback& callback) {
    return modifiedDispatcher_.add(callback);
}

}  // namespace inviwo
