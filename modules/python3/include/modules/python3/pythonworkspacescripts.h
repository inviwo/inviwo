/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/network/workspacemanager.h>

#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>

#include <map>
#include <optional>

namespace inviwo {

class IVW_MODULE_PYTHON3_API PythonWorkspaceScriptsObserver : public Observer {
public:
    virtual void onScriptAdded(std::string_view key, std::string_view script);
    virtual void onScriptRemoved(std::string_view key, std::string_view script);
    virtual void onScriptSaved(std::string_view key, std::string_view script);
    virtual void onScriptUpdate(std::string_view key, std::string_view script);
};

class IVW_MODULE_PYTHON3_API PythonWorkspaceScripts
    : public Observable<PythonWorkspaceScriptsObserver> {
public:
    PythonWorkspaceScripts(WorkspaceManager& manager);

    void addScript(std::string_view key, std::string_view script);

    void updateScript(std::string_view key, std::string_view script);

    void removeScript(std::string_view key);

    std::optional<std::string_view> getScript(std::string_view key);
    std::vector<std::string_view> getKeys() const;

private:
    void notifyObserversOnScriptAdded(std::string_view key, std::string_view script);
    void notifyObserversOnScriptRemoved(std::string_view key, std::string_view script);
    void notifyObserversOnScriptSaved(std::string_view key, std::string_view script);
    void notifyObserversOnScriptUpdate(std::string_view key, std::string_view script);

    WorkspaceManager& manager_;
    std::map<std::string, std::string, std::less<>> pythonScripts_;

    WorkspaceManager::SerializationHandle sHandle_;
    WorkspaceManager::DeserializationHandle dHandle_;
    WorkspaceManager::ClearHandle cHandle_;
};

}  // namespace inviwo
