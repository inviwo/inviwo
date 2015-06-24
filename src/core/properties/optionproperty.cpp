/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {
PropertyClassIdentifier(OptionPropertyString, "org.inviwo.OptionPropertyString");

void OptionPropertyString::addOption(std::string identifier, std::string displayName,
                                     std::string value) {
    TemplateOptionProperty<std::string>::addOption(identifier, displayName, value);
}

void OptionPropertyString::addOption(std::string identifier, std::string displayName) {
    TemplateOptionProperty<std::string>::addOption(identifier, displayName, identifier);
}

OptionPropertyString::OptionPropertyString(
    std::string identifier, std::string displayName,
    InvalidationLevel invalidationLevel /*= INVALID_OUTPUT*/,
    PropertySemantics semantics /*= PropertySemantics::Default*/)
    : TemplateOptionProperty<std::string>(identifier, displayName, invalidationLevel, semantics) {}

OptionPropertyString::OptionPropertyString(const OptionPropertyString& rhs)
    : TemplateOptionProperty<std::string>(rhs) {}

OptionPropertyString& OptionPropertyString::operator=(const OptionPropertyString& that) {
    if (this != &that) {
        TemplateOptionProperty<std::string>::operator=(that);
    }
    return *this;
}

OptionPropertyString* OptionPropertyString::clone() const {
    return new OptionPropertyString(*this);
}

OptionPropertyString::~OptionPropertyString() {}

BaseOptionProperty::BaseOptionProperty(const BaseOptionProperty& rhs) : Property(rhs) {}

BaseOptionProperty::BaseOptionProperty(std::string identifier, std::string displayName,
                                       InvalidationLevel invalidationLevel /*=INVALID_OUTPUT*/,
                                       PropertySemantics semantics /*=PropertySemantics::Default*/)
    : Property(identifier, displayName, invalidationLevel, semantics) {}

BaseOptionProperty::~BaseOptionProperty() {}

void BaseOptionProperty::set(const Property* srcProperty) {
    if (auto optionSrcProp = dynamic_cast<const BaseOptionProperty*>(srcProperty)) {
        size_t option = optionSrcProp->getSelectedIndex();

        if (option < size()) {
            setSelectedIndex(option);
        } else {
            setSelectedIndex(size() - 1);
        }
        propertyModified();
    }
}

}  // namespace
