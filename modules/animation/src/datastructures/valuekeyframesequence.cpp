/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2020 Inviwo Foundation
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

#include <modules/animation/datastructures/valuekeyframesequence.h>
#include <modules/animation/interpolation/camerasphericalinterpolation.h>

namespace inviwo {

namespace animation {

void ValueKeyframeSequenceObserverble::notifyValueKeyframeSequenceEasingChanged(
    ValueKeyframeSequence* seq) {
    forEachObserver(
        [&](ValueKeyframeSequenceObserver* o) { o->onValueKeyframeSequenceEasingChanged(seq); });
}

void ValueKeyframeSequenceObserverble::notifyValueKeyframeSequenceInterpolationChanged(
    ValueKeyframeSequence* seq) {
    forEachObserver([&](ValueKeyframeSequenceObserver* o) {
        o->onValueKeyframeSequenceInterpolationChanged(seq);
    });
}

template <>
KeyframeSequenceTyped<CameraKeyframe>::KeyframeSequenceTyped()
    : BaseKeyframeSequence<CameraKeyframe>{}
    , ValueKeyframeSequence{}
    , interpolation_{std::make_unique<CameraSphericalInterpolation>()} {}

template <>
KeyframeSequenceTyped<CameraKeyframe>::KeyframeSequenceTyped(std::vector<std::unique_ptr<CameraKeyframe>> keyframes)
    : BaseKeyframeSequence<CameraKeyframe>{std::move(keyframes)}
    , ValueKeyframeSequence()
    , interpolation_{std::make_unique<CameraSphericalInterpolation>()} {}

template <>
void KeyframeSequenceTyped<CameraKeyframe>::operator()(Seconds from, Seconds to, Camera& out) const {
    if (interpolation_) {
        (*interpolation_)(this->keyframes_, from, to, easing_, out);
    } else {
        // Note: Network will be locked at this point
        const auto& key = *this->keyframes_.front();
        out.setLookFrom(key.getLookFrom());
        out.setLookTo(key.getLookTo());
        out.setLookUp(key.getLookUp());
    }
}

template class IVW_MODULE_ANIMATION_TMPL_INST KeyframeSequenceTyped<CameraKeyframe>;

}  // namespace animation

}  // namespace inviwo
