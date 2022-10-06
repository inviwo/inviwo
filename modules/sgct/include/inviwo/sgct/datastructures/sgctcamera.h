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

#include <inviwo/sgct/sgctmoduledefine.h>
#include <inviwo/core/datastructures/camera.h>

namespace sgct {
struct RenderData;
}

namespace inviwo {

class IVW_MODULE_SGCT_API SGCTCamera final : public Camera {
public:
    SGCTCamera(vec3 lookFrom = cameradefaults::lookFrom, vec3 lookTo = cameradefaults::lookTo,
               vec3 lookUp = cameradefaults::lookUp, float nearPlane = cameradefaults::nearPlane,
               float farPlane = cameradefaults::farPlane,
               float fieldOfView = cameradefaults::fieldOfView,
               float aspectRatio = cameradefaults::aspectRatio);
    virtual ~SGCTCamera() = default;
    SGCTCamera(const SGCTCamera& other);
    SGCTCamera& operator=(const SGCTCamera& other);
    virtual SGCTCamera* clone() const override;
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    virtual void updateFrom(const Camera& source);
    virtual void configureProperties(CameraProperty& cameraProperty, bool attach) override;

    float getFovy() const;
    void setFovy(float val);
    void setExternal(const sgct::RenderData& renderData);
    virtual void zoom(float factor, std::optional<mat4> boundingBox) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    virtual bool equal(const Camera& other) const override;
    virtual mat4 calculateProjectionMatrix() const override;
    virtual mat4 calculateViewMatrix() const override;

    float fovy_;

    std::optional<mat4> extProj_{};
    mat4 extView_{1.0f};
    mat4 extModel_{1.0f};
};

inline float SGCTCamera::getFovy() const { return fovy_; }

}  // namespace inviwo
