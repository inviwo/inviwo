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

#include <modules/animation/datastructures/camerakeyframesequence.h>
#include <modules/animation/interpolation/camerasphericalinterpolation.h>

#include <algorithm>

namespace inviwo {

namespace animation {

CameraKeyframeSequence::CameraKeyframeSequence()
    : BaseKeyframeSequence<CameraKeyframe>{}
    , ValueKeyframeSequence{}
    , interpolation_{std::make_unique<CameraSphericalInterpolation>()} {}

CameraKeyframeSequence::CameraKeyframeSequence(
    std::vector<std::unique_ptr<CameraKeyframe>> keyframes)
    : BaseKeyframeSequence<CameraKeyframe>{std::move(keyframes)}
    , ValueKeyframeSequence()
    , interpolation_{std::make_unique<CameraSphericalInterpolation>()} {}

CameraKeyframeSequence::CameraKeyframeSequence(
    std::vector<std::unique_ptr<CameraKeyframe>> keyframes,
    std::unique_ptr<CameraInterpolation> interpolation)
    : BaseKeyframeSequence<CameraKeyframe>{std::move(keyframes)}
    , ValueKeyframeSequence()
    , interpolation_{std::move(interpolation)} {}

CameraKeyframeSequence::CameraKeyframeSequence(const CameraKeyframeSequence& rhs)
    : BaseKeyframeSequence<CameraKeyframe>(rhs)
    , ValueKeyframeSequence(rhs)
    , interpolation_(std::unique_ptr<CameraInterpolation>(rhs.interpolation_->clone())) {}

CameraKeyframeSequence& CameraKeyframeSequence::operator=(const CameraKeyframeSequence& that) {
    if (this != &that) {
        BaseKeyframeSequence<CameraKeyframe>::operator=(that);
        ValueKeyframeSequence::operator=(that);
        setInterpolation(std::unique_ptr<CameraInterpolation>(that.interpolation_->clone()));
        setEasingType(that.easing_);
    }
    return *this;
}

CameraKeyframeSequence::~CameraKeyframeSequence() = default;

CameraKeyframeSequence* CameraKeyframeSequence::clone() const {
    return new CameraKeyframeSequence(*this);
}

void CameraKeyframeSequence::operator()(Seconds from, Seconds to, Camera& out) const {
    if (interpolation_) {
        (*interpolation_)(this->keyframes_, from, to, easing_, out);
    } else {
        out.updateFrom(this->keyframes_.front()->getValue());
    }
}

const CameraInterpolation& CameraKeyframeSequence::getInterpolation() const {
    return *interpolation_;
}

void CameraKeyframeSequence::setInterpolation(std::unique_ptr<CameraInterpolation> interpolation) {
    if (interpolation && !interpolation_->equal(*interpolation)) {
        interpolation_ = std::move(interpolation);
        notifyValueKeyframeSequenceInterpolationChanged(this);
    }
}

void CameraKeyframeSequence::setInterpolation(std::unique_ptr<Interpolation> interpolation) {
    if (auto inter = util::dynamic_unique_ptr_cast<CameraInterpolation>(std::move(interpolation))) {
        setInterpolation(std::move(inter));
    } else {
        throw Exception("Interpolation type does not match key", IVW_CONTEXT);
    }
}

easing::EasingType CameraKeyframeSequence::getEasingType() const { return easing_; }

void CameraKeyframeSequence::setEasingType(easing::EasingType easing) {
    if (easing_ != easing) {
        easing_ = easing;
        notifyValueKeyframeSequenceEasingChanged(this);
    }
}

void CameraKeyframeSequence::serialize(Serializer& s) const {
    BaseKeyframeSequence<CameraKeyframe>::serialize(s);
    s.serialize("easing", easing_);
    s.serialize("interpolation", interpolation_);
}

void CameraKeyframeSequence::deserialize(Deserializer& d) {
    BaseKeyframeSequence<CameraKeyframe>::deserialize(d);
    {
        easing::EasingType easing = easing_;
        d.deserialize("easing", easing);
        setEasingType(easing);
    }
    {
        std::unique_ptr<CameraInterpolation> interpolation;
        d.deserializeAs<Interpolation>("interpolation", interpolation);
        setInterpolation(std::move(interpolation));
    }
}

bool operator==(const CameraKeyframeSequence& a, const CameraKeyframeSequence& b) {
    return a.getEasingType() == b.getEasingType() && a.getInterpolation() == b.getInterpolation() &&
           std::equal(a.begin(), a.end(), b.begin(), b.end(),
                      [](const auto& a, const auto& b) { return a == b; });
}

bool operator!=(const CameraKeyframeSequence& a, const CameraKeyframeSequence& b) {
    return !(a == b);
}

}  // namespace animation

}  // namespace inviwo
