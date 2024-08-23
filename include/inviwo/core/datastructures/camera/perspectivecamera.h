/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmmat.h>

#include <glm/ext/matrix_clip_space.hpp>

namespace inviwo {

class IVW_CORE_API PerspectiveCamera final : public Camera {
public:
    PerspectiveCamera(vec3 lookFrom = cameradefaults::lookFrom,
                      vec3 lookTo = cameradefaults::lookTo, vec3 lookUp = cameradefaults::lookUp,
                      float nearPlane = cameradefaults::nearPlane,
                      float farPlane = cameradefaults::farPlane,
                      float aspectRatio = cameradefaults::aspectRatio,
                      float fieldOfView = cameradefaults::fieldOfView);
    virtual ~PerspectiveCamera() = default;
    PerspectiveCamera(const PerspectiveCamera& other);
    PerspectiveCamera& operator=(const PerspectiveCamera& other);
    virtual PerspectiveCamera* clone() const override;
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    virtual void updateFrom(const Camera& source) override;
    virtual void configureProperties(CameraProperty& cameraProperty, bool attach) override;

    float getFovy() const;
    void setFovy(float val);

    virtual void zoom(float factor, std::optional<mat4> boundingBox) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    virtual bool equal(const Camera& other) const override;
    virtual mat4 calculateProjectionMatrix() const override;

    float fovy_;
};

inline float PerspectiveCamera::getFovy() const { return fovy_; }

inline mat4 PerspectiveCamera::calculateProjectionMatrix() const {
    return glm::perspective(glm::radians(fovy_), aspectRatio_, nearPlaneDist_, farPlaneDist_);
}

}  // namespace inviwo
