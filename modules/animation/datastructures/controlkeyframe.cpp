/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

ControlKeyframe::ControlKeyframe(Seconds time, ControlAction action, ControlPayload payload)
    : BaseKeyframe(time), action_(action), payload_(payload) {}
ControlKeyframe::~ControlKeyframe() = default;

ControlKeyframe::ControlKeyframe(const ControlKeyframe& rhs) = default;
ControlKeyframe& ControlKeyframe::operator=(const ControlKeyframe& that) {
    if (this != &that) {
        BaseKeyframe::operator=(that);
        action_ = that.action_;
        payload_ = that.payload_;
    }
    return *this;
}

ControlKeyframe* ControlKeyframe::clone() const { return new ControlKeyframe(*this); }

std::string ControlKeyframe::classIdentifier() { return "org.inviwo.animation.ControlKeyframe"; }
std::string ControlKeyframe::getClassIdentifier() const { return classIdentifier(); }

bool ControlKeyframe::equal(const Keyframe& other) const {
    if (!BaseKeyframe::equal(other)) return false;
    const auto& o = static_cast<const ControlKeyframe&>(other);
    if (getTime() == o.getTime() && getAction() == o.getAction()) {
        switch (getAction()) {
            case ControlAction::JumpTo:
                return getPayload().jumpToTime == o.getPayload().jumpToTime;
            case ControlAction::Script:
                return getPayload().script == o.getPayload().script;
            default:
                break;
        }
    }
    return false;
}

void ControlKeyframe::setAction(ControlAction action) { action_ = action; }

void ControlKeyframe::setPayload(ControlPayload payload) { payload_ = payload; }

ControlAction ControlKeyframe::getAction() const { return action_; }
ControlPayload ControlKeyframe::getPayload() const { return payload_; }

AnimationTimeState ControlKeyframe::operator()(Seconds from, Seconds to, AnimationState state) {

    if (state == AnimationState::Playing) {
        if (from < getTime() && to >= getTime() || to <= getTime() && from > getTime()) {
            // We passed over this keyframe
            switch (action_) {
                case ControlAction::Pause:
                    return {time_, AnimationState::Paused};
                case ControlAction::JumpTo:
                    return {getPayload().jumpToTime, state};
                case ControlAction::Script:
                    LogWarn("Scripts not implemented");
                    // TODO: IMPLEMENT RUN SCRIPT
                    return {to, state};
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
        case ControlAction::JumpTo: {
            double tmp = payload_.jumpToTime.count();
            d.deserialize("payload", tmp);
            payload_.jumpToTime = Seconds{tmp};
            break;
        }
        case ControlAction::Script:
            d.deserialize("payload", payload_.script);
            break;
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
        case ControlAction::JumpTo:
            s.serialize("payload", payload_.jumpToTime.count());
            break;
        case ControlAction::Script:
            s.serialize("payload", payload_.script);
            break;
        default:
            break;
    }
}

}  // namespace animation
}  // namespace inviwo
