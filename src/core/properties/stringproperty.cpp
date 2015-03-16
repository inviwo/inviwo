/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

PropertyClassIdentifier(StringProperty, "org.inviwo.StringProperty");

StringProperty::StringProperty(std::string identifier, std::string displayName, std::string value,
                               InvalidationLevel invalidationLevel,
                               PropertySemantics semantics)
    : TemplateProperty<std::string>(identifier, displayName, value, invalidationLevel, semantics) {}

StringProperty::StringProperty(const StringProperty& rhs)
    : TemplateProperty<std::string>(rhs)    {
}

StringProperty& StringProperty::operator=(const StringProperty& that) {
    if (this != &that) {
        TemplateProperty<std::string>::operator=(that);
    }
    return *this;
}

StringProperty& StringProperty::operator=(const std::string& value) {
    TemplateProperty<std::string>::operator=(value);
    return *this;
}

StringProperty* StringProperty::clone() const {
    return new StringProperty(*this);
}

StringProperty::~StringProperty() {}

}  // namespace
