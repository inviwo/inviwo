/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/animation/datastructures/callbackkeyframesequence.h>
#include <modules/animation/algorithm/animationrange.h>

namespace inviwo {

namespace animation {

CallbackKeyframeSequence::CallbackKeyframeSequence(
    std::vector<std::unique_ptr<CallbackKeyframe>> keyframes)
    : BaseKeyframeSequence<CallbackKeyframe>(std::move(keyframes)) {}

CallbackKeyframeSequence* CallbackKeyframeSequence::clone() const {
    return new CallbackKeyframeSequence(*this);
}

AnimationTimeState CallbackKeyframeSequence::operator()(Seconds from, Seconds to,
                                                        AnimationState state) const {
    auto animate = [](auto begin, auto end, Seconds from, Seconds to,
                      AnimationState state) -> AnimationTimeState {
        AnimationTimeState res{to, state};
        while (begin != end) {
            res = (**begin)(from, to, state);
            ++begin;
        }
        return res;
    };

    auto [fromIt, toIt] = getRange(keyframes_.begin(), keyframes_.end(), from, to);
    if (from <= to) {
        return animate(fromIt, toIt, from, to, state);
    } else {
        return animate(std::make_reverse_iterator(toIt), std::make_reverse_iterator(fromIt), from,
                       to, state);
    }
}

}  // namespace animation

}  // namespace inviwo
