/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/core/algorithm/camerautils.h>
#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/datastructures/camera/perspectivecamera.h>
#include <inviwo/core/datastructures/camera/skewedperspectivecamera.h>
#include <inviwo/core/datastructures/camera/orthographiccamera.h>
#include <inviwo/core/datastructures/camera/plotcamera.h>

#include <inviwo/core/util/glm.h>

#include <numbers>

namespace inviwo::camerautil {

namespace {

mat3 getView(Side side) {
    const auto [forward, up] = [&] -> std::pair<vec3, vec3> {
        switch (side) {
            case Side::XNegative:
                return {{1, 0, 0}, {0, 1, 0}};
            case Side::XPositive:
                return {{-1, 0, 0}, {0, 1, 0}};
            case Side::YNegative:
                return {{0, 1, 0}, {0, 0, 1}};
            case Side::YPositive:
                return {{0, -1, 0}, {0, 0, 1}};
            case Side::ZNegative:
                return {{0, 0, 1}, {0, 1, 0}};
            case Side::ZPositive:
                [[fallthrough]];
            default:
                return {{0, 0, -1}, {0, 1, 0}};
        }
    }();

    return glm::inverse(mat3{glm::cross(forward, up), up, forward});
}

constexpr std::array<vec3, 8> corners{
    {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}}};

std::tuple<vec3, vec3, vec3, vec2> fitOrthographicCameraView(const mat4& boundingBox,
                                                             glm::mat3 view, vec2 nearFar) {

    const auto unitCenter = vec3{.5f};
    const auto lookTo = vec3(boundingBox * vec4(unitCenter, 1.f));

    const auto viewPoints = corners | std::views::transform([&](const vec3 corner) {
                                const auto point = vec3(boundingBox * vec4(corner, 1.f));
                                const auto camPoint = view * (point - lookTo);
                                return camPoint;
                            });

    const auto max = std::ranges::fold_left(
        viewPoints, vec3{0}, [](const vec3& a, const vec3& b) { return glm::max(a, b); });
    const auto min = std::ranges::fold_left(
        viewPoints, vec3{0}, [](const vec3& a, const vec3& b) { return glm::min(a, b); });
    const auto size = max - min;

    const auto dist = std::clamp(size.z * 5.0f, nearFar.x * 5.0f, nearFar.y * 0.5f);

    const auto& [right, up, forward] = glm::inverse(view);
    return {lookTo + forward * dist, lookTo, up, vec2{size}};
}

std::tuple<vec3, vec3, vec3> fitPerspectiveCameraView(const mat4& boundingBox, glm::mat3 view,
                                                      float fitRatio, vec2 fov) {

    const auto unitCenter = vec3{.5f};
    const auto lookTo = vec3(boundingBox * vec4(unitCenter, 1.f));

    const vec2 scale{std::tan(std::numbers::pi_v<float> / 2.0f - fov.x / 2.0f),
                     std::tan(std::numbers::pi_v<float> / 2.0f - fov.y / 2.0f)};

    // Find the needed distance from the camera to lookTo given a field of view such that a
    // corner of the bounding box are within the view
    const auto dist = [&](vec3 corner) {
        const auto point = vec3(boundingBox * vec4(corner, 1.f));
        const auto camPoint = view * (point - lookTo);
        const auto max = glm::compMax(scale * glm::abs(vec2{camPoint})) - camPoint.z;
        return max;
    };

    // take the largest needed distance for all corners.
    const auto maxDist = std::ranges::max(corners | std::views::transform(dist));

    const auto& [right, up, forward] = glm::inverse(view);
    return {lookTo + forward * maxDist * fitRatio, lookTo, up};
}

}  // namespace

float fovxFrom(float fovy, float aspect) {
    return 2.0f * std::atan(std::tan(fovy / 2.0f) * aspect);
}
float fovxDegreesFrom(float fovyDegrees, float aspect) {
    return glm::degrees(fovxFrom(glm::radians(fovyDegrees), aspect));
}

