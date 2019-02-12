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

#ifndef IVW_CONTROLKEYFRAME_H
#define IVW_CONTROLKEYFRAME_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/animation/datastructures/basekeyframe.h>
#include <modules/animation/datastructures/animationstate.h>

namespace inviwo {

namespace animation {

enum class ControlAction { Pause, Jump };

/** \class ControlKeyframe
 * Base class for Keyframes that performs some type of control action.
 * @see Keyframe
 */
class IVW_MODULE_ANIMATION_API ControlKeyframe : public BaseKeyframe {
public:
    using value_type = void;
    ControlKeyframe() = default;
    ControlKeyframe(Seconds time, ControlAction action = ControlAction::Pause,
                    Seconds jumpTime = Seconds{0});
    ControlKeyframe(const ControlKeyframe& rhs);
    ControlKeyframe& operator=(const ControlKeyframe& that);
    virtual ~ControlKeyframe();
    virtual ControlKeyframe* clone() const override;

    ControlAction getAction() const;
    void setAction(ControlAction action);

    Seconds getJumpTime() const;
    void setJumpTime(Seconds jumpTime);

    AnimationTimeState operator()(Seconds from, Seconds to, AnimationState state);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    ControlAction action_;
    Seconds jumpTime_;
};

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_CONTROLKEYFRAME_H
