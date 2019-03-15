/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/animation/datastructures/controlkeyframe.h>

namespace inviwo {
namespace animation {

ControlKeyframe::ControlKeyframe(Seconds time, ControlAction action, Seconds jumpTime)
    : BaseKeyframe(time), action_(action), jumpTime_(jumpTime) {}
ControlKeyframe::~ControlKeyframe() = default;

ControlKeyframe::ControlKeyframe(const ControlKeyframe& rhs) = default;
ControlKeyframe& ControlKeyframe::operator=(const ControlKeyframe& that) {
    if (this != &that) {
        BaseKeyframe::operator=(that);
        action_ = that.action_;
        jumpTime_ = that.jumpTime_;
    }
    return *this;
}

ControlKeyframe* ControlKeyframe::clone() const { return new ControlKeyframe(*this); }

ControlAction ControlKeyframe::getAction() const { return action_; }
void ControlKeyframe::setAction(ControlAction action) { action_ = action; }

Seconds ControlKeyframe::getJumpTime() const { return jumpTime_; }
void ControlKeyframe::setJumpTime(Seconds jumpTime) { jumpTime_ = jumpTime; }

AnimationTimeState ControlKeyframe::operator()(Seconds from, Seconds to, AnimationState state) {

    if (state == AnimationState::Playing) {
        if ((from < getTime() && to >= getTime()) || (to <= getTime() && from > getTime())) {
            // We passed over this keyframe
            switch (action_) {
                case ControlAction::Pause:
                    return {time_, AnimationState::Paused};
                case ControlAction::Jump:
                    return {jumpTime_, state};
            }
        }
    }
    return {to, state};
}

void ControlKeyframe::deserialize(Deserializer& d) {
    BaseKeyframe::deserialize(d);
    d.deserialize("action", action_);
    switch (action_) {
        case ControlAction::Pause:
            break;
        case ControlAction::Jump: {
            double tmp = jumpTime_.count();
            d.deserialize("payload", tmp);
            jumpTime_ = Seconds{tmp};
            break;
        }
        default:
            break;
    }
}

void ControlKeyframe::serialize(Serializer& s) const {
    BaseKeyframe::serialize(s);
    s.serialize("action", action_);
    switch (action_) {
        case ControlAction::Pause:
            break;
        case ControlAction::Jump:
            s.serialize("payload", jumpTime_.count());
            break;

        default:
            break;
    }
}

}  // namespace animation
}  // namespace inviwo
