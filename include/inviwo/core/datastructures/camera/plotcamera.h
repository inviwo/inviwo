/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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
 * \class PlotCamera
 * \brief Orthographic camera for plotting that does not preserve aspect ratios.
 *
 * The PlotCamera is a specialized orthographic camera designed for plotting and visualization
 * tasks where maintaining the original aspect ratio of the view is not required. Unlike standard
 * orthographic cameras, PlotCamera allows independent scaling of the view along each axis,
 * enabling flexible and non-uniform projections suitable for data visualization.
 */
class IVW_CORE_API PlotCamera final : public Camera {
public:
    PlotCamera(vec3 lookFrom = cameradefaults::lookFrom, vec3 lookTo = cameradefaults::lookTo,
               vec3 lookUp = cameradefaults::lookUp, float nearPlane = cameradefaults::nearPlane,
               float farPlane = cameradefaults::farPlane,
               float aspectRatio = cameradefaults::aspectRatio, vec2 size = vec2{300, 300});
    virtual ~PlotCamera() = default;
    PlotCamera(const PlotCamera& other);
    PlotCamera& operator=(const PlotCamera& other);
    virtual PlotCamera* clone() const override;
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"PlotCamera"};

    virtual void updateFrom(const Camera& source) override;
    virtual void configureProperties(CameraProperty& cameraProperty, bool attach) override;

    vec2 getSize() const { return size_; }
    void setSize(vec2 size);
    virtual void zoom(const ZoomOptions& opts) override;

    virtual vec4 getClipPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    virtual bool equal(const Camera& other) const override;
    virtual mat4 calculateProjectionMatrix() const override;

    vec2 size_;
};

}  // namespace inviwo
