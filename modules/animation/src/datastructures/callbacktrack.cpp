/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <modules/animation/datastructures/callbacktrack.h>

#include <inviwo/core/io/serialization/deserializer.h>                  // for ContainerWrapper<...
#include <inviwo/core/util/exception.h>                                 // for Exception
#include <modules/animation/algorithm/animationrange.h>                 // for animateRange
#include <modules/animation/datastructures/animationstate.h>            // for AnimationState
#include <modules/animation/datastructures/animationtime.h>             // for Seconds
#include <modules/animation/datastructures/basetrack.h>                 // for BaseTrack<>::key_...
#include <modules/animation/datastructures/callbackkeyframe.h>          // for CallbackKeyframe
#include <modules/animation/datastructures/callbackkeyframesequence.h>  // for CallbackKeyframeS...

namespace inviwo {

namespace animation {

CallbackTrack::CallbackTrack() : BaseTrack<CallbackKeyframeSequence>{"Callback Track", 0} {}

CallbackTrack* CallbackTrack::clone() const { return new CallbackTrack(*this); }

std::string CallbackTrack::classIdentifier() { return "org.inviwo.animation.CallbackTrack"; }
std::string CallbackTrack::getClassIdentifier() const { return classIdentifier(); }

AnimationTimeState CallbackTrack::operator()(Seconds from, Seconds to, AnimationState state) const {
    if (!isEnabled() || empty()) return {to, state};
    return animateRange(begin(), end(), from, to, state);
}

}  // namespace animation

}  // namespace inviwo
