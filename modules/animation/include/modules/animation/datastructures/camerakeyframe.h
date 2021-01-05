/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2021 Inviwo Foundation
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
#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/datastructures/camera/perspectivecamera.h>
#include <modules/animation/datastructures/basekeyframe.h>

namespace inviwo {

namespace animation {

/** \class CameraKeyframe
 * Keyframe of Camera value type.
 * Stores the KeyFrame value at a given time.
 * @note Only considers the Camera lookFrom, lookTo and lookUp
 * @see Keyframe
 */
class IVW_MODULE_ANIMATION_API CameraKeyframe : public BaseKeyframe {
public:
    using value_type = Camera;
    CameraKeyframe() = default;
    CameraKeyframe(Seconds time);
    CameraKeyframe(Seconds time, const Camera& cam);
    virtual ~CameraKeyframe() = default;
    virtual CameraKeyframe* clone() const override;

    CameraKeyframe(const CameraKeyframe& rhs) = default;
    CameraKeyframe& operator=(const CameraKeyframe& that) = default;

    const vec3& getLookFrom() const;
    virtual void setLookFrom(vec3 val);

    const vec3& getLookTo() const;
    virtual void setLookTo(vec3 val);

    const vec3& getLookUp() const;
    virtual void setLookUp(vec3 val);

    /**
     * \brief Get unnormalized direction of camera: lookTo - lookFrom
     */
    vec3 getDirection() const;

    void updateFrom(const Camera& value);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    static const std::string& getName() {
        static std::string name{"Camera"};
        return name;
    }

private:
    vec3 lookFrom_{cameradefaults::lookFrom};
    vec3 lookTo_{cameradefaults::lookTo};
    vec3 lookUp_{cameradefaults::lookUp};
};

IVW_MODULE_ANIMATION_API bool operator==(const CameraKeyframe& a, const CameraKeyframe& b);
IVW_MODULE_ANIMATION_API bool operator!=(const CameraKeyframe& a, const CameraKeyframe& b);

}  // namespace animation

}  // namespace inviwo
