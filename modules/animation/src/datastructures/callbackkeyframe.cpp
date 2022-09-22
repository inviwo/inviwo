/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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

#include <modules/animation/datastructures/callbackkeyframe.h>

#include <modules/animation/datastructures/animationstate.h>  // for AnimationState, AnimationTi...
#include <modules/animation/datastructures/animationtime.h>   // for Seconds
#include <modules/animation/datastructures/basekeyframe.h>    // for BaseKeyframe

#include <chrono>   // for operator<=, operator>=, dur...
#include <utility>  // for move

namespace inviwo {

namespace animation {

CallbackKeyframe::CallbackKeyframe(Seconds time, std::function<void()> doForward,
                                   std::function<void()> undo)
    : BaseKeyframe(time), do_(std::move(doForward)), undo_(std::move(undo)) {}

CallbackKeyframe* CallbackKeyframe::clone() const { return new CallbackKeyframe(*this); }

AnimationTimeState CallbackKeyframe::operator()(Seconds from, Seconds to,
                                                AnimationState state) const {
    if (do_ && (from <= getTime() && to >= getTime())) {
        // Animating forward, passing from left to right
        do_();
    } else if (undo_ && (to <= getTime() && from >= getTime())) {
        // Animating backward, passing from right to left
        undo_();
    }
    return {to, state};
}

}  // namespace animation

}  // namespace inviwo
