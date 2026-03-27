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

#include <inviwo/core/properties/scriptproperty.h>

#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/properties/scriptbackendfactory.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

std::string_view ScriptProperty::getClassIdentifier() const { return classIdentifier; }

ScriptProperty::ScriptProperty(std::string_view identifier, std::string_view displayName,
                               Document help, std::string_view source,
                               InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : Property(identifier, displayName, std::move(help), invalidationLevel, semantics)
    , source_("source", std::string{source}) {}

ScriptProperty::ScriptProperty(std::string_view identifier, std::string_view displayName,
                               std::string_view source, InvalidationLevel invalidationLevel,
                               PropertySemantics semantics)
    : ScriptProperty(identifier, displayName, {}, source, invalidationLevel, semantics) {}

ScriptProperty::ScriptProperty(const ScriptProperty& rhs)
    : Property(rhs), source_(rhs.source_) {}

ScriptProperty* ScriptProperty::clone() const { return new ScriptProperty(*this); }

ScriptProperty& ScriptProperty::setSource(std::string_view source) {
    if (source_.update(source)) {
        propertyModified();
    }
    return *this;
}

const std::string& ScriptProperty::getSource() const { return source_.value; }

ScriptProperty& ScriptProperty::setDefaultSource(std::string_view source) {
    source_.defaultValue = source;
    return *this;
}

void ScriptProperty::setBackend(Backend backend) { backend_ = std::move(backend); }

bool ScriptProperty::hasBackend() const {
    if (backend_) return true;
    if (auto* factory = util::getScriptBackendFactory(const_cast<ScriptProperty*>(this))) {
        return !factory->getKeys().empty();
    }
    return false;
}

std::any ScriptProperty::call(std::vector<std::any> args) const {
    if (backend_) {
        return backend_(source_.value, args);
    }

    // Fall back to the factory
    if (auto* factory = util::getScriptBackendFactory(const_cast<ScriptProperty*>(this))) {
        auto backend = factory->createDefault();
        if (backend) {
            return backend(source_.value, args);
        }
    }

    throw Exception(IVW_CONTEXT, "No script backend set for ScriptProperty '{}'",
                    getDisplayName());
}

ScriptProperty& ScriptProperty::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    source_.setAsDefault();
    return *this;
}

ScriptProperty& ScriptProperty::resetToDefaultState() {
    if (source_.reset()) {
        propertyModified();
    }
    return *this;
}

bool ScriptProperty::isDefaultState() const { return source_.isDefault(); }

void ScriptProperty::serialize(Serializer& s) const {
    Property::serialize(s);
    source_.serialize(s, serializationMode_);
}

void ScriptProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);
    bool modified = source_.deserialize(d, serializationMode_);
    if (modified) propertyModified();
}

Document ScriptProperty::getDescription() const {
    Document doc = Property::getDescription();
    doc.append("pre", source_.value);
    return doc;
}

}  // namespace inviwo
