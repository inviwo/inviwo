/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

Plane::Plane() :
    point_(vec3(0.0f,0.0f,0.0f)),
    normal_(vec3(0.0f,1.0f,0.0f)) {
}

Plane::Plane(vec3 point, vec3 normal) :
    point_(point),
    normal_(glm::normalize(normal)) {
}

Plane::~Plane() {
}

vec3 Plane::getPoint() const {
    return point_;
}

vec3 Plane::getNormal() const {
    return normal_;
}

vec3 Plane::getIntersection(const vec3& p1, const vec3& p2) const {
    ivwAssert(!(glm::abs(glm::dot(p2-p1,normal_)) < 0.0001f), "Line parallel with clip plane.");
    vec3 l = p2 - p1;
    float nom = glm::dot(point_ - p1, normal_);
    float denom = glm::dot(l, normal_);
    float dist = nom/denom;
    float roundDist = static_cast<float>(static_cast<int>(dist*1000000 + (dist<0.f ? -0.5f : 0.5f)))/1000000.f;
    vec3 res = vec3(roundDist*l + p1);
    return res;
}

vec3 Plane::projectPoint(const vec3& p1) const {
    float dist = glm::dot(p1 - point_, normal_);
    return p1 - dist*normal_;
}

bool Plane::isInside(const vec3& p) const {
    return (glm::dot(normal_,p-point_) > 0.f) ? true : false;
}

bool Plane::perpendicularToPlane(const vec3& p) const {
    return (glm::abs(glm::dot(normal_, p)) < 0.0001f);
}

void Plane::setPoint(const vec3 p) {
    this->point_ = p;
}

void Plane::setNormal(const vec3& n) {
    this->normal_ = n;
}

IntersectionResult Plane::getSegmentIntersection(const vec3& start, const vec3& stop) const {
    float numerator = glm::dot(point_ - start, normal_);
    
    // If line is in plane return start point.
    if (glm::abs(numerator) < 1e-6) {
        return IntersectionResult(true, start);
    }
    
    // Line not in plane 
    vec3 d = stop - start;
    float denom = glm::dot(normal_, d);
    if (fabs(denom) > 1e-6) {
        float numerator = glm::dot(point_ - start, normal_);

        float tHit = numerator / denom;

        if (tHit >= 0.0f && tHit <= 1.0f) {
            return IntersectionResult(true, start + tHit * d);
        }
    }  

    // no intersection
    return IntersectionResult(false);
}

IVW_CORE_API bool rayPlaneIntersection(const vec3& origin, const vec3& dir, const vec3& pointInPlane, const vec3& planeNormal, float& tHit)
{
    // Ray-plane intersection ( http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-plane-and-ray-disk-intersection/ )
    float denom = glm::dot(planeNormal, dir);
    if (denom > 1e-6) {
        tHit = glm::dot(pointInPlane-origin, planeNormal) / denom;
        return tHit >= 0.f;
    }
    return false;
}

IntersectionResult::IntersectionResult(bool intersects, vec3 intersection)
    : intersection_(intersection), intersects_(intersects) {}

IntersectionResult::IntersectionResult(bool intersects)
    : intersection_(0.0f), intersects_(intersects) {}

} // namespace