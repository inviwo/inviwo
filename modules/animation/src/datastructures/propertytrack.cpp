/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2020 Inviwo Foundation
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

#include <modules/animation/datastructures/propertytrack.h>

namespace inviwo {
namespace animation {
namespace detail {
/**
 * Helper function for inviwo::animation::PropertyTrack::setPropertyFromKeyframe
 * @see inviwo::animation::BasePropertyTrack::setPropertyFromKeyframe
 */
void setPropertyFromKeyframeHelper(CameraProperty* property, const CameraKeyframe* keyframe) {
    property->setLook(keyframe->getLookFrom(), keyframe->getLookTo(), keyframe->getLookUp());
}
/**
 * Helper function for inviwo::animation::PropertyTrack::setKeyframeFromProperty
 * @see inviwo::animation::BasePropertyTrack::setKeyframeFromProperty
 */
void setKeyframeFromPropertyHelper(const CameraProperty* property, CameraKeyframe* keyframe) {
    keyframe->updateFrom(property->get());
}

}  // namespace detail

template <>
std::string PropertyTrack<CameraProperty, CameraKeyframe>::classIdentifier() {
    // Use property class identifier since multiple properties
    // may have the same key (data type)
    std::string id = "org.inviwo.animation.PropertyTrack.for." + CameraProperty::classIdentifier;
    return id;
}

template <>
AnimationTimeState PropertyTrack<CameraProperty, CameraKeyframe>::animateSequence(
    const KeyframeSequenceTyped<CameraKeyframe>& seq, Seconds from, Seconds to,
    AnimationState state) const {
    seq(from, to, property_->get());
    return {to, state};
}

}  // namespace animation
}  // namespace inviwo
