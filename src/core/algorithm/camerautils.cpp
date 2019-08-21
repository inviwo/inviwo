/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

namespace camerautil {

namespace detail {

vec3 getViewDir(Side side) {
    switch (side) {
        case Side::XNegative:
            return vec3(1, 0, 0);
        case Side::XPositive:
            return vec3(-1, 0, 0);
        case Side::YNegative:
            return vec3(0, 1, 0);
        case Side::YPositive:
            return vec3(0, -1, 0);
        case Side::ZNegative:
            return vec3(0, 0, 1);
        case Side::ZPositive:
            return vec3(0, 0, -1);
        default:
            return vec3(0, 0, -1);
    }
}

vec3 getLookUp(Side side) {
    switch (side) {
        case Side::XNegative:
            return vec3(0, 1, 0);
        case Side::XPositive:
            return vec3(0, 1, 0);
        case Side::YNegative:
            return vec3(1, 0, 0);
        case Side::YPositive:
            return vec3(1, 0, 0);
        case Side::ZNegative:
            return vec3(0, 1, 0);
        case Side::ZPositive:
            return vec3(0, 1, 0);
        default:
            return vec3(0, 1, 0);
    }
}

}  // namespace detail

void setCameraView(CameraProperty &cam, const mat4 &boundingBox, vec3 inViewDir, vec3 inLookUp,
                   float fitRatio, UpdateNearFar updateNearFar, UpdateLookRanges updateLookRanges) {
    if (auto perspectiveCamera = dynamic_cast<PerspectiveCamera *>(&cam.get())) {
        const auto offset = vec3{.5f};
        const auto lookTo = vec3(boundingBox * vec4(offset, 1.f));

        const auto viewDir = glm::normalize(inViewDir);
        const auto lookUp = glm::normalize(inLookUp);
        const auto sideDir = glm::cross(viewDir, lookUp);

        const float fovy = perspectiveCamera->getFovy() / 2.0f;
        const float fovx =
            glm::degrees(std::atan(cam.getAspectRatio() * std::tan(glm::radians(fovy))));

        const std::array<vec3, 8> corners = {vec3{0, 0, 0}, vec3{1, 0, 0}, vec3{1, 1, 0},
                                             vec3{0, 1, 0}, vec3{0, 0, 1}, vec3{1, 0, 1},
                                             vec3{1, 1, 1}, vec3{0, 1, 1}};

        // Find the needed distance from the camera to lookTo given a field of view such that a
        // corner of the bounding box are within the view
        const auto dist = [&](vec3 corner) {
            const auto point = vec3(boundingBox * vec4(corner, 1.f));

            const auto d0 = glm::dot(point - lookTo, -viewDir);
            const auto height = glm::abs(glm::dot(point - lookTo, lookUp)) * fitRatio;
            const auto width = glm::abs(glm::dot(point - lookTo, sideDir)) * fitRatio;
            const float d1 = height * std::tan(glm::radians(90 - fovy));
            const float d2 = width * std::tan(glm::radians(90 - fovx));
            return d0 + std::max(d1, d2);
        };

        // take the largest needed distance for all corners.
        const auto it = std::max_element(corners.begin(), corners.end(),
                                         [&](vec3 a, vec3 b) { return dist(a) < dist(b); });
        const auto lookOffset = dist(*it) * viewDir;

        NetworkLock lock(&cam);
        if (updateNearFar == UpdateNearFar::Yes) {
            setCameraNearFar(cam, boundingBox);
        }
        if (updateLookRanges == UpdateLookRanges::Yes) {
            setCameraLookRanges(cam, boundingBox);
        }
        cam.setLook(lookTo + lookOffset, lookTo, lookUp);
    } else {
        LogWarnCustom("camerautil::setCameraView",
                      "setCameraView only supports perspective cameras");
    }
}

void setCameraView(CameraProperty &cam, const mat4 &boundingBox, float fitRatio,
                   UpdateNearFar updateNearFar, UpdateLookRanges updateLookRanges) {
    setCameraView(cam, boundingBox, cam.getLookFrom() - cam.getLookTo(), cam.getLookUp(), fitRatio,
                  updateNearFar, updateLookRanges);
}

void setCameraView(CameraProperty &cam, const mat4 &boundingBox, Side side, float fitRatio,
                   UpdateNearFar updateNearFar, UpdateLookRanges updateLookRanges) {
    const auto viewDir = mat3(boundingBox) * detail::getViewDir(side);
    const auto lookUp = mat3(boundingBox) * detail::getLookUp(side);
    setCameraView(cam, boundingBox, viewDir, lookUp, fitRatio, updateNearFar, updateLookRanges);
}

void setCameraLookRanges(CameraProperty &cam, const mat4 &boundingBox, float zoomRange) {
    NetworkLock lock(&cam);

    vec3 lookTo(boundingBox * vec4(vec3(.5f), 1.f));

    vec3 dx(boundingBox[0]);
    vec3 dy(boundingBox[1]);
    vec3 dz(boundingBox[2]);

    auto p0 = lookTo + (dx + dy + dz) * zoomRange;
    auto p1 = lookTo - (dx + dy + dz) * zoomRange;
    cam.lookFrom_.setMinValue(glm::min(p0, p1));
    cam.lookFrom_.setMaxValue(glm::max(p0, p1));
    p0 = lookTo + (dx + dy + dz);
    p1 = lookTo - (dx + dy + dz);
    cam.lookTo_.setMinValue(glm::min(p0, p1));
    cam.lookTo_.setMaxValue(glm::max(p0, p1));
}

std::pair<float, float> computeCameraNearFar(const mat4 &boundingBox, float zoomRange,
                                             float nearFarRatio) {
    vec3 bx(boundingBox[0]);
    vec3 by(boundingBox[1]);
    vec3 bz(boundingBox[2]);

    float dx = glm::length(bx);
    float dy = glm::length(by);
    float dz = glm::length(bz);

    auto d = std::max({dx, dy, dz});

    auto newFar = d * (zoomRange + 1);
    auto newNear = newFar * nearFarRatio;
    return {newNear, newFar};
}

void setCameraNearFar(CameraProperty &cam, const mat4 &boundingBox, float zoomRange,
                      float nearFarRatio) {

    auto [newNear, newFar] = computeCameraNearFar(boundingBox, zoomRange, nearFarRatio);

    cam.setNearFarPlaneDist(newNear, newFar);
}

}  // namespace camerautil

}  // namespace inviwo
