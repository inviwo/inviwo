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

namespace inviwo {

namespace camerautil {

namespace {

vec3 getViewDir(Side side) {
    switch (side) {
        case Side::XNegative:
            return {1, 0, 0};
        case Side::XPositive:
            return {-1, 0, 0};
        case Side::YNegative:
            return {0, 1, 0};
        case Side::YPositive:
            return {0, -1, 0};
        case Side::ZNegative:
            return {0, 0, 1};
        case Side::ZPositive:
            [[fallthrough]];
        default:
            return {0, 0, -1};
    }
}

vec3 getLookUp(Side side) {
    switch (side) {
        case Side::XNegative:
            [[fallthrough]];
        case Side::XPositive:
            return {0, 1, 0};

        case Side::YNegative:
            [[fallthrough]];
        case Side::YPositive:
            return {0, 0, 1};

        case Side::ZNegative:
            [[fallthrough]];
        case Side::ZPositive:
            [[fallthrough]];
        default:
            return {0, 1, 0};
    }
}

constexpr std::array<vec3, 8> corners{
    {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}}};

std::tuple<vec3, vec3, vec3, vec2> fitOrthographicCameraView(const mat4& boundingBox,
                                                             vec3 inViewDir, vec3 inLookUp) {

    const auto offset = vec3{.5f};
    const auto lookTo = vec3(boundingBox * vec4(offset, 1.f));

    // Camera basis
    const auto forward = glm::normalize(inViewDir);
    const glm::vec3 right = glm::normalize(glm::cross(forward, inLookUp));
    const glm::vec3 up = glm::cross(right, forward);
    const auto cb = glm::inverse(mat3{right, up, forward});

    const auto viewPoints = corners | std::views::transform([&](const vec3 corner) {
                                const auto point = vec3(boundingBox * vec4(corner, 1.f));
                                const auto camPoint = cb * point;
                                return camPoint;
                            });

    const auto max = std::ranges::fold_left(viewPoints, vec3{0}, [](const vec3& a, const vec3& b) {
        return glm::max(glm::abs(a), glm::abs(b));
    });

    return {lookTo + forward * max.z * 5.0f, lookTo, up, vec2{max}};
}

template <typename CamType>
std::tuple<vec3, vec3, vec3> fitPerspectiveCameraView(const CamType& cam, const mat4& boundingBox,
                                                      vec3 inViewDir, vec3 inLookUp,
                                                      float fitRatio) {

    const auto offset = vec3{.5f};
    const auto lookTo = vec3(boundingBox * vec4(offset, 1.f));

    // Camera basis
    const auto forward = glm::normalize(inViewDir);
    const auto up = glm::normalize(inLookUp);
    const auto right = glm::cross(forward, up);

    const float fovy = cam.getFovy() / 2.0f;
    const float fovx = glm::degrees(std::atan(cam.getAspectRatio() * std::tan(glm::radians(fovy))));

    // Find the needed distance from the camera to lookTo given a field of view such that a
    // corner of the bounding box are within the view
    const auto dist = [&](vec3 corner) {
        const auto point = vec3(boundingBox * vec4(corner, 1.f));

        const auto d0 = glm::dot(point - lookTo, -forward);
        const auto height = glm::abs(glm::dot(point - lookTo, up)) * fitRatio;
        const auto width = glm::abs(glm::dot(point - lookTo, right)) * fitRatio;
        const float d1 = height * std::tan(glm::radians(90 - fovy));
        const float d2 = width * std::tan(glm::radians(90 - fovx));
        return d0 + std::max(d1, d2);
    };

    // take the largest needed distance for all corners.
    const auto it = std::max_element(corners.begin(), corners.end(),
                                     [&](vec3 a, vec3 b) { return dist(a) < dist(b); });
    const auto lookOffset = dist(*it) * forward;

    return {lookTo + lookOffset, lookTo, up};
}

}  // namespace

void setCameraView(CameraProperty& cam, const mat4& boundingBox, vec3 inViewDir, vec3 inLookUp,
                   float fitRatio, UpdateNearFar updateNearFar, UpdateLookRanges updateLookRanges,
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

        auto [lookFrom, lookTo, lookUp] =
            fitPerspectiveCameraView(*perspectiveCamera, fixedBBox, inViewDir, inLookUp, fitRatio);
        cam.setLook(lookFrom, lookTo, lookUp);
    } else if (auto* orthographicCamera = dynamic_cast<OrthographicCamera*>(&cam.get())) {
        auto [lookFrom, lookTo, lookUp, range] =
            fitOrthographicCameraView(fixedBBox, inViewDir, inLookUp);

        orthographicCamera->setLook(lookFrom, lookTo, lookUp);
        const auto aspect = orthographicCamera->getAspectRatio();
        if (range.y * aspect > range.x) {
            orthographicCamera->setWidth(range.y * aspect * 2 * fitRatio);
        } else {
            orthographicCamera->setWidth(range.x * 2 * fitRatio);
        }
    } else if (auto* plotCamera = dynamic_cast<PlotCamera*>(&cam.get())) {
        auto [lookFrom, lookTo, lookUp, range] =
            fitOrthographicCameraView(fixedBBox, inViewDir, inLookUp);
        plotCamera->setLook(lookFrom, lookTo, lookUp);
        plotCamera->setSize(2 * range * fitRatio);
    } else if (auto* skewedPerspectiveCamera = dynamic_cast<SkewedPerspectiveCamera*>(&cam.get())) {

        auto [lookFrom, lookTo, lookUp] = fitPerspectiveCameraView(
            *skewedPerspectiveCamera, fixedBBox, inViewDir, inLookUp, fitRatio);
        cam.setLook(lookFrom, lookTo, lookUp);
    } else {
        log::warn("setCameraView does not support {}", cam.get().getClassIdentifier());
    }
}

void setCameraView(CameraProperty& cam, const mat4& boundingBox, float fitRatio,
                   UpdateNearFar updateNearFar, UpdateLookRanges updateLookRanges,
                   float maxZoomFactor, float farNearRatio) {
    setCameraView(cam, boundingBox, cam.getLookTo() - cam.getLookFrom(), cam.getLookUp(), fitRatio,
                  updateNearFar, updateLookRanges, maxZoomFactor, farNearRatio);
}

void setCameraView(CameraProperty& cam, const mat4& boundingBox, Side side, float fitRatio,
                   UpdateNearFar updateNearFar, UpdateLookRanges updateLookRanges,
                   float maxZoomFactor, float farNearRatio) {
    const mat4 fixedBBox = util::minExtentBoundingBox(boundingBox);
    const auto viewDir = mat3(fixedBBox) * getViewDir(side);
    const auto lookUp = mat3(fixedBBox) * getLookUp(side);
    setCameraView(cam, fixedBBox, viewDir, lookUp, fitRatio, updateNearFar, updateLookRanges,
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

}  // namespace camerautil

}  // namespace inviwo
