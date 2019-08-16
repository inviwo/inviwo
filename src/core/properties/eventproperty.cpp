/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

const std::string EventProperty::classIdentifier = "org.inviwo.EventProperty";
std::string EventProperty::getClassIdentifier() const { return classIdentifier; }

EventProperty::EventProperty(const std::string& identifier, const std::string& displayName,
                             Action action, std::unique_ptr<EventMatcher> matcher,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : Property(identifier, displayName, invalidationLevel, semantics)
    , matcher_{std::move(matcher)}
    , action_{std::move(action)} {}

EventProperty::EventProperty(const std::string& identifier, const std::string& displayName,
                             Action action, IvwKey key, KeyStates states, KeyModifiers modifiers,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : EventProperty(identifier, displayName, std::move(action),
                    std::make_unique<KeyboardEventMatcher>(key, states, modifiers),
                    invalidationLevel, semantics) {}

EventProperty::EventProperty(const std::string& identifier, const std::string& displayName,
                             Action action, MouseButtons buttons, MouseStates states,
                             KeyModifiers modifiers, InvalidationLevel invalidationLevel,
                             PropertySemantics semantics)
    : EventProperty(identifier, displayName, std::move(action),
                    std::make_unique<MouseEventMatcher>(buttons, states, modifiers),
                    invalidationLevel, semantics) {}

EventProperty::EventProperty(const EventProperty& rhs)
    : Property(rhs)
    , matcher_{rhs.matcher_ ? rhs.matcher_->clone() : nullptr}
    , action_{rhs.action_}
    , enabled_{rhs.enabled_} {}

EventProperty* EventProperty::clone() const { return new EventProperty(*this); }

void EventProperty::invokeEvent(Event* e) {
    if (enabled_ && (*matcher_)(e)) action_(e);
}

EventMatcher* EventProperty::getEventMatcher() const { return matcher_.get(); }

EventProperty::Action EventProperty::getAction() const { return action_; }

bool EventProperty::isEnabled() const { return enabled_; }

void EventProperty::setEnabled(bool enabled) { enabled_ = enabled; }

void EventProperty::setEventMatcher(std::unique_ptr<EventMatcher> matcher) {
    matcher_ = std::move(matcher);
}

void EventProperty::setAction(Action action) { action_ = std::move(action); }

EventProperty& EventProperty::setCurrentStateAsDefault() {
    if (matcher_) matcher_->setCurrentStateAsDefault();
    return *this;
}

EventProperty& EventProperty::resetToDefaultState() {
    if (matcher_) matcher_->resetToDefaultState();
    return *this;
}

void EventProperty::serialize(Serializer& s) const {
    Property::serialize(s);
    if (this->serializationMode_ == PropertySerializationMode::None) return;

    s.serialize("Event", matcher_.get());
}

void EventProperty::deserialize(Deserializer& d) {
    Property::deserialize(d);
    EventMatcher* e = matcher_.get();
    d.deserialize("Event", e);  // e has to be a lvalue.
    if (e != matcher_.get()) matcher_.reset(e);
}

}  // namespace inviwo
