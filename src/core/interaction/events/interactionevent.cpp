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

#include <inviwo/core/interaction/events/interactionevent.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

const std::string InteractionEvent::modifierNames_[] = {"", "Alt", "Ctrl", "Shift"};

InteractionEvent::InteractionEvent(int modifiers)
    : Event()
    , modifiers_(modifiers) {
}

InteractionEvent::InteractionEvent(const InteractionEvent& rhs)
    : Event(rhs)
    , modifiers_(rhs.modifiers_) {
}

InteractionEvent& InteractionEvent::operator=(const InteractionEvent& that) {
    if (this != &that) {
        Event::operator=(that);
        modifiers_ = that.modifiers_;
    }
    return *this;
}

InteractionEvent* InteractionEvent::clone() const {
    return new InteractionEvent(*this);
}

InteractionEvent::~InteractionEvent() {}

void InteractionEvent::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), true);
    s.serialize("modifiers", modifiers_);
}

void InteractionEvent::deserialize(Deserializer& d) {
    d.deserialize("modifiers", modifiers_);
}

int InteractionEvent::modifiers() const {
    return modifiers_;
}

std::string InteractionEvent::modifierNames() const {
    std::vector<std::string> names;
    if ((modifiers_ & MODIFIER_ALT) == MODIFIER_ALT) names.push_back(modifierNames_[1]);
    if ((modifiers_ & MODIFIER_CTRL) == MODIFIER_CTRL) names.push_back(modifierNames_[2]);
    if ((modifiers_ & MODIFIER_SHIFT) == MODIFIER_SHIFT) names.push_back(modifierNames_[3]);

    if (!names.empty()) {
        return joinString(names, "+");
    } else {
        return "";
    }
}

std::string InteractionEvent::getClassIdentifier() const {
    return "org.inviwo.InteractionEvent";
}

bool InteractionEvent::matching(const Event* aEvent) const {
    const InteractionEvent* event = dynamic_cast<const InteractionEvent*>(aEvent);
    if (event) {
        return modifiers_ == event->modifiers_;
    } else {
        return false;
    }
}

bool InteractionEvent::equalSelectors(const Event* event) const {
    const InteractionEvent* ievent = dynamic_cast<const InteractionEvent*>(event);
    if (ievent) {
        return modifiers_ == ievent->modifiers_;
    } else {
        return false;
    }
}

void InteractionEvent::setModifiers(int modifiers) {
    modifiers_ = modifiers;
}

} // namespace