void setCameraView(CameraProperty& cam, const mat4& boundingBox, glm::mat3 view, float fitRatio,
                   UpdateNearFar updateNearFar, UpdateLookRanges updateLookRanges,
                   float maxZoomFactor, float farNearRatio) {
    const NetworkLock lock(&cam);
    const mat4 fixedBBox = util::minExtentBoundingBox(boundingBox);

    if (updateNearFar == UpdateNearFar::Yes) {
        setCameraNearFar(cam, fixedBBox, maxZoomFactor, farNearRatio);
    }
    if (updateLookRanges == UpdateLookRanges::Yes) {
        setCameraLookRanges(cam, fixedBBox, maxZoomFactor);
    }

    if (auto* perspectiveCamera = dynamic_cast<PerspectiveCamera*>(&cam.get())) {
        const auto fovy = glm::radians(perspectiveCamera->getFovy());
        const auto fovx = fovxFrom(fovy, perspectiveCamera->getAspectRatio());
        const auto [lookFrom, lookTo, lookUp] =
            fitPerspectiveCameraView(fixedBBox, view, fitRatio, vec2{fovx, fovy});
        perspectiveCamera->setLook(lookFrom, lookTo, lookUp);

    } else if (auto* orthoCamera = dynamic_cast<OrthographicCamera*>(&cam.get())) {
        const auto nearFar = vec2{orthoCamera->getNearPlaneDist(), orthoCamera->getFarPlaneDist()};
        const auto [lookFrom, lookTo, lookUp, size] =
            fitOrthographicCameraView(fixedBBox, view, nearFar);

        orthoCamera->setLook(lookFrom, lookTo, lookUp);
        const auto aspect = orthoCamera->getAspectRatio();
        if (size.y * aspect > size.x) {
            orthoCamera->setWidth(size.y * aspect * fitRatio);
        } else {
            orthoCamera->setWidth(size.x * fitRatio);
        }

    } else if (auto* plotCamera = dynamic_cast<PlotCamera*>(&cam.get())) {
        const auto nearFar = vec2{plotCamera->getNearPlaneDist(), plotCamera->getFarPlaneDist()};
        const auto [lookFrom, lookTo, lookUp, size] =
            fitOrthographicCameraView(fixedBBox, view, nearFar);
        plotCamera->setLook(lookFrom, lookTo, lookUp);
        plotCamera->setSize(size * fitRatio);

    } else if (auto* skewedCamera = dynamic_cast<SkewedPerspectiveCamera*>(&cam.get())) {
        const auto fovy = glm::radians(skewedCamera->getFovy());
        const auto fovx = fovxFrom(fovy, skewedCamera->getAspectRatio());
        const auto [lookFrom, lookTo, lookUp] =
            fitPerspectiveCameraView(fixedBBox, view, fitRatio, vec2{fovx, fovy});
        skewedCamera->setLook(lookFrom, lookTo, lookUp);

    } else {
        log::warn("setCameraView does not support {}", cam.get().getClassIdentifier());
    }
}

void setCameraView(CameraProperty& cam, const mat4& boundingBox, float fitRatio,
                   UpdateNearFar updateNearFar, UpdateLookRanges updateLookRanges,
                   float maxZoomFactor, float farNearRatio) {
    setCameraView(cam, boundingBox, mat3{cam.viewMatrix()}, fitRatio, updateNearFar,
                  updateLookRanges, maxZoomFactor, farNearRatio);
}

void setCameraView(CameraProperty& cam, const mat4& boundingBox, Side side, float fitRatio,
                   UpdateNearFar updateNearFar, UpdateLookRanges updateLookRanges,
                   float maxZoomFactor, float farNearRatio) {

    auto box = mat3{util::minExtentBoundingBox(boundingBox)};
    box = mat3{glm::normalize(box[0]), glm::normalize(box[1]), glm::normalize(box[2])};

    setCameraView(cam, boundingBox, getView(side), fitRatio, updateNearFar, updateLookRanges,
                  maxZoomFactor, farNearRatio);
}

void setCameraLookRanges(CameraProperty& cam, const mat4& boundingBox, float zoomRange) {
    const NetworkLock lock(&cam);

    const vec3 lookTo(boundingBox * vec4(vec3(.5f), 1.f));

    const vec3 dx(boundingBox[0]);
    const vec3 dy(boundingBox[1]);
    const vec3 dz(boundingBox[2]);

    auto p0 = lookTo + (dx + dy + dz) * zoomRange;
    auto p1 = lookTo - (dx + dy + dz) * zoomRange;
    cam.lookFrom_.setMinValue(glm::min(p0, p1));
    cam.lookFrom_.setMaxValue(glm::max(p0, p1));
    p0 = lookTo + (dx + dy + dz);
    p1 = lookTo - (dx + dy + dz);
    cam.lookTo_.setMinValue(glm::min(p0, p1));
    cam.lookTo_.setMaxValue(glm::max(p0, p1));
}

std::pair<float, float> computeCameraNearFar(const mat4& boundingBox, float zoomRange,
                                             float farNearRatio) {
    const vec3 bx(boundingBox[0]);
    const vec3 by(boundingBox[1]);
    const vec3 bz(boundingBox[2]);

    const float dx = glm::length(bx);
    const float dy = glm::length(by);
    const float dz = glm::length(bz);

    const auto d = std::max({dx, dy, dz});

    const auto newFar = d * (zoomRange + 1);
    const auto newNear = newFar / farNearRatio;
    return {newNear, newFar};
}

void setCameraNearFar(CameraProperty& cam, const mat4& boundingBox, float zoomRange,
                      float farNearRatio) {

    auto [newNear, newFar] = computeCameraNearFar(boundingBox, zoomRange, farNearRatio);

    cam.setNearFarPlaneDist(newNear, newFar);
}

}  // namespace inviwo::camerautil
