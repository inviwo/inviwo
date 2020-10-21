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

/** \class CameraSphericalInterpolation
 * Spherical interpolation between two neighboring key frames.
 * 1. Orbit around lookAt if lookFrom's are different between key frames.
 * 2. Pan/tilt (rotate lookAt between key frames) otherwise.
 *
 * @note Only modifies lookFrom, lookAt, lookUp.
 * @see CameraPanTiltInterpolation
 */
class IVW_MODULE_ANIMATION_API CameraSphericalInterpolation
    : public InterpolationTyped<CameraKeyframe, CameraKeyframe::value_type> {
public:
    CameraSphericalInterpolation() = default;
    virtual ~CameraSphericalInterpolation() = default;
    virtual CameraSphericalInterpolation* clone() const override;

    virtual std::string getName() const override;

    static std::string classIdentifier();
    virtual std::string getClassIdentifier() const override;

    virtual bool equal(const Interpolation& other) const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;
    /*
     * Uses orbit between two key frames if lookFrom has changed and pan/tilt otherwise.
     * Orbit: Rotate the lookFrom around the lookAt position.
     * lookAt position will be linearly interpolated.
     * Pan/tilt: Rotate lookAt between key frames.
     */
    virtual void operator()(const std::vector<std::unique_ptr<CameraKeyframe>>& keys, Seconds from,
                            Seconds to, easing::EasingType easing, Camera& out) const;
};

}  // namespace animation

}  // namespace inviwo
