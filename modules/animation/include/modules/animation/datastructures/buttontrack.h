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
#pragma once

#include <inviwo/core/properties/buttonproperty.h>                    // for ButtonProperty
#include <modules/animation/datastructures/animationstate.h>          // for AnimationState, Ani...
#include <modules/animation/datastructures/animationtime.h>           // for Seconds
#include <modules/animation/datastructures/buttonkeyframesequence.h>  // for ButtonKeyframeSequence
#include <modules/animation/datastructures/propertytrack.h>           // for PropertyTrack

namespace inviwo {

namespace animation {
class ButtonKeyframe;

using ButtonTrack = PropertyTrack<ButtonProperty, ButtonKeyframe, ButtonKeyframeSequence>;

namespace detail {

/**
 * Presses the button.
 * Helper function for inviwo::animation::PropertyTrack::setPropertyFromKeyframe
 * @see inviwo::animation::BasePropertyTrack::setPropertyFromKeyframe
 */
template <>
inline void setPropertyFromKeyframeHelper(ButtonProperty* property, const ButtonKeyframe*) {
    property->pressButton();
}
/**
 * Does nothing. The property of a ButtonKeyframe cannot be changed since all are supposed to be the
 * same for a ButtonTrack. Helper function for
 * inviwo::animation::PropertyTrack::setKeyframeFromProperty
 * @see inviwo::animation::BasePropertyTrack::setKeyframeFromProperty
 */
template <>
inline void setKeyframeFromPropertyHelper(const ButtonProperty*, ButtonKeyframe*) {}

template <>
struct AnimateSequence<ButtonProperty, ButtonKeyframeSequence> {
    static AnimationTimeState animate(ButtonProperty* prop, const ButtonKeyframeSequence& seq,
                                      Seconds from, Seconds to, AnimationState state) {
        bool pressed = false;
        seq(from, to, pressed);
        if (pressed) {
            prop->pressButton();
        }
        return {to, state};
    }
};

}  // namespace detail

}  // namespace animation

}  // namespace inviwo
