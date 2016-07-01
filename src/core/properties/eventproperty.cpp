/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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

EventProperty::EventProperty(std::string identifier, std::string displayName,
                             std::unique_ptr<EventMatcher> matcher, Action action,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : Property(identifier, displayName, invalidationLevel, semantics)
    , event_{std::move(matcher)}
    , action_{std::move(action)} {}

EventProperty::EventProperty(const EventProperty& rhs)
    : Property(rhs)
    , event_{rhs.event_ ? rhs.event_->clone() : nullptr}
    , action_{rhs.action_} {}

EventProperty& EventProperty::operator=(const EventProperty& that) {
    if (this != &that) {
        Property::operator=(that);

        std::unique_ptr<EventMatcher> e{that.event_ ? that.event_->clone() : nullptr};
        Action a{that.action_};

        std::swap(event_, e);
        std::swap(action_, a);
    }
    return *this;
}

EventProperty* EventProperty::clone() const { return new EventProperty(*this); }

void EventProperty::serialize(Serializer& s) const {
    Property::serialize(s);
    if (this->serializationMode_ == PropertySerializationMode::None) return;
    
    s.serialize("Event", event_.get());
}

void EventProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);
    EventMatcher* e = event_.get();
    d.deserialize("Event", e); // e has to be a lvalue.
    if(e != event_.get()) event_.reset(e);
}

EventMatcher* EventProperty::getEvent() const {
    return event_.get();
}

EventProperty::Action EventProperty::getAction() const {
    return action_;
}

void EventProperty::setEvent(std::unique_ptr<EventMatcher> matcher) {
    event_ = std::move(matcher);
}

void EventProperty::setAction(Action action) {
    action_ = std::move(action);
}

void EventProperty::setCurrentStateAsDefault() {
    if (event_) event_->setCurrentStateAsDefault();
}

void EventProperty::resetToDefaultState() {
    if (event_) event_->resetToDefaultState();
}

}  // namespace