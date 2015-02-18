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

#include <inviwo/core/interaction/events/touchevent.h>

namespace inviwo {

TouchEvent::TouchEvent(ivec2 position, TouchEvent::TouchState state)
    : InteractionEvent(), position_(position), state_(state) {}


TouchEvent::TouchEvent(const TouchEvent& rhs)
    : InteractionEvent(rhs)
    , position_(rhs.position_)
    , state_(rhs.state_) {
}

TouchEvent& TouchEvent::operator=(const TouchEvent& that) {
    if (this != &that) {
        InteractionEvent::operator=(that);
        position_ = that.position_;
        state_ = that.state_;
    }
    return *this;
}

TouchEvent* TouchEvent::clone() const {
    return new TouchEvent(*this);
}

TouchEvent::~TouchEvent() {}

void TouchEvent::serialize(IvwSerializer& s) const { InteractionEvent::serialize(s); }

void TouchEvent::deserialize(IvwDeserializer& d) { InteractionEvent::deserialize(d); }

bool TouchEvent::matching(const Event* aEvent) const {
    const TouchEvent* event = dynamic_cast<const TouchEvent*>(aEvent);
    if (event) {
        return matching(event);
    } else {
        return false;
    }
}

bool TouchEvent::matching(const TouchEvent* aEvent) const {
    return state_ == aEvent->state_
        && modifiers_ == aEvent->modifiers_;
}

bool TouchEvent::equalSelectors(const Event* aEvent) const {
    const TouchEvent* event = dynamic_cast<const TouchEvent*>(aEvent);
    if (event) {
        return InteractionEvent::equalSelectors(event)
            && state_ == event->state_;
    } else {
        return false;
    }
}


}  // namespace