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

TouchPoint::TouchPoint(vec2 pos, vec2 posNormalized, vec2 prevPos, vec2 prevPosNormalized)
    : pos_(pos), posNormalized_(posNormalized), prevPos_(prevPos), prevPosNormalized_(prevPosNormalized) {

}

void TouchPoint::serialize(IvwSerializer& s) const {
    s.serialize("pos", pos_);
    s.serialize("posNormalized", posNormalized_);
    s.serialize("prevPos", prevPos_);
    s.serialize("prevPosNormalized", prevPosNormalized_);

}

void TouchPoint::deserialize(IvwDeserializer& d) {
    d.deserialize("pos", pos_);
    d.deserialize("posNormalized", posNormalized_);
    d.deserialize("prevPos", prevPos_);
    d.deserialize("prevPosNormalized", prevPosNormalized_);
}

TouchEvent::TouchEvent(TouchEvent::TouchState state, uvec2 canvasSize)
    : InteractionEvent(), state_(state), canvasSize_(canvasSize) {}

TouchEvent::TouchEvent(std::vector<TouchPoint> touchPoints, TouchEvent::TouchState state, uvec2 canvasSize)
    : InteractionEvent(), touchPoints_(touchPoints), state_(state), canvasSize_(canvasSize) {}

TouchEvent* TouchEvent::clone() const {
    return new TouchEvent(*this);
}

TouchEvent::~TouchEvent() {}

vec2 TouchEvent::getCenterPoint() const {
    if (touchPoints_.empty()) {
        return vec2(0);
    } else {
        // Compute average position
        vec2 sum(0);
        std::for_each(touchPoints_.begin(), touchPoints_.end(), [&](const TouchPoint& p){
            sum += p.getPos();
        });
        return sum / static_cast<float>(touchPoints_.size());
    }
}

inviwo::vec2 TouchEvent::getCenterPointNormalized() const {
    if (touchPoints_.empty()) {
        return vec2(0);
    } else {
        // Compute average position
        vec2 sum(0);
        std::for_each(touchPoints_.begin(), touchPoints_.end(), [&](const TouchPoint& p){
            sum += p.getPosNormalized();
        });
        return sum / static_cast<float>(touchPoints_.size());
    }
}

vec2 TouchEvent::getPrevCenterPointNormalized() const {
    if (touchPoints_.empty()) {
        return vec2(0);
    } else {
        // Compute average position
        vec2 sum(0);
        std::for_each(touchPoints_.begin(), touchPoints_.end(), [&](const TouchPoint& p){
            sum += p.getPrevPosNormalized();
        });
        return sum / static_cast<float>(touchPoints_.size());
    }
}

void TouchEvent::serialize(IvwSerializer& s) const {
    s.serialize("touchState", state_);
    s.serialize("touchPoints", touchPoints_, "touchPoint");
    InteractionEvent::serialize(s); 
}

void TouchEvent::deserialize(IvwDeserializer& d) { 
    d.deserialize("touchState", state_);
    d.deserialize("touchPoints", touchPoints_, "touchPoint");
    InteractionEvent::deserialize(d); 
}

bool TouchEvent::matching(const Event* aEvent) const {
    const TouchEvent* event = dynamic_cast<const TouchEvent*>(aEvent);
    if (event) {
        return matching(event);
    } else {
        return false;
    }
}

bool TouchEvent::matching(const TouchEvent* aEvent) const {
    return (state_ & aEvent->state_) == aEvent->state_  // aEvent.state equal to any of this.state
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