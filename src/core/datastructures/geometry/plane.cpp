/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

namespace inviwo {

inviwo::uvec3 Plane::COLOR_CODE = uvec3(225, 174, 225);
const std::string Plane::CLASS_IDENTIFIER = "org.inviwo.Plane";

Plane::Plane() : point_(vec3(0.0f, 0.0f, 0.0f)), normal_(vec3(0.0f, 1.0f, 0.0f)) {}

Plane::Plane(vec3 point, vec3 normal) : point_(point), normal_(glm::normalize(normal)) {}

Plane::~Plane() {}

vec3 Plane::getPoint() const { return point_; }

vec3 Plane::getNormal() const { return normal_; }

float Plane::distance(const vec3& p) const { return glm::dot(p - point_, normal_); }

vec3 Plane::projectPoint(const vec3& p) const { return p - distance(p) * normal_; }

bool Plane::isInside(const vec3& p) const { return (distance(p) >= 0.f) ? true : false; }

bool Plane::perpendicularToPlane(const vec3& p) const {
    return (glm::abs(glm::dot(normal_, p)) < glm::epsilon<float>());
}

void Plane::setPoint(const vec3 p) { this->point_ = p; }

void Plane::setNormal(const vec3& n) { this->normal_ = n; }

IntersectionResult Plane::getIntersection(const vec3& start, const vec3& stop) const {
    // Distance from point to plane
    float d = glm::dot(point_ - start, normal_);

    if (glm::abs(d) < glm::epsilon<float>()) {
        // segment is in plane, return start point.
        return IntersectionResult(true, start);
    }

    vec3 segment = stop - start;
    // Distance of segment projected onto plane normal
    float denom = glm::dot(normal_, segment);  // if zero, segment is parallel to plane
    if (std::abs(denom) > glm::epsilon<float>()) {
        float tHit = d / denom;

        if (tHit >= 0.0f && tHit <= 1.0f) {
            return IntersectionResult(true, start + tHit * segment);
        }
    }

    // No intersection
    return IntersectionResult(false);
}

std::string Plane::getDataInfo() const { return "Plane"; }

IntersectionResult::IntersectionResult(bool intersects, vec3 intersection)
    : intersection_(intersection), intersects_(intersects) {}

IntersectionResult::IntersectionResult(bool intersects)
    : intersection_(0.0f), intersects_(intersects) {}

}  // namespace inviwo