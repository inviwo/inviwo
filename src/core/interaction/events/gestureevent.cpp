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

#include <inviwo/core/interaction/events/gestureevent.h>

namespace inviwo {

GestureEvent::GestureEvent(vec2 deltaPos, double deltaDistance, GestureEvent::GestureType type,
                           int state, int numFingers, vec2 screenPosNorm, uvec2 canvasSize)
    : InteractionEvent()
    , type_(type)
    , state_(state)
    , numFingers_(numFingers)
    , deltaPos_(deltaPos)
    , deltaDistance_(deltaDistance)
    , screenPosNorm_(screenPosNorm)
    , canvasSize_(canvasSize)
    {}

GestureEvent::GestureEvent(GestureEvent::GestureType type,
                           int state, int numFingers)
    : InteractionEvent()
    , type_(type)
    , state_(state)
    , numFingers_(numFingers)
    , deltaPos_(0)
    , deltaDistance_(0)
    , screenPosNorm_(0)
    , canvasSize_(0) {}


GestureEvent::GestureEvent(const GestureEvent& rhs) 
    : InteractionEvent(rhs)
    , type_(rhs.type_)
    , state_(rhs.state_)
    , numFingers_(rhs.numFingers_)
    , deltaPos_(rhs.deltaPos_)
    , deltaDistance_(rhs.deltaDistance_)
    , screenPosNorm_(rhs.screenPosNorm_)
    , canvasSize_(rhs.canvasSize_){
}

GestureEvent& GestureEvent::operator=(const GestureEvent& that) {
    if (this != &that) {
        InteractionEvent::operator=(that);
        type_ = that.type_;
        state_ = that.state_;
        numFingers_ = that.numFingers_;
        deltaPos_ = that.deltaPos_;
        deltaDistance_ = that.deltaDistance_;
        screenPosNorm_ = that.screenPosNorm_;
        canvasSize_ = that.canvasSize_;
    }
    return *this;
}

GestureEvent* GestureEvent::clone() const { return new GestureEvent(*this); }

GestureEvent::~GestureEvent() {}

void GestureEvent::modify(vec2 posNorm) { screenPosNorm_ = posNorm; }

void GestureEvent::serialize(IvwSerializer& s) const { InteractionEvent::serialize(s); }

void GestureEvent::deserialize(IvwDeserializer& d) { InteractionEvent::deserialize(d); }

bool GestureEvent::matching(const Event* aEvent) const {
    const GestureEvent* event = dynamic_cast<const GestureEvent*>(aEvent);
    if (event) {
        return matching(event);
    } else {
        return false;
    }
}

bool GestureEvent::matching(const GestureEvent* aEvent) const {
    return type_ == aEvent->type_
        && (state_ & aEvent->state_) == aEvent->state_ // aEvent.state equal to any of this.state.
        && numFingers_ == aEvent->numFingers_
        && modifiers_ == aEvent->modifiers_;
}

bool GestureEvent::equalSelectors(const Event* aEvent) const {
    const GestureEvent* event = dynamic_cast<const GestureEvent*>(aEvent);
    if (event) {
        return InteractionEvent::equalSelectors(event)
            && type_ == event->type_
            && state_ == event->state_
            && numFingers_ == event->numFingers_;
    } else {
        return false;
    }
}

}  // namespace