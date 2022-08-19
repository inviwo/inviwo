/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#include <inviwo/core/properties/buttongroupproperty.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

const std::string ButtonGroupProperty::classIdentifier = "org.inviwo.ButtonGroupProperty";
std::string ButtonGroupProperty::getClassIdentifier() const { return classIdentifier; }

ButtonGroupProperty::ButtonGroupProperty(std::string_view identifier, std::string_view displayName,
                                         Document help, std::vector<Button> buttons,
                                         InvalidationLevel invalidationLevel,
                                         PropertySemantics semantics)
    : Property(identifier, displayName, {}, invalidationLevel, semantics)
    , buttons_{std::move(buttons)}
    , partialHelp_{std::move(help)} {
    setValid();  // the initial state for a button should be valid

    updateHelp();
}

void ButtonGroupProperty::updateHelp() {
    getHelp() = Document{};

    getHelp().append(partialHelp_);
    auto list = getHelp().handle().append("ul");
    for (auto&& [i, button] : util::enumerate(buttons_)) {
        auto item = list.append("li");
        if (button.icon) {
            item.append("img", "", {{"src", *button.icon}, {"width", "20"}});
        }
        if (button.name) {
            item += *button.name;
        }
        item += " ";
        if (button.tooltip) {
            item += *button.tooltip;
        }
    }
}

ButtonGroupProperty::ButtonGroupProperty(std::string_view identifier, std::string_view displayName,
                                         InvalidationLevel invalidationLevel,
                                         PropertySemantics semantics)
    : ButtonGroupProperty(identifier, displayName, Document{}, std::vector<Button>{},
                          invalidationLevel, semantics) {}

ButtonGroupProperty::ButtonGroupProperty(std::string_view identifier, std::string_view displayName,
                                         Document help, InvalidationLevel invalidationLevel,
                                         PropertySemantics semantics)
    : ButtonGroupProperty(identifier, displayName, std::move(help), std::vector<Button>{},
                          invalidationLevel, semantics) {}

ButtonGroupProperty::ButtonGroupProperty(std::string_view identifier, std::string_view displayName,
                                         std::vector<Button> buttons,
                                         InvalidationLevel invalidationLevel,
                                         PropertySemantics semantics)
    : ButtonGroupProperty(identifier, displayName, Document{}, std::move(buttons),
                          invalidationLevel, semantics) {}

ButtonGroupProperty::ButtonGroupProperty(const ButtonGroupProperty& rhs)
    : Property(rhs), buttons_{} {
    setValid();
}

ButtonGroupProperty::ButtonGroupProperty(const ButtonGroupProperty& rhs,
                                         std::vector<Button> buttons)
    : Property(rhs), buttons_{std::move(buttons)} {}

ButtonGroupProperty* ButtonGroupProperty::clone() const { return new ButtonGroupProperty(*this); }

void ButtonGroupProperty::addButton(Button button) {
    buttons_.push_back(std::move(button));
    updateHelp();
}

auto ButtonGroupProperty::getButton(size_t i) const -> const Button& { return buttons_[i]; }

size_t ButtonGroupProperty::size() const { return buttons_.size(); }

void ButtonGroupProperty::set(const Property* src) {
    if (auto prop = dynamic_cast<const ButtonGroupProperty*>(src)) {
        pressButton(prop->buttonPressed_);
    }
}

void ButtonGroupProperty::pressButton(size_t index) {
    if (index < buttons_.size()) {
        buttonPressed_ = index;
        buttons_[index].action();
        Property::propertyModified();
        buttonPressed_ = std::numeric_limits<size_t>::max();
    }
}

ButtonGroupProperty& ButtonGroupProperty::propertyModified() {
    if (buttonPressed_ < buttons_.size()) {
        Property::propertyModified();
    }
    return *this;
}

ButtonGroupProperty& ButtonGroupProperty::resetToDefaultState() { return *this; }

}  // namespace inviwo
