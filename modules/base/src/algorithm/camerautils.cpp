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

#include <modules/base/algorithm/camerautils.h>

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
            return vec3(0);
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
            return vec3(0);
    }
}

}  // namespace detail

void setCameraView(CameraProperty &cam, const mat4 &dataToWorld, Side side, float scale,
                   bool setNearFar, bool setLookRanges) {
    if (auto perspectiveCamera = dynamic_cast<PerspectiveCamera *>(&cam.get())) {

        auto viewDir = detail::getViewDir(side);
        auto lookUp = detail::getLookUp(side);

        const float offset = .5f;
        auto lookTo = vec3(dataToWorld * vec4(vec3(offset), 1.f));
        auto sideDir = glm::cross(viewDir, lookUp);

        auto frontPoint = vec3(dataToWorld * vec4(vec3(offset) - (viewDir * offset), 1.f));
        auto sidePoint =
            vec3(dataToWorld * vec4(vec3(offset) + (sideDir * offset) - (viewDir * offset), 1.f));
        auto topPoint =
            vec3(dataToWorld * vec4(vec3(offset) + (lookUp * offset) - (viewDir * offset), 1.f));

        float fovy = perspectiveCamera->getFovy() / 2.0f;
        float fovx = glm::degrees(std::atan(cam.getAspectRatio() * std::tan(glm::radians(fovy))));

        float h1 = glm::distance(frontPoint, topPoint) * scale;
        float d1 = h1 * std::tan(glm::radians(90 - fovy));

        float h2 = glm::distance(frontPoint, sidePoint) * scale;
        float d2 = h2 * std::tan(glm::radians(90 - fovx));

        auto lookFrom = frontPoint + std::max(d1, d2) * glm::normalize(frontPoint - lookTo);
        auto camUp = glm::normalize(topPoint - frontPoint);

        NetworkLock lock(&cam);

        if (setNearFar) {
            setCameraNearFar(cam, dataToWorld);
        }
        if (setLookRanges) {
            setCameraLookRanges(cam, dataToWorld);
        }

        cam.setLook(lookFrom, lookTo, camUp);
    } else {
        LogWarnCustom("camerautil::setCameraView",
                      "setCameraView only supports perspective cameras");
    }
}

void setCameraLookRanges(CameraProperty &cam, const mat4 &dataToWorld, float zoomRange) {
    auto m = dataToWorld;

    vec3 lookTo(dataToWorld * vec4(vec3(.5f), 1.f));

    vec3 dx(dataToWorld[0]);
    vec3 dy(dataToWorld[1]);
    vec3 dz(dataToWorld[2]);

    auto p0 = lookTo + (dx + dy + dz) * zoomRange;
    auto p1 = lookTo - (dx + dy + dz) * zoomRange;
    cam.lookFrom_.setMinValue(glm::min(p0, p1));
    cam.lookFrom_.setMaxValue(glm::max(p0, p1));
    p0 = lookTo + (dx + dy + dz);
    p1 = lookTo - (dx + dy + dz);
    cam.lookTo_.setMinValue(glm::min(p0, p1));
    cam.lookTo_.setMaxValue(glm::max(p0, p1));
}

std::pair<float, float> computeCameraNearFar(const mat4 &dataToWorld, float zoomRange,
                                             float nearFarRatio) {
    vec3 bx(dataToWorld[0]);
    vec3 by(dataToWorld[1]);
    vec3 bz(dataToWorld[2]);

    float dx = glm::length(bx);
    float dy = glm::length(by);
    float dz = glm::length(bz);

    auto d = std::max({dx, dy, dz});

    auto newFar = d * (zoomRange + 1);
    auto newNear = newFar * nearFarRatio;
    return {newNear, newFar};
}

void setCameraNearFar(CameraProperty &cam, const mat4 &dataToWorld, float zoomRange,
                      float nearFarRatio) {

    auto [newNear, newFar] = computeCameraNearFar(dataToWorld, zoomRange, nearFarRatio);

    cam.setNearFarPlaneDist(newNear, newFar);
}

FitCameraPropertiesHelper::FitCameraPropertiesHelper(std::string identifier,
                                                     std::string displayName,
                                                     CameraProperty &camera,
                                                     VolumeInport &volumePort)
    : FitCameraPropertiesHelper(identifier, displayName) {
    auto getToWorld = [vp = &volumePort] {
        return vp->getData()->getCoordinateTransformer().getDataToWorldMatrix();
    };
    init(camera, getToWorld, volumePort);
}

FitCameraPropertiesHelper::FitCameraPropertiesHelper(std::string identifier,
                                                     std::string displayName)
    : composite_(identifier, displayName,
                 InvalidationLevel::Valid)  // Indirect invalidation by setting the camera
    , lookAt_("lookAt", "Look At")
    , lookAtSettings_("lookAtSettings", "Settings")
    , flipUp_("flipUp", "Flip Upvector")
    , updateNearFar_("updateNearFar", "Update Near/Far Distances", true)
    , updateLookRanges_("updateLookRanges", "Update Look-to/-from Ranges", true)
    , fittingRatio_("fittingRatio", "Fitting Ratio", 1.05f, 0, 2, 0.01f)
    , xNegative_("xNegative", "X -")
    , xPositive_("xPostive", "X +")
    , yNegative_("yNegative", "Y -")
    , yPositive_("yPostive", "Y +")
    , zNegative_("zNegative", "Z -")
    , zPositive_("zPostive", "Z +")

    , setNearFarButton_("setNearFarButton", "Set Near/Far Distances")
    , setLookRangesButton_("setLookRangesButton", "Set Look-to/-from Ranges") {

    composite_.addProperties(lookAt_, flipUp_, setNearFarButton_, setLookRangesButton_);

    lookAt_.addProperties(xNegative_, xPositive_, yNegative_, yPositive_, zNegative_, zPositive_,
                          lookAtSettings_);
    lookAtSettings_.addProperties(updateNearFar_, updateLookRanges_, fittingRatio_);
    lookAtSettings_.setCollapsed(true);
}

}  // namespace camerautil
                        
}  // namespace inviwo
