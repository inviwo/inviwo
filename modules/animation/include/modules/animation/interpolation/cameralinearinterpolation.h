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
#pragma once

#include <modules/animation/animationmoduledefine.h>
#include <modules/animation/datastructures/camerakeyframe.h>
#include <modules/animation/interpolation/interpolation.h>

namespace inviwo {

namespace animation {

/** \class CameraLinearInterpolation
 * Perfoms linear interpolation between lookFrom and lookTo of two neighboring key frames.
 * lookUp is rotated between the two neighboring keyframes.
 * Generally, you do not want to linearly interpolate lookFrom/lookTo - use
 * CameraSphericalInterpolation instead.
 * @note Only considers lookFrom, lookAt, lookUp in Camera
 * @see CameraSphericalInterpolation
 */
class IVW_MODULE_ANIMATION_API CameraLinearInterpolation
    : public InterpolationTyped<CameraKeyframe, CameraKeyframe::value_type> {
public:
    CameraLinearInterpolation() = default;
    virtual ~CameraLinearInterpolation() = default;

    virtual CameraLinearInterpolation* clone() const override;

    virtual std::string getName() const override;

    static std::string classIdentifier();
    virtual std::string getClassIdentifier() const override;

    virtual bool equal(const Interpolation& other) const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    /*
     * Returns linear interpolation of keyframe values at time t.
     */
    virtual void operator()(const std::vector<std::unique_ptr<CameraKeyframe>>& keys, Seconds from,
                            Seconds to, easing::EasingType easing, CameraKeyframe::value_type& out) const override;
};

}  // namespace animation

}  // namespace inviwo
