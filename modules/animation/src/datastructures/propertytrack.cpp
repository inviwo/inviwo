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
 * Helper function for inviwo::animation::PropertyTrack::setOtherProperty
 * @see inviwo::animation::BasePropertyTrack::setOtherProperty
 */
void setOtherPropertyHelper(CameraProperty* property, CameraKeyframe* keyframe) {
    property->setLook(keyframe->getLookFrom(), keyframe->getLookTo(), keyframe->getLookUp());
}
/**
 * Helper function for inviwo::animation::PropertyTrack::updateKeyframeFromProperty
 * @see inviwo::animation::BasePropertyTrack::updateKeyframeFromProperty
 */
void updateKeyframeFromPropertyHelper(CameraProperty* property, CameraKeyframe* keyframe) {
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
/**
 * Track of sequences
 * ----------X======X====X-----------X=========X-------X=====X--------
 * |- case 1-|-case 2----------------|-case 2----------|-case 2------|
 *           |-case 2a---|-case 2b---|
 */
template <>
AnimationTimeState PropertyTrack<CameraProperty, CameraKeyframe>::operator()(
    Seconds from, Seconds to, AnimationState state) const {
    using Prop = CameraProperty;
    if (!this->isEnabled() || this->empty()) return {to, state};

    // 'it' will be the first seq. with a first time larger then 'to'.
    auto it = std::upper_bound(this->begin(), this->end(), to,
                               [](const auto& a, const auto& b) { return a < b; });

    if (it == this->begin()) {
        if (from > it->getFirstTime()) {  // case 1
            const auto& key = it->getFirst();
            // Do not update aspect ratio, e.g. updateFrom(Camera&)
            property_->setLook(key.getLookFrom(), key.getLookTo(), key.getLookUp());
        }
    } else {  // case 2
        auto& seq1 = *std::prev(it);

        if (to < seq1.getLastTime()) {  // case 2a
            seq1(from, to, property_->get());
        } else {  // case 2b
            if (from < seq1.getLastTime()) {
                // We came from before the previous key
                const auto& key = seq1.getLast();
                // Do not update aspect ratio, e.g. updateFrom(Camera&)
                property_->setLook(key.getLookFrom(), key.getLookTo(), key.getLookUp());
            } else if (it != this->end() && from > it->getFirstTime()) {
                // We came form after the next key
                const auto& key = it->getFirst();
                // Do not update aspect ratio, e.g. updateFrom(Camera&)
                property_->setLook(key.getLookFrom(), key.getLookTo(), key.getLookUp());
            }
            // we moved in an unmarked region, do nothing.
        }
    }
    return {to, state};
}

}  // namespace animation
}  // namespace inviwo
