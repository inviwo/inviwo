/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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

#include <inviwo/core/util/glm.h>

#include <algorithm>
#include <glm/gtx/perpendicular.hpp>
#include <array>

namespace inviwo {


Plane::Plane(dvec3 point, dvec3 normal) noexcept : point_(point), normal_(glm::normalize(normal)) {}

double Plane::distance(const dvec3& p) const { return glm::dot(p - point_, normal_); }

dvec3 Plane::projectPoint(const dvec3& p) const { return p - distance(p) * normal_; }

bool Plane::isInside(const dvec3& p) const { return distance(p) >= 0.f; }

bool Plane::perpendicularToPlane(const dvec3& p) const {
    return (glm::abs(glm::dot(normal_, p)) < glm::epsilon<double>());
}

dmat4 Plane::inPlaneBasis() const {
    std::array<dvec3, 3> perp = {glm::perp(dvec3{1.0f, 0.0f, 0.0}, normal_),
                                 glm::perp(dvec3{0.0f, 1.0f, 0.0}, normal_),
                                 glm::perp(dvec3{0.0f, 0.0f, 1.0}, normal_)};

    const auto a1 = glm::normalize(*std::ranges::max_element(
        perp, [](const dvec3& a, const dvec3& b) { return glm::length2(a) < glm::length2(b); }));
    const auto a2 = glm::cross(normal_, a1);

    return glm::translate(point_) *
           dmat4{dvec4{a1, 0.0f}, dvec4{a2, 0.0f}, dvec4{normal_, 0.0f}, dvec4{dvec3{0.0f}, 1.0}};
}

void Plane::setPoint(const dvec3 p) { point_ = p; }

void Plane::setNormal(const dvec3& n) { normal_ = glm::normalize(n); }

Plane Plane::transform(const dmat4& transform) const {
    const auto newPos = util::transformPos(transform, point_);
    const auto newNormal = glm::normalize(util::transformNormal(transform, normal_));
    return {newPos, newNormal};
}

std::optional<dvec3> Plane::getIntersection(const dvec3& start, const dvec3& stop) const {
    // Distance from point to plane
    const double d = glm::dot(point_ - start, normal_);

    if (glm::abs(d) < glm::epsilon<double>()) {
        // segment is in plane, return start point.
        return start;
    }

    const dvec3 segment = stop - start;
    // Distance of segment projected onto plane normal
    const double denom = glm::dot(normal_, segment);  // if zero, segment is parallel to plane
    if (std::abs(denom) > glm::epsilon<double>()) {
        const double tHit = d / denom;

        if (tHit >= 0.0f && tHit <= 1.0f) {
            return start + tHit * segment;
        }
    }

    // No intersection
    return std::nullopt;
}

std::optional<double> Plane::getIntersectionWeight(const dvec3& start, const dvec3& stop) const {
    // Distance from point to plane
    const double d = glm::dot(point_ - start, normal_);

    if (glm::abs(d) < glm::epsilon<double>()) {
        // segment is in plane, return start point.
        return 0.0f;
    }

    const dvec3 segment = stop - start;
    // Distance of segment projected onto plane normal
    const double denom = glm::dot(normal_, segment);  // if zero, segment is parallel to plane
    if (std::abs(denom) > glm::epsilon<double>()) {
        const double tHit = d / denom;

        if (tHit >= 0.0f && tHit <= 1.0f) {
            return tHit;
        }
    }

    // No intersection
    return std::nullopt;
}

std::string_view Plane::getDataInfo() { return "Plane"; }

}  // namespace inviwo
