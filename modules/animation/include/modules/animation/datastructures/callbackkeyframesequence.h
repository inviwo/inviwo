/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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

#include <modules/animation/datastructures/animationstate.h>        // for AnimationState, Anima...
#include <modules/animation/datastructures/animationtime.h>         // for Seconds
#include <modules/animation/datastructures/basekeyframesequence.h>  // for BaseKeyframeSequence
#include <modules/animation/datastructures/callbackkeyframe.h>      // for CallbackKeyframe
#include <modules/animation/datastructures/keyframe.h>              // for operator<

#include <functional>  // for __base
#include <memory>      // for unique_ptr
#include <string>      // for basic_string
#include <vector>      // for vector

namespace inviwo {

namespace animation {

/** \class CallbackKeyframeSequence
 * KeyframeSequence for CallbackKeyframe.
 * Animating over a CallbackKeyframe in the sequence will call its corresponding callbacks.
 * @see CallbackKeyframe
 * @see CallbackTrack
 */
class IVW_MODULE_ANIMATION_API CallbackKeyframeSequence
    : public BaseKeyframeSequence<CallbackKeyframe> {
public:
    using key_type = typename BaseKeyframeSequence<CallbackKeyframe>::key_type;
    CallbackKeyframeSequence() = default;
    CallbackKeyframeSequence(const CallbackKeyframeSequence& rhs) = default;
    CallbackKeyframeSequence(CallbackKeyframeSequence&& rhs) = default;
    CallbackKeyframeSequence(std::vector<std::unique_ptr<CallbackKeyframe>> keyframes);
    CallbackKeyframeSequence& operator=(const CallbackKeyframeSequence& that) = default;
    CallbackKeyframeSequence& operator=(CallbackKeyframeSequence&& that) = default;

    virtual ~CallbackKeyframeSequence() = default;

    virtual CallbackKeyframeSequence* clone() const override;

    virtual AnimationTimeState operator()(Seconds from, Seconds to, AnimationState state) const;
};

}  // namespace animation

}  // namespace inviwo
