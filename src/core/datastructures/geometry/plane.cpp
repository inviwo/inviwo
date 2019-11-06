/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/plane.h>

#include <glm/gtx/perpendicular.hpp>

namespace inviwo {

inviwo::uvec3 Plane::COLOR_CODE = uvec3(225, 174, 225);
const std::string Plane::CLASS_IDENTIFIER = "org.inviwo.Plane";

Plane::Plane(vec3 point, vec3 normal) noexcept : point_(point), normal_(glm::normalize(normal)) {}

float Plane::distance(const vec3& p) const { return glm::dot(p - point_, normal_); }

vec3 Plane::projectPoint(const vec3& p) const { return p - distance(p) * normal_; }

bool Plane::isInside(const vec3& p) const { return distance(p) >= 0.f; }

bool Plane::perpendicularToPlane(const vec3& p) const {
    return (glm::abs(glm::dot(normal_, p)) < glm::epsilon<float>());
}

mat4 Plane::inPlaneBasis() const {
    std::array<vec3, 3> perp = {glm::perp(vec3{1.0f, 0.0f, 0.0}, normal_),
                                glm::perp(vec3{0.0f, 1.0f, 0.0}, normal_),
                                glm::perp(vec3{0.0f, 0.0f, 1.0}, normal_)};

    const auto a1 = glm::normalize(*std::max_element(
        perp.begin(), perp.end(),
        [](const vec3& a, const vec3& b) { return glm::length2(a) < glm::length2(b); }));
    const auto a2 = glm::cross(normal_, a1);

    return glm::translate(point_) *
           mat4{vec4{a1, 0.0f}, vec4{a2, 0.0f}, vec4{normal_, 0.0f}, vec4{vec3{0.0f}, 1.0}};
}

void Plane::setPoint(const vec3 p) { point_ = p; }

void Plane::setNormal(const vec3& n) { normal_ = glm::normalize(n); }

Plane Plane::transform(const mat4& transform) const {
    const auto newPos = vec3(transform * vec4(point_, 1.0));
    const auto normalTransform = glm::transpose(glm::inverse(transform));
    const auto newNormal = glm::normalize(vec3(normalTransform * vec4(normal_, 0.0)));
    return Plane(newPos, newNormal);
}

std::optional<vec3> Plane::getIntersection(const vec3& start, const vec3& stop) const {
    // Distance from point to plane
    const float d = glm::dot(point_ - start, normal_);

    if (glm::abs(d) < glm::epsilon<float>()) {
        // segment is in plane, return start point.
        return start;
    }

    const vec3 segment = stop - start;
    // Distance of segment projected onto plane normal
    float denom = glm::dot(normal_, segment);  // if zero, segment is parallel to plane
    if (std::abs(denom) > glm::epsilon<float>()) {
        float tHit = d / denom;

        if (tHit >= 0.0f && tHit <= 1.0f) {
            return start + tHit * segment;
        }
    }

    // No intersection
    return std::nullopt;
}

std::optional<float> Plane::getIntersectionWeight(const vec3& start, const vec3& stop) const {
    // Distance from point to plane
    const float d = glm::dot(point_ - start, normal_);

    if (glm::abs(d) < glm::epsilon<float>()) {
        // segment is in plane, return start point.
        return 0.0f;
    }

    const vec3 segment = stop - start;
    // Distance of segment projected onto plane normal
    float denom = glm::dot(normal_, segment);  // if zero, segment is parallel to plane
    if (std::abs(denom) > glm::epsilon<float>()) {
        const float tHit = d / denom;

        if (tHit >= 0.0f && tHit <= 1.0f) {
            return tHit;
        }
    }

    // No intersection
    return std::nullopt;
}

std::string Plane::getDataInfo() const { return "Plane"; }

}  // namespace inviwo