/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/util/glm.h>

namespace inviwo {

/**
 * \brief Camera with off axis perspective projection.
 *
 * The camera with unsymmetrical frustum for stereo in VR
 * Kooima, Robert. "Generalized perspective projection." School of Elect. Eng. and Computer Science
 * (2008): 1-7.
 * @see Camera
 */
class IVW_CORE_API SkewedPerspectiveCamera final : public Camera {
public:
    SkewedPerspectiveCamera(
        vec3 lookFrom = cameradefaults::lookFrom, vec3 lookTo = cameradefaults::lookTo,
        vec3 lookUp = cameradefaults::lookUp, float nearPlane = cameradefaults::nearPlane,
        float farPlane = cameradefaults::farPlane, float aspectRatio = cameradefaults::aspectRatio,
        float fieldOfView = cameradefaults::fieldOfView, vec2 frustumOffset = vec2(0.0f, 0.0f));
    virtual ~SkewedPerspectiveCamera() = default;
    SkewedPerspectiveCamera(const SkewedPerspectiveCamera& other);
    SkewedPerspectiveCamera& operator=(const SkewedPerspectiveCamera& other);
    virtual SkewedPerspectiveCamera* clone() const override;
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"SkewedPerspectiveCamera"};

    virtual void updateFrom(const Camera& source) override;
    virtual void configureProperties(CameraProperty& cameraProperty, bool attach) override;

    float getFovy() const;
    void setFovy(float val);

    const vec2& getOffset() const;
    void setOffset(vec2 val);
    virtual void zoom(float factor, std::optional<mat4> boundingBox) override;

    virtual void setLookFrom(vec3 val) override;
    virtual void setLookTo(vec3 val) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    virtual bool equal(const Camera& other) const override;
    virtual mat4 calculateViewMatrix() const override;
    virtual mat4 calculateProjectionMatrix() const override;

    // Left, right, bottom, top view volume
    float fovy_;
    vec2 offset_;
};

inline float SkewedPerspectiveCamera::getFovy() const { return fovy_; }
inline const vec2& SkewedPerspectiveCamera::getOffset() const { return offset_; }

}  // namespace inviwo
