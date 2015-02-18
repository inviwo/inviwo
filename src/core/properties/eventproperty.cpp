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
    , event_(e)
    , defaultEvent_(e->clone())
    , action_(action) {
}

EventProperty::EventProperty(const EventProperty& rhs)
    : Property(rhs)
    , event_(rhs.event_->clone())
    , defaultEvent_(rhs.defaultEvent_->clone())
    , action_(rhs.action_->clone()) {}

EventProperty& EventProperty::operator=(const EventProperty& that) {
    if (this != &that) {
        Property::operator=(that);
        if (event_) delete event_;
        event_ = that.event_->clone();
        if (defaultEvent_) delete defaultEvent_;
        defaultEvent_ = that.defaultEvent_->clone();
        if (action_) delete action_;
        action_ = that.action_->clone();
    }
    return *this;
}

EventProperty* EventProperty::clone() const { return new EventProperty(*this); }

EventProperty::~EventProperty() {
    if (event_) delete event_;
    if (defaultEvent_) delete defaultEvent_;
    if (action_) delete action_;
}

void EventProperty::serialize(IvwSerializer& s) const {
    Property::serialize(s);
    if(!event_->equalSelectors(defaultEvent_))
        s.serialize("Event", event_);
}

void EventProperty::deserialize(IvwDeserializer& d) {
    Property::deserialize(d);
    d.deserialize("Event", event_);
}

InteractionEvent* EventProperty::getEvent() const {
    return event_;
}

Action* EventProperty::getAction() const {
    return action_;
}

void EventProperty::setEvent(InteractionEvent* e) {
    if (event_ && event_ != e) delete event_;
    event_ = e;
}

void EventProperty::setAction(Action* action) {
    if (action_ && action_ != action) delete action_;
    action_ = action;
}

void EventProperty::setCurrentStateAsDefault() {
    if (defaultEvent_) delete defaultEvent_;
    defaultEvent_ = event_->clone();
}

void EventProperty::resetToDefaultState() {
    if (event_) delete event_;
    event_ = defaultEvent_->clone();
}

}  // namespace