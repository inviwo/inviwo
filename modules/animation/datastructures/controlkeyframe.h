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

#ifndef IVW_CONTROLKEYFRAME_H
#define IVW_CONTROLKEYFRAME_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/animationstate.h>

namespace inviwo {

namespace animation {

enum class ControlAction { Pause, JumpTo, Script };

// TODO: Perhaps replace with variant in future
struct ControlPayload {
    Seconds jumpToTime;
    std::string script;
};

/** \class ControlKeyframe
 * Base class for Keyframes that performs some type of control action.
 * @see Keyframe
 */
class IVW_MODULE_ANIMATION_API ControlKeyframe : public Keyframe {
public:
    using value_type = void;
    ControlKeyframe() = default;

	ControlKeyframe(Seconds time, ControlAction action = ControlAction::Pause, ControlPayload payload = {}) : time_(time), action_(action), payload_(payload) {}
    virtual ~ControlKeyframe() = default;

    ControlKeyframe(const ControlKeyframe& rhs) = default;
    ControlKeyframe& operator=(const ControlKeyframe& that) {
        if (this != &that) {
            setTime(that.time_);
        }
        return *this;
    }

    virtual void setTime(Seconds time) override {
        if (time != time_) {
            auto oldTime = time_;
            time_ = time;
            notifyKeyframeTimeChanged(this, oldTime);
        }
    }
    virtual Seconds getTime() const override { return time_; }

    const void getValue() const {}
    void getValue() {}

    ControlAction getAction() const { return action_; }
    ControlPayload getPayload() const { return payload_; }

    AnimationTimeState operator()(Seconds from, Seconds to, AnimationState state);

	std::string classIdentifier() const {
		return "org.inviwo.animation.ControlKeyframe";
	}

	std::string getClassIdentifier() const {
		return classIdentifier();
	}

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    Seconds time_{0.0};
    ControlAction action_;
    ControlPayload payload_;
};

IVW_MODULE_ANIMATION_API bool operator==(const ControlKeyframe& a, const ControlKeyframe& b);
IVW_MODULE_ANIMATION_API bool operator!=(const ControlKeyframe& a, const ControlKeyframe& b);

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_CONTROLKEYFRAME_H
