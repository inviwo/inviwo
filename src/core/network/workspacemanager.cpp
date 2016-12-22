/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

namespace inviwo {

WorkspaceManager::WorkspaceManager()  {}

WorkspaceManager::~WorkspaceManager() = default;

void WorkspaceManager::clearWorkspace() {
    clears_.invoke();
}

void WorkspaceManager::saveWorkspace(std::ostream& stream, const std::string& refPath,
                                     const ExceptionHandler& exceptionHandler) {
    Serializer serializer(refPath);
    serializers_.invoke(serializer, exceptionHandler);
    serializer.writeFile(stream);
}

void WorkspaceManager::loadWorkspace(std::istream& stream, const std::string& refPath,
                                     const ExceptionHandler& exceptionHandler) {
    Deserializer deserializer(stream, refPath);
    for (const auto& factory : registeredFactories_) {
        deserializer.registerFactory(factory);
    }
    deserializers_.invoke(deserializer, exceptionHandler);
}

void WorkspaceManager::saveWorkspace(const std::string& path,
                                     const ExceptionHandler& exceptionHandler) {
    if (auto ostream = std::ofstream(path.c_str())) {
        saveWorkspace(ostream, path, exceptionHandler);
    } else {
        throw AbortException("Could not open workspace file: " + path, IvwContext);
    }
}

void WorkspaceManager::loadWorkspace(const std::string& path,
                                     const ExceptionHandler& exceptionHandler) {

    if (auto istream = std::ifstream(path.c_str())) {
        loadWorkspace(istream, path, exceptionHandler);
    } else {
        throw AbortException("Could not open workspace file: " + path, IvwContext);
    }
}

void WorkspaceManager::registerFactory(FactoryBase* factory) {
    registeredFactories_.push_back(factory);
}

WorkspaceManager::ClearHandle WorkspaceManager::addClearCallback(const ClearCallback& callback) {
    return clears_.add(callback);
}

WorkspaceManager::SerializationHandle WorkspaceManager::addSerializationCallback(
    const SerializationCallback& callback) {
    return serializers_.add([callback](Serializer& s, const ExceptionHandler& exceptionHandler) {
        try {
            callback(s);
        } catch (Exception& e) {
            if (exceptionHandler) {
                exceptionHandler(e.getContext());
            } else {
                util::log(e.getContext(), e.getMessage(), LogLevel::Error);
            }
        }
    });
}

WorkspaceManager::DeserializationHandle WorkspaceManager::addDeserializationCallback(
    const DeserializationCallback& callback) {
    return deserializers_.add([callback](Deserializer& d,
                                         const ExceptionHandler& exceptionHandler) {
        try {
            callback(d);
        } catch (const Exception& e) {
            if (exceptionHandler) {
                exceptionHandler(e.getContext());
            } else {
                util::log(e.getContext(), e.getMessage(), LogLevel::Error);
            }
        }
    });
}

} // namespace

