/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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
#pragma once

#include <modules/animation/animationmoduledefine.h>  // for IVW_MODULE_ANIMATION_API

#include <modules/animation/datastructures/animationstate.h>  // for AnimationState, AnimationTi...
#include <modules/animation/datastructures/animationtime.h>   // for Seconds
#include <modules/animation/datastructures/basekeyframe.h>    // for BaseKeyframe

#include <functional>  // for function

namespace inviwo {

namespace animation {

/** \class CallbackKeyframe
 * Keyframe which calls one function when animating forward and a different one when animating
 * backwards. This makes it possible to do/undo things when animating back and forth.
 * This keyframe is intended to be added programatically, i.e., not through the animation user
 * interface.
 * @note With great power comes great responsibility. Errors can occur if for example the do/undo
 * states are inconsistent.
 * @see CallbackKeyframeSequence
 * @see CallbackTrack
 */
class IVW_MODULE_ANIMATION_API CallbackKeyframe : public BaseKeyframe {
public:
    CallbackKeyframe() = default;
    CallbackKeyframe(Seconds time, std::function<void()> do_ = nullptr,
                     std::function<void()> undo = nullptr);
    CallbackKeyframe(const CallbackKeyframe& rhs) = default;
    CallbackKeyframe& operator=(const CallbackKeyframe& that) = default;
    virtual ~CallbackKeyframe() = default;

    virtual CallbackKeyframe* clone() const override;

    AnimationTimeState operator()(Seconds from, Seconds to, AnimationState state) const;

private:
    std::function<void()> do_;
    std::function<void()> undo_;
};

}  // namespace animation

}  // namespace inviwo
