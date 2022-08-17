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
#pragma once

#include <modules/animation/animationmoduledefine.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/animationstate.h>
#include <modules/animation/datastructures/keyframesequence.h>

#include <algorithm>
#include <iterator>

namespace inviwo {

namespace animation {

namespace detail {

template <typename T>
Seconds getTimeHelper(const T& elem, PlaybackDirection direction) {
    if constexpr (std::is_base_of_v<KeyframeSequence, T>) {
        if (direction == PlaybackDirection::Forward) {
            return elem.getLast().getTime();
        } else {
            return elem.getFirst().getTime();
        }

    } else {
        return elem.getTime();
    };
}

}  // namespace detail
/*
 * Calls operator(Seconds from, Seconds to,
                      AnimationState state) -> AnimationTimeState for each element [begin, end) in
 the time range [from, to].
 * The elements are processed in the order given by from and to, e.g. reverse order if from > to.
 * The function is currently designed for Dirac delta Keyframe and KeyframeSequence, i.e. no
 interpolation (such as ControlKeyframe, ControlKeyframeSequence).
 * @param begin, end iterator range to types std::unique_ptr<Keyframe> or
 std::unique_ptr<KeyframeSequence>.
 * @return AnimationTimeState after processing the elements.
 */
template <typename Iterator>
AnimationTimeState animateRange(Iterator begin, Iterator end, Seconds from, Seconds to,
                                AnimationState state) {
    auto animate = [](auto begin, auto end, Seconds from, Seconds to,
                      AnimationState state) -> AnimationTimeState {
        auto direction = from <= to ? PlaybackDirection::Forward : PlaybackDirection::Backward;
        AnimationTimeState res{to, state};
        while (begin != end && res.state != AnimationState::Paused) {
            res = (*begin)(from, to, state);
            if ((direction == PlaybackDirection::Forward && res.time <= (*begin)) ||
                (direction == PlaybackDirection::Backward && res.time >= (*begin))) {
                // We jumped in the opposite direction
                break;
            }
            // Use jump-to-time if set, previous Keyframe/KeyframeSequence time otherwise
            from = res.time != to ? res.time : detail::getTimeHelper(*begin, direction);

            ++begin;
        }
        return res;
    };
    auto first = std::min(from, to);
    auto last = std::max(from, to);

    // 'fromIt' will be the first item with a time larger than or equal to 'first'
    auto fromIt = std::lower_bound(begin, end, first,
                                   [](const auto& it, const auto& val) { return it < val; });
    // 'toIt' will be the first key with a time larger than 'last'
    auto toIt = std::upper_bound(fromIt, end, last,
                                 [](const auto& val, const auto& it) { return val < it; });
    if (from <= to) {
        return animate(fromIt, toIt, from, to, state);
    } else {
        return animate(std::make_reverse_iterator(toIt), std::make_reverse_iterator(fromIt), from,
                       to, state);
    }
}

}  // namespace animation

}  // namespace inviwo
