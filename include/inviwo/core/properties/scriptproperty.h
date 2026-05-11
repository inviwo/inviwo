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
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/valuewrapper.h>

#include <any>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo {

/**
 * @ingroup properties
 * @brief A property that stores script source code and supports pluggable execution backends.
 *
 * The ScriptProperty holds a script source as a string and provides a mechanism for executing
 * the script via a pluggable backend. This allows processors to use scripting without depending
 * on any specific scripting language implementation (e.g., Python).
 *
 * A backend is a callable that takes the script source and a vector of arguments (as std::any)
 * and returns a result (as std::any). Different scripting modules (e.g., Python3) can set their
 * own backend to provide language-specific execution.
 *
 * Usage example in a processor:
 * @code
 * class MyProcessor : public Processor {
 * public:
 *     MyProcessor()
 *         : script_{"script", "Script", "def transform(x):\n    return x * 2\n"} {
 *         addProperty(script_);
 *     }
 *
 *     void process() override {
 *         auto result = script_.eval<double>(42.0);
 *         // ...
 *     }
 *
 * private:
 *     ScriptProperty script_;
 * };
 * @endcode
 *
 * @see PythonScriptPropertyWidgetQt
 */
class IVW_CORE_API ScriptProperty : public Property {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.ScriptProperty"};

    /**
     * The backend callable type. Takes the script source and a vector of arguments,
     * returns a result as std::any. A backend returning a default-constructed std::any
     * indicates a void return.
     */
    using Backend = std::function<std::any(const std::string&, const std::vector<std::any>&)>;

    ScriptProperty(std::string_view identifier, std::string_view displayName, Document help,
                   std::string_view source = "",
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    ScriptProperty(std::string_view identifier, std::string_view displayName,
                   std::string_view source = "",
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    ScriptProperty(const ScriptProperty& rhs);
    virtual ScriptProperty* clone() const override;
    virtual ~ScriptProperty() = default;

    /**
     * @brief Set the script source code.
     * Triggers property modification notification.
     */
    ScriptProperty& setSource(std::string_view source);

    /**
     * @brief Get the current script source code.
     */
    const std::string& getSource() const;

    /**
     * @brief Set the default script source code.
     */
    ScriptProperty& setDefaultSource(std::string_view source);

    /**
     * @brief Set the execution backend.
     * The backend will be called when eval() or call() is invoked.
     * Not serialized; must be set at runtime by the scripting module.
     */
    void setBackend(Backend backend);

    /**
     * @brief Check if a backend is available.
     */
    bool hasBackend() const;

    /**
     * @brief Call the script with the given arguments.
     * @param args Arguments passed to the backend as a vector of std::any.
     * @return The result of the script execution as std::any.
     * @throws Exception if no backend is set.
     */
    std::any call(std::vector<std::any> args = {}) const;

    /**
     * @brief Convenience template for calling the script with typed arguments.
     * @tparam R The expected return type.
     * @tparam Args The argument types.
     * @param args Arguments passed to the script.
     * @return The result cast to type R.
     * @throws Exception if no backend is set.
     * @throws std::bad_any_cast if the result cannot be cast to R.
     */
    template <typename R, typename... Args>
    R eval(Args&&... args) const {
        return std::any_cast<R>(call({std::any(std::forward<Args>(args))...}));
    }

    virtual ScriptProperty& setCurrentStateAsDefault() override;
    virtual ScriptProperty& resetToDefaultState() override;
    virtual bool isDefaultState() const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual Document getDescription() const override;

private:
    ValueWrapper<std::string> source_;
    Backend backend_;
};

}  // namespace inviwo
