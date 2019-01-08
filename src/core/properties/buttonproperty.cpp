/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/util/raiiutils.h>

namespace inviwo {

const std::string ButtonProperty::classIdentifier = "org.inviwo.ButtonProperty";
std::string ButtonProperty::getClassIdentifier() const { return classIdentifier; }

ButtonProperty::ButtonProperty(
    std::string identifier, std::string displayName,
    InvalidationLevel invalidationLevel /*=InvalidationLevel::InvalidOutput*/,
    PropertySemantics semantics /*= PropertySemantics::Default*/)
    : Property(identifier, displayName, invalidationLevel, semantics) {
    setValid();  // the initial state for a button should be valid
}

ButtonProperty::ButtonProperty(const ButtonProperty& rhs) : Property(rhs) {}

ButtonProperty& ButtonProperty::operator=(const ButtonProperty& that) {
    if (this != &that) {
        Property::operator=(that);
    }
    return *this;
}

ButtonProperty* ButtonProperty::clone() const { return new ButtonProperty(*this); }

ButtonProperty::~ButtonProperty() {}

void ButtonProperty::set(const Property* src) {
    bool* ptr = nullptr;
    if (auto boolprop = dynamic_cast<const ButtonProperty*>(src)) {
        if (boolprop->buttonPressed_) ptr = &buttonPressed_;
    }
    util::KeepTrueWhileInScope guard(ptr);
    Property::set(src);
}

void ButtonProperty::pressButton() {
    util::KeepTrueWhileInScope guard(&buttonPressed_);
    propertyModified();
}

void ButtonProperty::propertyModified() {
    if (!buttonPressed_) return;
    Property::propertyModified();
}

void ButtonProperty::resetToDefaultState() {}

}  // namespace inviwo
