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

#include <inviwo/core/datastructures/camera/cameratools.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/util/docbuilder.h>
#include <inviwo/core/util/glm.h>

namespace inviwo::util {
FloatRefProperty* getCameraFovProperty(CameraProperty& comp) {
    return dynamic_cast<FloatRefProperty*>(comp.getCameraProperty("fov"));
}
std::unique_ptr<FloatRefProperty> createCameraFovProperty(std::function<float()> get,
                                                          std::function<void(const float&)> set) {
    return std::make_unique<FloatRefProperty>(
        "fov", "FOV", "The perspective field of view in the vertical direction"_help, get, set,
        std::pair<float, ConstraintBehavior>{0.0f, ConstraintBehavior::Immutable},
        std::pair<float, ConstraintBehavior>{180.0f, ConstraintBehavior::Immutable}, 0.1f);
}

FloatRefProperty* updateOrCreateCameraFovProperty(CameraProperty& comp, std::function<float()> get,
                                                  std::function<void(const float&)> set) {
    auto fov = getCameraFovProperty(comp);
    if (fov) {
        fov->setGetAndSet(get, set);
    } else {
        auto newFov = util::createCameraFovProperty(get, set);
        fov = newFov.get();
        comp.addCamerapProperty(std::move(newFov));
    }
    fov->setVisible(true);
    return fov;
}

FloatRefProperty* getCameraWidthProperty(CameraProperty& comp) {
    return dynamic_cast<FloatRefProperty*>(comp.getCameraProperty("width"));
}

std::unique_ptr<FloatRefProperty> createCameraWidthProperty(std::function<float()> get,
                                                            std::function<void(const float&)> set) {
    return std::make_unique<FloatRefProperty>(
        "width", "Width", "The viewport width in world space"_help, get, set,
        std::pair<float, ConstraintBehavior>{0.0f, ConstraintBehavior::Immutable},
        std::pair<float, ConstraintBehavior>{1000.0f, ConstraintBehavior::Ignore}, 0.1f);
}

FloatRefProperty* updateOrCreateCameraWidthProperty(CameraProperty& comp,
                                                    std::function<float()> get,
                                                    std::function<void(const float&)> set) {
    auto width = getCameraWidthProperty(comp);
    if (width) {
        width->setGetAndSet(get, set);
    } else {
        auto newWidth = createCameraWidthProperty(get, set);
        width = newWidth.get();
        comp.addCamerapProperty(std::move(newWidth));
    }
    width->setVisible(true);
    return width;
}

FloatVec2RefProperty* getCameraEyeOffsetProperty(CameraProperty& comp) {
    return dynamic_cast<FloatVec2RefProperty*>(comp.getCameraProperty("offset"));
}

std::unique_ptr<FloatVec2RefProperty> createCameraEyeOffsetProperty(
    std::function<vec2()> get, std::function<void(const vec2&)> set) {
    return std::make_unique<FloatVec2RefProperty>(
        "offset", "Eye Offset", "Offset from the view direction to the eye in world space"_help,
        get, set, std::pair<vec2, ConstraintBehavior>{vec2(-10.0f), ConstraintBehavior::Ignore},
        std::pair<vec2, ConstraintBehavior>{vec2(10.0f), ConstraintBehavior::Ignore}, vec2(0.01f));
}

FloatVec2RefProperty* updateOrCreateCameraEyeOffsetProperty(CameraProperty& comp,
                                                            std::function<vec2()> get,
                                                            std::function<void(const vec2&)> set) {

    auto offset = getCameraEyeOffsetProperty(comp);
    if (offset) {
        offset->setGetAndSet(get, set);
    } else {
        auto newOffset = createCameraEyeOffsetProperty(get, set);
        offset = newOffset.get();
        comp.addCamerapProperty(std::move(newOffset));
    }
    offset->setVisible(true);
    return offset;
}

float fovyToWidth(float fovy, float distance, float aspect) {
    return 2.0f * distance * std::tan(0.5f * glm::radians(fovy)) * aspect;
}

float widthToFovy(float width, float distance, float aspect) {
    return glm::degrees(2.0f * std::atan(width / aspect / 2.0f / distance));
}

float widthToViewDist(float width, float fov, float aspect) {
    return width / (2.0f * aspect * std::tan(0.5f * glm::radians(fov)));
}

}  // namespace inviwo::util
