/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#include <inviwo/core/properties/stringproperty.h>

namespace inviwo {

const std::string StringProperty::classIdentifier = "org.inviwo.StringProperty";
std::string StringProperty::getClassIdentifier() const { return classIdentifier; }

StringProperty::StringProperty(std::string_view identifier, std::string_view displayName,
                               Document help, std::string_view value,
                               InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : TemplateProperty<std::string>(identifier, displayName, std::move(help), std::string{value},
                                    invalidationLevel, semantics) {}

StringProperty::StringProperty(std::string_view identifier, std::string_view displayName,
                               std::string_view value, InvalidationLevel invalidationLevel,
                               PropertySemantics semantics)
    : StringProperty(identifier, displayName, {}, value, invalidationLevel, semantics) {}

StringProperty::StringProperty(const StringProperty& rhs) : TemplateProperty<std::string>(rhs) {}

StringProperty& StringProperty::operator=(const std::string& value) {
    TemplateProperty<std::string>::operator=(value);
    return *this;
}

StringProperty* StringProperty::clone() const { return new StringProperty(*this); }

StringProperty& StringProperty::set(std::string_view value) {
    if (value_.update(value)) {
        propertyModified();
    }
    return *this;
}

StringProperty& StringProperty::set(const char* value) { return set(std::string_view{value}); }

StringProperty& StringProperty::setDefault(std::string_view value) {
    value_.defaultValue = value;
    return *this;
}

StringProperty::operator std::string_view() const { return value_.value; }

Document StringProperty::getDescription() const {
    Document doc = TemplateProperty<std::string>::getDescription();
    doc.append("pre", value_.value);
    return doc;
}

}  // namespace inviwo
