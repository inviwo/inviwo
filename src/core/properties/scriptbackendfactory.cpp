/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/properties/scriptbackendfactory.h>

#include <inviwo/core/util/logcentral.h>

#include <algorithm>

namespace inviwo {

ScriptBackendFactoryObject::ScriptBackendFactoryObject(std::string_view classIdentifier)
    : classIdentifier_{classIdentifier} {}

ScriptBackendFactoryObject::~ScriptBackendFactoryObject() = default;

const std::string& ScriptBackendFactoryObject::getClassIdentifier() const {
    return classIdentifier_;
}

bool ScriptBackendFactory::registerObject(ScriptBackendFactoryObject* obj) {
    auto [it, inserted] = map_.emplace(obj->getClassIdentifier(), obj);
    if (!inserted) {
        log::warn("ScriptBackendFactory: Failed to register backend \"{}\", already registered",
                  obj->getClassIdentifier());
    }
    return inserted;
}

bool ScriptBackendFactory::unRegisterObject(ScriptBackendFactoryObject* obj) {
    size_t removed =
        std::erase_if(map_, [obj](const auto& elem) { return elem.second == obj; });
    return removed > 0;
}

bool ScriptBackendFactory::hasKey(std::string_view key) const {
    return map_.find(key) != map_.end();
}

ScriptProperty::Backend ScriptBackendFactory::create(std::string_view key) const {
    auto it = map_.find(key);
    if (it != map_.end()) {
        return it->second->create();
    }
    return {};
}

ScriptProperty::Backend ScriptBackendFactory::createDefault() const {
    if (!map_.empty()) {
        return map_.begin()->second->create();
    }
    return {};
}

std::vector<std::string> ScriptBackendFactory::getKeys() const {
    std::vector<std::string> keys;
    keys.reserve(map_.size());
    for (const auto& [key, _] : map_) {
        keys.push_back(key);
    }
    return keys;
}

}  // namespace inviwo
