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

#include <modules/animation/datastructures/controlkeyframesequence.h>

namespace inviwo {

namespace animation {

ControlKeyframeSequence::ControlKeyframeSequence() = default;

ControlKeyframeSequence::ControlKeyframeSequence(
    std::vector<std::unique_ptr<ControlKeyframe>> keyframes)
    : BaseKeyframeSequence<ControlKeyframe>(std::move(keyframes)) {}

ControlKeyframeSequence::ControlKeyframeSequence(const ControlKeyframeSequence& rhs) = default;

ControlKeyframeSequence& ControlKeyframeSequence::operator=(const ControlKeyframeSequence& that) =
    default;

ControlKeyframeSequence::~ControlKeyframeSequence() = default;

ControlKeyframeSequence* ControlKeyframeSequence::clone() const {
    return new ControlKeyframeSequence(*this);
}

AnimationTimeState ControlKeyframeSequence::operator()(Seconds from, Seconds to,
                                                       AnimationState state) const {
    // 'it' will be the first key. with a time larger then 'to'.
    auto fromIt = std::upper_bound(keyframes_.begin(), keyframes_.end(), from,
                                   [](const auto& a, const auto& b) { return a < *b; });
    auto toIt = std::upper_bound(keyframes_.begin(), keyframes_.end(), to,
                                 [](const auto& a, const auto& b) { return a < *b; });
    if (toIt != fromIt) {
        if (from < to) {
            return (**fromIt)(from, to, state);
        } else {
            return (**toIt)(from, to, state);
        }
    }

    return {to, state};
}

}  // namespace animation

}  // namespace inviwo
