/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/interaction/events/keyboardevent.h>

namespace inviwo {

KeyboardEvent::KeyboardEvent(int key, int modifiers, int state)
    : InteractionEvent(modifiers)
    , state_(state)
    , key_(key) {
}

KeyboardEvent::KeyboardEvent(const KeyboardEvent& rhs) 
    : InteractionEvent(rhs)
    , state_(rhs.state_)
    , key_(rhs.key_) {
}

KeyboardEvent& KeyboardEvent::operator=(const KeyboardEvent& that) {
    if (this != &that) {
        InteractionEvent::operator=(that);
        state_ = that.state_;
        key_ = that.key_;
    }
    return *this;
}

KeyboardEvent* KeyboardEvent::clone() const { return new KeyboardEvent(*this); }

KeyboardEvent::~KeyboardEvent() {}

void KeyboardEvent::serialize(IvwSerializer& s) const {
    InteractionEvent::serialize(s);
    s.serialize("state", state_);
    s.serialize("key", key_);
}

void KeyboardEvent::deserialize(IvwDeserializer& d) {
    InteractionEvent::deserialize(d);
    d.deserialize("state", state_);
    d.deserialize("key", key_);
}

std::string KeyboardEvent::getClassIdentifier() const {
    return "org.inviwo.KeyboardEvent";
}

int KeyboardEvent::state() const {
    return state_;
}

int KeyboardEvent::button() const {
    return key_;
}

bool KeyboardEvent::matching(const Event* aEvent) const {
    const KeyboardEvent* event = dynamic_cast<const KeyboardEvent*>(aEvent);
    if (event) {
        return matching(event);
    } else {
        return false;
    }
}

bool KeyboardEvent::matching(const KeyboardEvent* aEvent) const {
    return key_ == aEvent->key_
        && (state_ & aEvent->state_) == aEvent->state_
        && modifiers_ == aEvent->modifiers_;
}

bool KeyboardEvent::equalSelectors(const Event* aEvent) const {
    const KeyboardEvent* event = dynamic_cast<const KeyboardEvent*>(aEvent);
    if (event) {
        return InteractionEvent::equalSelectors(event)
            && state_ == event->state_
            && key_ == event->key_;
    } else {
        return false;
    }
}

void KeyboardEvent::setState(int state) {
    state_ = state;
}

void KeyboardEvent::setButton(int button) {
    key_ = button;
}

}  // namespace