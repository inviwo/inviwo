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

#pragma once

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/observer.h>

#include <modules/animation/datastructures/basekeyframesequence.h>
#include <modules/animation/datastructures/camerakeyframe.h>
#include <modules/animation/datastructures/valuekeyframesequence.h>
#include <modules/animation/interpolation/camerainterpolation.h>

namespace inviwo {

namespace animation {
/** \class CameraKeyframeSequence
 * KeyframeSequence for a given type of KeyFames.
 * @see KeyframeSequence
 */

class IVW_MODULE_ANIMATION_API CameraKeyframeSequence : public BaseKeyframeSequence<CameraKeyframe>, public ValueKeyframeSequence {
public:
    using key_type = CameraKeyframe;
    using value_type = typename CameraKeyframe::value_type;

    CameraKeyframeSequence();
    CameraKeyframeSequence(std::vector<std::unique_ptr<CameraKeyframe>> keyframes);
    CameraKeyframeSequence(std::vector<std::unique_ptr<CameraKeyframe>> keyframes,
                          std::unique_ptr<CameraInterpolation> interpolation);

    CameraKeyframeSequence(const CameraKeyframeSequence& rhs);
    CameraKeyframeSequence& operator=(const CameraKeyframeSequence& that);

    /**
     * Remove all keyframes and call KeyframeObserver::notifyKeyframeRemoved
     */
    virtual ~CameraKeyframeSequence();

    virtual CameraKeyframeSequence* clone() const override;

    virtual void operator()(Seconds from, Seconds to, Camera& out) const;

    virtual const CameraInterpolation& getInterpolation() const override;
    virtual void setInterpolation(std::unique_ptr<Interpolation> interpolation) override;
    void setInterpolation(std::unique_ptr<CameraInterpolation> interpolation);

    virtual easing::EasingType getEasingType() const override;
    virtual void setEasingType(easing::EasingType easing) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    easing::EasingType easing_{easing::EasingType::Linear};
    std::unique_ptr<CameraInterpolation> interpolation_;
};


IVW_MODULE_ANIMATION_API bool operator==(const CameraKeyframeSequence& a,
                                         const CameraKeyframeSequence& b);

IVW_MODULE_ANIMATION_API bool operator!=(const CameraKeyframeSequence& a,
                                         const CameraKeyframeSequence& b);


}  // namespace animation

}  // namespace inviwo

