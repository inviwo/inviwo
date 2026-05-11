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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/scriptproperty.h>

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo {

/**
 * @brief Factory object for creating ScriptProperty backends.
 *
 * Subclass this to provide a script backend implementation for a specific language
 * (e.g., Python, Lua). Register instances with the ScriptBackendFactory via
 * InviwoModule::registerScriptBackend().
 *
 * @see ScriptProperty
 * @see ScriptBackendFactory
 */
class IVW_CORE_API ScriptBackendFactoryObject {
public:
    explicit ScriptBackendFactoryObject(std::string_view classIdentifier);
    virtual ~ScriptBackendFactoryObject();

    /**
     * @brief Create a script backend.
     * @return A callable backend that can execute scripts.
     */
    virtual ScriptProperty::Backend create() const = 0;

    const std::string& getClassIdentifier() const;

private:
    std::string classIdentifier_;
};

/**
 * @brief Factory for ScriptProperty backends.
 *
 * Manages registered ScriptBackendFactoryObjects, allowing backends to be looked up by
 * identifier (e.g., "python"). The ScriptProperty uses this factory to find a suitable
 * backend when none has been explicitly set.
 *
 * @see ScriptBackendFactoryObject
 * @see ScriptProperty
 */
class IVW_CORE_API ScriptBackendFactory {
public:
    ScriptBackendFactory() = default;
    ~ScriptBackendFactory() = default;

    /**
     * @brief Register a backend factory object.
     * The factory does not assume ownership.
     * @return true if registration was successful, false if a backend with the same key exists.
     */
    bool registerObject(ScriptBackendFactoryObject* obj);

    /**
     * @brief Unregister a backend factory object.
     * @return true if unregistration was successful.
     */
    bool unRegisterObject(ScriptBackendFactoryObject* obj);

    /**
     * @brief Check if a backend with the given identifier is registered.
     */
    bool hasKey(std::string_view key) const;

    /**
     * @brief Create a backend by identifier.
     * @return The backend, or a default-constructed (empty) Backend if key is not found.
     */
    ScriptProperty::Backend create(std::string_view key) const;

    /**
     * @brief Create a backend from the first registered factory object.
     *
     * Useful when only one backend is available and the caller doesn't need to specify a key.
     * @return The backend, or a default-constructed (empty) Backend if no backends are registered.
     */
    ScriptProperty::Backend createDefault() const;

    /**
     * @brief Get all registered backend identifiers.
     */
    std::vector<std::string> getKeys() const;

private:
    std::map<std::string, ScriptBackendFactoryObject*, std::less<>> map_;
};

}  // namespace inviwo
