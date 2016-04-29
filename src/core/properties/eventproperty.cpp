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

#include <inviwo/core/properties/eventproperty.h>

namespace inviwo {

PropertyClassIdentifier(EventProperty, "org.inviwo.EventProperty");

EventProperty::EventProperty(std::string identifier, std::string displayName, InteractionEvent* e,
                             Action* action, InvalidationLevel invalidationLevel,
                             PropertySemantics semantics)
    : Property(identifier, displayName, invalidationLevel, semantics)
    , event_{e}
    , defaultEvent_{e ? e->clone() : nullptr}
    , action_{action} {}

EventProperty::EventProperty(const EventProperty& rhs)
    : Property(rhs)
    , event_{rhs.event_ ? rhs.event_->clone() : nullptr}
    , defaultEvent_{rhs.defaultEvent_ ? rhs.defaultEvent_->clone() : nullptr}
    , action_{rhs.action_ ? rhs.action_->clone() : nullptr} {}

EventProperty& EventProperty::operator=(const EventProperty& that) {
    if (this != &that) {
        Property::operator=(that);

        std::unique_ptr<InteractionEvent> e{that.event_ ? that.event_->clone() : nullptr};
        std::unique_ptr<InteractionEvent> d{that.defaultEvent_ ? that.defaultEvent_->clone()
                                                               : nullptr};
        std::unique_ptr<Action> a{that.action_ ? that.action_->clone() : nullptr};

        std::swap(event_, e);
        std::swap(defaultEvent_, d);
        std::swap(action_, a);
    }
    return *this;
}

EventProperty* EventProperty::clone() const { return new EventProperty(*this); }

void EventProperty::serialize(Serializer& s) const {
    Property::serialize(s);
    if (this->serializationMode_ == PropertySerializationMode::None) return;
    if (!event_->equalSelectors(defaultEvent_.get())) {
        s.serialize("Event", event_.get());
    }
}

void EventProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);
    InteractionEvent* e = event_.get();
    d.deserialize("Event", e); // e has to be a lvalue.
    if(e != event_.get()) event_.reset(e);
}

InteractionEvent* EventProperty::getEvent() const {
    return event_.get();
}

Action* EventProperty::getAction() const {
    return action_.get();
}

void EventProperty::setEvent(InteractionEvent* e) {
    if (e != event_.get()) event_.reset(e);
}

void EventProperty::setAction(Action* action) {
    if (action != action_.get()) action_.reset(action);
}

void EventProperty::setCurrentStateAsDefault() {
    defaultEvent_.reset(event_ ? event_->clone() : nullptr);
}

void EventProperty::resetToDefaultState() {
    event_.reset(defaultEvent_ ? defaultEvent_->clone() : nullptr);
}

}  // namespace