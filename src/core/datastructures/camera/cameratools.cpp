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

vec2 fovyToSize(float fovy, float distance, float aspect) {
    const auto height = 2.0f * distance * std::tan(0.5f * glm::radians(fovy));
    return {height * aspect, height};
}

float widthToFovy(float width, float distance, float aspect) {
    return glm::degrees(2.0f * std::atan(width / aspect / 2.0f / distance));
}

float widthToViewDist(float width, float fov, float aspect) {
    return width / (2.0f * aspect * std::tan(0.5f * glm::radians(fov)));
}

namespace {

constexpr bool overlap(vec2 r1, vec2 r2) { return r1.y >= r2.x && r2.y >= r1.x; };

constexpr std::array<vec3, 8> corners{
    {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}}};

constexpr std::array<std::pair<int, int>, 12> edges{{{0, 1},
                                                     {1, 2},
                                                     {2, 3},
                                                     {3, 1},
                                                     {4, 5},
                                                     {5, 6},
                                                     {6, 7},
                                                     {7, 4},
                                                     {0, 4},
                                                     {1, 5},
                                                     {2, 6},
                                                     {3, 7}}};

}  // namespace

FovBounds calculateFovBounds(const mat4& boundingBox, const vec3& lookFrom, const vec3& lookTo,
                             const vec3& lookUp, float nearPlane, float farPlane) {

    // Camera basis
    const vec3 forward = glm::normalize(lookTo - lookFrom);
    const vec3 right = glm::normalize(glm::cross(forward, lookUp));
    const vec3 up = glm::cross(right, forward);

    std::array<vec3, 8> camPts{};
    std::ranges::transform(corners, camPts.begin(), [&](const vec3& unitCorner) {
        const auto corner = vec3{boundingBox * vec4{unitCorner, 1.0}};
        const auto v = corner - lookFrom;
        return vec3{glm::dot(v, right), glm::dot(v, up), glm::dot(v, forward)};
    });

    std::array<vec3, camPts.size() + 2 * edges.size()> finalPts{};
    size_t finalPtsCount = 0;
    bool nearPlaneClipped = false;
    bool farPlaneClipped = false;
    // Keep points in front of nearPlane
    for (const auto& p : camPts) {
        if (p.z >= nearPlane && p.z <= farPlane) {
            finalPts[finalPtsCount++] = p;
        } else if (p.z < nearPlane) {
            nearPlaneClipped = true;
        } else {
            farPlaneClipped = true;
        }
    }

    // Clip edges against near and far plane
    for (const auto& e : edges) {
        const vec3 a = camPts[e.first];
        const vec3 b = camPts[e.second];

        if (a.z < nearPlane && b.z < nearPlane) continue;
        if (a.z > farPlane && b.z > farPlane) continue;

        const float dz = b.z - a.z;
        if (std::fabs(dz) < 1e-8f) continue;

        if (a.z < nearPlane || b.z < nearPlane) {
            const float t = (nearPlane - a.z) / dz;
            const vec3 ip = {glm::mix(vec2{a}, vec2{b}, t), nearPlane};
            finalPts[finalPtsCount++] = ip;
        }
        if (a.z > farPlane || b.z > farPlane) {
            const float t = (farPlane - a.z) / dz;
            const vec3 ip = {glm::mix(vec2{a}, vec2{b}, t), nearPlane};
            finalPts[finalPtsCount++] = ip;
        }
    }

    if (finalPtsCount == 0) {
        return {.bounds = std::nullopt,
                .nearPlaneClipped = nearPlaneClipped,
                .farPlaneClipped = farPlaneClipped};
    }

    const std::span<const vec3> pts{finalPts.data(), finalPtsCount};

    const auto [xmin, xmax] = std::ranges::minmax(
        pts | std::views::transform([](const vec3& p) { return std::atan2(p.x, p.z); }));

    const auto [ymin, ymax] = std::ranges::minmax(
        pts | std::views::transform([](const vec3& p) { return std::atan2(p.y, p.z); }));

    return {.bounds = std::pair{vec2{xmin, xmax}, vec2{ymin, ymax}},
            .nearPlaneClipped = nearPlaneClipped,
            .farPlaneClipped = farPlaneClipped};
}

bool canZoomBounded(const FovBounds& fovBounds, vec2 fov, float zoomFactor) {

    if (!fovBounds.bounds) {  // we have zoomed in to far
        if (fovBounds.nearPlaneClipped) {
            return zoomFactor < 0.0f;
        } else {
            return zoomFactor > 0.0f;
        }
    }

    const auto [fovxBounds, fovyBounds] = fovBounds.bounds.value();

    if (!overlap(fovxBounds, vec2{-fov.x / 2.0, fov.x / 2.0}) ||
        !overlap(fovyBounds, vec2{-fov.y / 2.0, fov.y / 2.0})) {
        // Bounding box is outside of the field of view
        return zoomFactor < 0.0f;
    }

    const auto fractFovx = (fovxBounds.y - fovxBounds.x) / fov.x;
    const auto fractFovy = (fovyBounds.y - fovyBounds.x) / fov.y;

    if (fractFovx < 0.05 || fractFovy < 0.05) {
        if (fovBounds.nearPlaneClipped) {
            return zoomFactor < 0.0f;
        } else {
            // we don't want to zoom out any further
            return zoomFactor > 0.0f;
        }
    }
    return true;
}

}  // namespace inviwo::util
