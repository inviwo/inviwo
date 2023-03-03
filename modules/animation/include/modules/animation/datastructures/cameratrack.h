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

#include <inviwo/core/properties/cameraproperty.h>                         // for CameraProperty
#include <modules/animation/datastructures/animationstate.h>               // for AnimationState
#include <modules/animation/datastructures/animationtime.h>                // for Seconds
#include <modules/animation/datastructures/camerakeyframe.h>               // for CameraKeyframe
#include <modules/animation/datastructures/propertytrack.h>                // for PropertyTrack
#include <modules/animation/datastructures/valuekeyframesequence.h>        // for KeyframeSequen...
#include <modules/animation/interpolation/camerasphericalinterpolation.h>  // for CameraSpherica...
#include <modules/animation/interpolation/interpolation.h>                 // for InterpolationT...

#include <memory>  // for make_unique
#include <vector>  // for vector

namespace inviwo {

namespace animation {

using CameraKeyframeSequence = KeyframeSequenceTyped<CameraKeyframe>;
using CameraTrack = PropertyTrack<CameraProperty, CameraKeyframe>;

namespace detail {

/**
 * Helper function for inviwo::animation::PropertyTrack::setPropertyFromKeyframe
 * @see inviwo::animation::BasePropertyTrack::setPropertyFromKeyframe
 */
template <>
inline void setPropertyFromKeyframeHelper(CameraProperty* property,
                                          const CameraKeyframe* keyframe) {
    property->setLook(keyframe->getLookFrom(), keyframe->getLookTo(), keyframe->getLookUp());
}
/**
 * Helper function for inviwo::animation::PropertyTrack::setKeyframeFromProperty
 * @see inviwo::animation::BasePropertyTrack::setKeyframeFromProperty
 */
template <>
inline void setKeyframeFromPropertyHelper(const CameraProperty* property,
                                          CameraKeyframe* keyframe) {
    keyframe->updateFrom(property->get());
}

template <>
struct AnimateSequence<CameraProperty, CameraKeyframeSequence> {
    static AnimationTimeState animate(CameraProperty* prop, const CameraKeyframeSequence& seq,
                                      Seconds from, Seconds to, AnimationState state) {
        seq(from, to, prop->get());
        return {to, state};
    }
};

template <>
struct DefaultInterpolationCreator<CameraKeyframe> {
    static std::unique_ptr<CameraSphericalInterpolation> create() {
        return std::make_unique<CameraSphericalInterpolation>();
    }
};

}  // namespace detail

}  // namespace animation

}  // namespace inviwo
