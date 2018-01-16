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

bool operator==(const ControlKeyframe& a, const ControlKeyframe& b) {
    if (a.getTime() == b.getTime() && a.getAction() == b.getAction()) {
        switch (a.getAction()) {
            case ControlAction::JumpTo:
                return a.getPayload().jumpToTime == b.getPayload().jumpToTime;
            case ControlAction::Script:
                return a.getPayload().script == b.getPayload().script;
            default:
                break;
        }
    }
    return false;
}

bool operator!=(const ControlKeyframe& a, const ControlKeyframe& b) { return !(a == b); }

AnimationTimeState ControlKeyframe::operator()(Seconds from, Seconds to, AnimationState state) {
	if (state == AnimationState::Playing) {
		if (from < getTime() && getTime() < to || to < getTime() && getTime() < from) {
			// We passed over this keyframe
			switch (getAction()) {
			case ControlAction::Pause:
				return { to, AnimationState::Paused };
			case ControlAction::JumpTo:
				return { getPayload().jumpToTime, state };
			case ControlAction::Script:
				// TODO: IMPLEMENT RUN SCRIPT
				return { to, state };
			}
		}
	}
	return { to, state };
}

void ControlKeyframe::deserialize(Deserializer& d) {
    double tmp = time_.count();
    d.deserialize("time", tmp);
    time_ = Seconds{tmp};
    d.deserialize("action", action_);
    switch (action_) {
        case ControlAction::Pause:
            break;
        case ControlAction::JumpTo:
            d.deserialize("payload", tmp);
            payload_.jumpToTime = Seconds{tmp};
            break;
        case ControlAction::Script:
            d.deserialize("payload", payload_.script);
            break;
        default:
            break;
    }
}

void ControlKeyframe::serialize(Serializer& s) const {
    s.serialize("time", time_.count());
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
