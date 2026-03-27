/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2026 Inviwo Foundation
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
#include <inviwo/core/algorithm/camerautils.h>

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
IVW_CORE_API DoubleRefProperty* getCameraFovProperty(CameraProperty& cameraProperty);

/**
 * @brief Create a vertical fov property for use in a camera property
 * @see PerspectiveCamera
 * @see SkewedPerspectiveCamera
 */
IVW_CORE_API std::unique_ptr<DoubleRefProperty> createCameraFovProperty(
    std::function<double()> get, std::function<void(const double&)> set);

/**
 * @brief Either return an existing vertical fov property updated with the provided get and set
 * functions or create a new one. The new one will automatically be added to the camera property
 * @see PerspectiveCamera
 * @see SkewedPerspectiveCamera
 */
IVW_CORE_API DoubleRefProperty* updateOrCreateCameraFovProperty(
    CameraProperty& cameraProperty, std::function<double()> get,
    std::function<void(const double&)> set);

/**
 * @brief Find the width property in the cameraProperty
 * @param cameraProperty
 * @return the width property if found, nullptr otherwise
 * @see OrthographicCamera
 */
IVW_CORE_API DoubleRefProperty* getCameraWidthProperty(CameraProperty& cameraProperty);

/**
 * @brief Create a width property for use in a CameraProperty
 * @see OrthographicCamera
 */
IVW_CORE_API std::unique_ptr<DoubleRefProperty> createCameraWidthProperty(
    std::function<double()> get, std::function<void(const double&)> set);

/**
 * @brief Either return an existing width property updated with the provided get and set functions
 * or create a new one. The new one will automatically be added to the camera property
 * @see OrthographicCamera
 */
IVW_CORE_API DoubleRefProperty* updateOrCreateCameraWidthProperty(
    CameraProperty& cameraProperty, std::function<double()> get,
    std::function<void(const double&)> set);

/**
 * @brief Find the eye offset property in the cameraProperty
 * @param cameraProperty
 * @return the eye offset property if found, nullptr otherwise
 * @see SkewedPerspectiveCamera
 */
IVW_CORE_API DoubleVec2RefProperty* getCameraEyeOffsetProperty(CameraProperty& cameraProperty);

/**
 * @brief Create an eye offset property for use in a camera property
 * @see SkewedPerspectiveCamera
 */
IVW_CORE_API std::unique_ptr<DoubleVec2RefProperty> createCameraEyeOffsetProperty(
    std::function<dvec2()> get, std::function<void(const dvec2&)> set);

/**
 * @brief Either return an existing eye offset property updated with the provided get and set
 * functions or create a new one. The new one will automatically be added to the camera property
 * @see SkewedPerspectiveCamera
 */
IVW_CORE_API DoubleVec2RefProperty* updateOrCreateCameraEyeOffsetProperty(
    CameraProperty& cameraProperty, std::function<dvec2()> get,
    std::function<void(const dvec2&)> set);

IVW_CORE_API dvec2 fovyToSize(double fovy, double distance, double aspect);
IVW_CORE_API double fovyToWidth(double fovy, double distance, double aspect);
IVW_CORE_API double widthToFovy(double width, double distance, double aspect);
IVW_CORE_API double widthToViewDist(double width, double fov, double aspect);

struct IVW_CORE_API FovBounds {
    std::optional<std::pair<dvec2, dvec2>> bounds;
    bool nearPlaneClipped;
    bool farPlaneClipped;
};

IVW_CORE_API FovBounds calculateFovBounds(const glm::dmat4& boundingBox, const glm::dvec3& lookFrom,
                                          const glm::dvec3& lookTo, const glm::dvec3& lookUp,
                                          double nearPlane, double farPlane);

IVW_CORE_API bool canZoomBounded(const FovBounds& bounds, glm::dvec2 fov, double zoomFactor);

template <typename CamType>
void perspectiveZoom(CamType& cam, const ZoomOptions& opts) {
    const auto direction = cam.getLookFrom() - cam.getLookTo();
    if (opts.origin) {

        const auto up = cam.getLookUp();
        const auto dir = -glm::normalize(cam.getDirection());
        const auto right = glm::cross(up, dir);
        const auto basis = dmat3(right, up, dir);

        const auto size =
            fovyToSize(cam.getFovy(), glm::length(cam.getDirection()), cam.getAspectRatio());
        const auto translate = glm::translate(dvec3{0.5 * size * opts.origin.value(), 0.0});
        const auto scale = glm::scale(dvec3{1.0 - opts.factor.y, 1.0 - opts.factor.y, 1.0});
        const auto m = translate * scale * glm::inverse(translate);
        const auto offset = basis * dvec3{m * dvec4{0.0, 0.0, 0.0, 1.0}};

        cam.setLook(cam.getLookFrom() + offset - direction * opts.factor.y,
                    cam.getLookTo() + offset, cam.getLookUp());
    } else {
        const auto newFrom = cam.getLookFrom() - direction * opts.factor.y;

        if (opts.bounded == ZoomOptions::Bounded::Yes && opts.boundingBox &&
            !opts.boundingBox()
                 .transform([&](const dmat4& bb) {
                     const auto bounds =
                         calculateFovBounds(bb, newFrom, cam.getLookTo(), cam.getLookUp(),
                                            cam.getNearPlaneDist(), cam.getFarPlaneDist());

                     const auto fovy = glm::radians(cam.getFovy());
                     const auto fovx = camerautil::fovxFrom(fovy, cam.getAspectRatio());
                     return canZoomBounded(bounds, vec2{fovx, fovy}, opts.factor.y);
                 })
                 .value_or(true)) {
            return;
        }

        cam.setLookFrom(newFrom);
    }
}

}  // namespace util

}  // namespace inviwo
