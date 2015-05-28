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

#include <inviwo/core/interaction/events/mouseevent.h>

namespace inviwo {

MouseEvent::MouseEvent(ivec2 position, int delta, int button, int state, int orientation,
                       int modifiers, uvec2 canvasSize, double depth)
    : InteractionEvent(modifiers)
    , button_(button)
    , state_(state)
    , wheelOrientation_(orientation)

    , position_(position)
    , wheelSteps_(delta)
    , canvasSize_(canvasSize)
    , depth_(depth) {}

MouseEvent::MouseEvent(ivec2 position, int button, int state /*= MOUSE_STATE_NONE*/,
                       int modifiers /*= InteractionEvent::MODIFIER_NONE*/,
                       uvec2 canvasSize /*= uvec2(0)*/, double depth)
    : InteractionEvent(modifiers)
    , button_(button)
    , state_(state)
    , wheelOrientation_(MOUSE_WHEEL_NONE)

    , position_(position)
    , wheelSteps_(0)
    , canvasSize_(canvasSize)
    , depth_(depth) {}

MouseEvent::MouseEvent(int button, int modifiers /*= InteractionEvent::MODIFIER_NONE*/,
                       int state /*= MOUSE_STATE_NONE*/, int orientation /*= MOUSE_WHEEL_NONE*/)
    : InteractionEvent(modifiers)
    , button_(button)
    , state_(state)
    , wheelOrientation_(orientation)

    , position_(0)
    , wheelSteps_(0)
    , canvasSize_(0)
    , depth_(1.0) {}

MouseEvent::MouseEvent(const MouseEvent& rhs)
    : InteractionEvent(rhs)
    , button_(rhs.button_)
    , state_(rhs.state_)
    , wheelOrientation_(rhs.wheelOrientation_)
    , position_(rhs.position_)
    , wheelSteps_(rhs.wheelSteps_)
    , canvasSize_(rhs.canvasSize_) {}

MouseEvent& MouseEvent::operator=(const MouseEvent& that) {
    if (this != &that) {
        InteractionEvent::operator=(that);
        button_ = that.button_;
        state_ = that.state_;
        wheelOrientation_ = that.wheelOrientation_;
        position_ = that.position_;
        wheelSteps_ = that.wheelSteps_;
        canvasSize_ = that.canvasSize_;
    }
    return *this;
}

MouseEvent* MouseEvent::clone() const {
    return new MouseEvent(*this);
}

MouseEvent::~MouseEvent() {}

void MouseEvent::modify(ivec2 newPosition, uvec2 newCanvasSize) {
    position_ = newPosition;
    canvasSize_ = newCanvasSize;
};

void MouseEvent::serialize(IvwSerializer& s) const {
    InteractionEvent::serialize(s);
    s.serialize("button", button_);
    s.serialize("state", state_);
    s.serialize("wheelOrientation", wheelOrientation_);
}

void MouseEvent::deserialize(IvwDeserializer& d) {
    InteractionEvent::deserialize(d);
    d.deserialize("button", button_);
    d.deserialize("state", state_);
    d.deserialize("wheelOrientation", wheelOrientation_);
}

const std::string MouseEvent::buttonNames_[] = {"", "Left button", "Middle button", "Right button"};

std::string MouseEvent::getClassIdentifier() const { return "org.inviwo.MouseEvent"; }

bool MouseEvent::matching(const Event* aEvent) const {
    const MouseEvent* event = dynamic_cast<const MouseEvent*>(aEvent);
    if (event) {
        return matching(event);
    } else {
        return false;
    }
}
bool MouseEvent::matching(const MouseEvent* aEvent) const {
    return (button_ & aEvent->button_) == aEvent->button_ 
        && (state_ & aEvent->state_) == aEvent->state_  // aEvent.state equal to any of this.state.
        && (wheelOrientation_ & aEvent->wheelOrientation_) == aEvent->wheelOrientation_ 
        && modifiers_ == aEvent->modifiers_;
}

bool MouseEvent::equalSelectors(const Event* aEvent) const {
    const MouseEvent* event = dynamic_cast<const MouseEvent*>(aEvent);
    if (event) {
        return InteractionEvent::equalSelectors(event)
            && button_ == event->button_
            && state_ == event->state_
            && wheelOrientation_ == event->wheelOrientation_;
    } else {
        return false;
    }
}

std::string MouseEvent::buttonName() const {
    std::vector<std::string> names;
    if ((button_ & MOUSE_BUTTON_LEFT) == MOUSE_BUTTON_LEFT) names.push_back(buttonNames_[1]);
    if ((button_ & MOUSE_BUTTON_MIDDLE) == MOUSE_BUTTON_MIDDLE) names.push_back(buttonNames_[2]);
    if ((button_ & MOUSE_BUTTON_RIGHT) == MOUSE_BUTTON_RIGHT) names.push_back(buttonNames_[3]);

    if (!names.empty()) {
        return joinString(names, "+");
    } else {
        return buttonNames_[0];
    }
}



}  // namespace