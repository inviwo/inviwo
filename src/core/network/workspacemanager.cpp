/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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
#include <inviwo/core/util/inviwosetupinfo.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/serialization/serialization.h>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/std.h>

namespace inviwo {

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
        TxElement newNode;
        newNode.SetValue("ProcessorNetwork");

        // temp list
        std::vector<TxElement*> toBeDeleted;

        std::vector<std::string> toMove = {"ProcessorNetworkVersion", "Processors", "Connections",
                                           "PropertyLinks"};

        ticpp::Iterator<TxElement> child;
        for (child = child.begin(root); child != child.end(); child++) {
            if (std::find(toMove.begin(), toMove.end(), child.Get()->Value()) != toMove.end()) {
                newNode.InsertEndChild(*(child.Get()->Clone()));
                toBeDeleted.push_back(child.Get());
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

WorkspaceManager::WorkspaceManager(InviwoApplication* app) : app_(app) {}

WorkspaceManager::~WorkspaceManager() = default;

void WorkspaceManager::clear() {
    RenderContext::getPtr()->activateDefaultRenderContext();
    clears_.invoke();
}

void WorkspaceManager::save(std::ostream& stream, const std::filesystem::path& refPath,
                            const ExceptionHandler& exceptionHandler, WorkspaceSaveMode mode) {
    Serializer serializer(refPath);

    if (mode != WorkspaceSaveMode::Undo) {
        InviwoSetupInfo info(*app_, *app_->getProcessorNetwork());
        serializer.serialize("InviwoSetup", info);
    }

    serializers_.invoke(serializer, exceptionHandler, mode);
    serializer.writeFile(stream, true);
}

void WorkspaceManager::load(std::istream& stream, const std::filesystem::path& refPath,
                            const ExceptionHandler& exceptionHandler) {
    RenderContext::getPtr()->activateDefaultRenderContext();

    auto deserializer = createWorkspaceDeserializer(stream, refPath);

    InviwoSetupInfo info;
    deserializer.deserialize("InviwoSetup", info);

    DeserializationErrorHandle<ErrorHandle> errorHandle(deserializer, info, refPath);

    deserializers_.invoke(deserializer, exceptionHandler);
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
                            const ExceptionHandler& exceptionHandler) {
    auto istream = std::ifstream(path);
    if (istream.is_open()) {
        load(istream, path, exceptionHandler);
    } else {
        throw AbortException(IVW_CONTEXT, "Could not open workspace file: {}", path);
    }
}

void WorkspaceManager::registerFactory(FactoryBase* factory) {
    registeredFactories_.push_back(factory);
}

Deserializer WorkspaceManager::createWorkspaceDeserializer(std::istream& stream,
                                                           const std::filesystem::path& refPath,
                                                           Logger* logger) const {

    Deserializer deserializer(stream, refPath);
    deserializer.setLogger(logger);
    for (const auto& factory : registeredFactories_) {
        deserializer.registerFactory(factory);
    }

    if (SerializeConstants::InviwoWorkspaceVersion != deserializer.getInviwoWorkspaceVersion()) {
        WorkspaceConverter converter(deserializer.getInviwoWorkspaceVersion());
        deserializer.convertVersion(&converter);
    }

    InviwoSetupInfo info;
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

    return deserializer;
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
    const DeserializationCallback& callback) {
    return deserializers_.add(
        [callback](Deserializer& d, const ExceptionHandler& exceptionHandler) {
            try {
                callback(d);
            } catch (Exception& e) {
                exceptionHandler(e.getContext());
            } catch (...) {
                exceptionHandler(IVW_CONTEXT_CUSTOM("WorkspaceManager"));
            }
        });
}

}  // namespace inviwo
