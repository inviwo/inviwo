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
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/datastructures/camera/camera.h>

#include <glm/gtx/transform.hpp>

#include <memory>

namespace inviwo {

class CameraProperty;

namespace util {

/**
 * @brief Find the vertical fov property in the cameraProperty
 * @param cameraProperty
 * @return the fov property if found, nullptr otherwise
 * @see PerspectiveCamera
 * @see SkewedPerspectiveCamera
 */
IVW_CORE_API FloatRefProperty* getCameraFovProperty(CameraProperty& cameraProperty);

/**
 * @brief Create a vertical fov property for use in a camera property
 * @see PerspectiveCamera
 * @see SkewedPerspectiveCamera
 */
IVW_CORE_API std::unique_ptr<FloatRefProperty> createCameraFovProperty(
    std::function<float()> get, std::function<void(const float&)> set);

/**
 * @brief Either return an existing vertical fov property updated with the provided get and set
 * functions or create a new one. The new one will automatically be added to the camera property
 * @see PerspectiveCamera
 * @see SkewedPerspectiveCamera
 */
IVW_CORE_API FloatRefProperty* updateOrCreateCameraFovProperty(
    CameraProperty& cameraProperty, std::function<float()> get,
    std::function<void(const float&)> set);

/**
 * @brief Find the width property in the cameraProperty
 * @param cameraProperty
 * @return the width property if found, nullptr otherwise
 * @see OrthographicCamera
 */
IVW_CORE_API FloatRefProperty* getCameraWidthProperty(CameraProperty& cameraProperty);

/**
 * @brief Create a width property for use in a CameraProperty
 * @see OrthographicCamera
 */
IVW_CORE_API std::unique_ptr<FloatRefProperty> createCameraWidthProperty(
    std::function<float()> get, std::function<void(const float&)> set);

/**
 * @brief Either return an existing width property updated with the provided get and set functions
 * or create a new one. The new one will automatically be added to the camera property
 * @see OrthographicCamera
 */
IVW_CORE_API FloatRefProperty* updateOrCreateCameraWidthProperty(
    CameraProperty& cameraProperty, std::function<float()> get,
    std::function<void(const float&)> set);

/**
 * @brief Find the eye offset property in the cameraProperty
 * @param cameraProperty
 * @return the eye offset property if found, nullptr otherwise
 * @see SkewedPerspectiveCamera
 */
IVW_CORE_API FloatVec2RefProperty* getCameraEyeOffsetProperty(CameraProperty& cameraProperty);

/**
 * @brief Create an eye offset property for use in a camera property
 * @see SkewedPerspectiveCamera
 */
IVW_CORE_API std::unique_ptr<FloatVec2RefProperty> createCameraEyeOffsetProperty(
    std::function<vec2()> get, std::function<void(const vec2&)> set);

/**
 * @brief Either return an existing eye offset property updated with the provided get and set
 * functions or create a new one. The new one will automatically be added to the camera property
 * @see SkewedPerspectiveCamera
 */
IVW_CORE_API FloatVec2RefProperty* updateOrCreateCameraEyeOffsetProperty(
    CameraProperty& cameraProperty, std::function<vec2()> get,
    std::function<void(const vec2&)> set);

IVW_CORE_API vec2 fovyToSize(float fovy, float distance, float aspect);
IVW_CORE_API float fovyToWidth(float fovy, float distance, float aspect);
IVW_CORE_API float widthToFovy(float width, float distance, float aspect);
IVW_CORE_API float widthToViewDist(float width, float fov, float aspect);

template <typename CamType>
void perspectiveZoom(CamType& cam, const ZoomOptions& opts) {
    const auto direction = cam.getLookFrom() - cam.getLookTo();
    if (opts.origin) {

        const auto up = cam.getLookUp();
        const auto dir = -glm::normalize(cam.getDirection());
        const auto right = glm::cross(up, dir);
        const auto basis = mat3(right, up, dir);

        const auto size =
            fovyToSize(cam.getFovy(), glm::length(cam.getDirection()), cam.getAspectRatio());
        const auto translate = glm::translate(vec3{0.5f * size * opts.origin.value(), 0.f});
        const auto scale = glm::scale(vec3{1.0f - opts.factor.y, 1.0f - opts.factor.y, 1.f});
        const auto m = translate * scale * glm::inverse(translate);
        const auto offset = basis * vec3{m * vec4{0.f, 0.f, 0.f, 1.f}};

        cam.setLook(cam.getLookFrom() + offset - direction * opts.factor.y,
                    cam.getLookTo() + offset, cam.getLookUp());
    } else {
        cam.setLookFrom(cam.getLookFrom() - direction * opts.factor.y);
    }
}

}  // namespace util

}  // namespace inviwo
