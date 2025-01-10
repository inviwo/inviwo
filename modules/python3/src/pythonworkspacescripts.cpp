/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <modules/python3/pythonworkspacescripts.h>

namespace inviwo {

void PythonWorkspaceScriptsObserver::onScriptAdded(std::string_view, std::string_view){};
void PythonWorkspaceScriptsObserver::onScriptRemoved(std::string_view, std::string_view){};
void PythonWorkspaceScriptsObserver::onScriptSaved(std::string_view, std::string_view){};
void PythonWorkspaceScriptsObserver::onScriptUpdate(std::string_view, std::string_view){};

PythonWorkspaceScripts::PythonWorkspaceScripts(WorkspaceManager& manager)
    : manager_{manager}
    , sHandle_{manager.onSave(
          [this](Serializer& s) {
              s.serialize("PythonScripts", pythonScripts_, "Script");

              for (auto& [key, script] : pythonScripts_) {
                  notifyObserversOnScriptSaved(key, script);
              }
          },
          WorkspaceSaveMode::Disk)}
    , dHandle_{manager.onLoad([this](Deserializer& d) {
        d.deserialize("PythonScripts", pythonScripts_, "Script",
                      deserializer::MapFunctions{
                          .idTransform = [](std::string_view id) { return std::string{id}; },
                          .makeNew = []() { return std::string{}; },
                          .onNew = [&](const std::string& key,
                                       std::string& script) { addScript(key, script); },
                          .onRemove = [&](const std::string& key) { removeScript(key); }});
    })}
    , cHandle_{manager.onClear([this]() {
        while (!pythonScripts_.empty()) {
            auto it = pythonScripts_.begin();
            notifyObserversOnScriptRemoved(it->first, it->second);
            pythonScripts_.erase(it);
        }
    })} {}

void PythonWorkspaceScripts::addScript(std::string_view key, std::string_view script) {
    if (pythonScripts_.emplace(key, script).second) {
        notifyObserversOnScriptAdded(key, script);
        manager_.setModified();
    }
}

void PythonWorkspaceScripts::updateScript(std::string_view key, std::string_view script) {
    if (auto it = pythonScripts_.find(key); it != pythonScripts_.end()) {
        if (it->second != script) {
            it->second = script;
            notifyObserversOnScriptUpdate(it->first, it->second);
            manager_.setModified();
        }
    }
}

void PythonWorkspaceScripts::removeScript(std::string_view key) {
    if (auto it = pythonScripts_.find(key); it != pythonScripts_.end()) {
        notifyObserversOnScriptRemoved(it->first, it->second);
        pythonScripts_.erase(it);
        manager_.setModified();
    }
}

std::optional<std::string_view> PythonWorkspaceScripts::getScript(std::string_view key) {
    if (auto it = pythonScripts_.find(key); it != pythonScripts_.end()) {
        return it->second;
    } else {
        return std::nullopt;
    }
}
std::vector<std::string_view> PythonWorkspaceScripts::getKeys() const {
    std::vector<std::string_view> keys;
    keys.reserve(pythonScripts_.size());
    for (const auto& [key, script] : pythonScripts_) {
        keys.push_back(key);
    }
    return keys;
}

void PythonWorkspaceScripts::notifyObserversOnScriptAdded(std::string_view key,
                                                          std::string_view script) {
    forEachObserver([&](PythonWorkspaceScriptsObserver* o) { o->onScriptAdded(key, script); });
}
void PythonWorkspaceScripts::notifyObserversOnScriptRemoved(std::string_view key,
                                                            std::string_view script) {
    forEachObserver([&](PythonWorkspaceScriptsObserver* o) { o->onScriptRemoved(key, script); });
}
void PythonWorkspaceScripts::notifyObserversOnScriptSaved(std::string_view key,
                                                          std::string_view script) {
    forEachObserver([&](PythonWorkspaceScriptsObserver* o) { o->onScriptSaved(key, script); });
}
void PythonWorkspaceScripts::notifyObserversOnScriptUpdate(std::string_view key,
                                                           std::string_view script) {
    forEachObserver([&](PythonWorkspaceScriptsObserver* o) { o->onScriptUpdate(key, script); });
}

}  // namespace inviwo
