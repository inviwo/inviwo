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

#ifndef IVW_PLANE_H
#define IVW_PLANE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

struct IntersectionResult;

class IVW_CORE_API Plane {

public:
    Plane();
    Plane(vec3 point, vec3 normal);
    virtual ~Plane();

    vec3 getPoint() const;
    vec3 getNormal() const;

    /** 
     * \brief Get intersection point with plane between p1 and p2
     * 
     * @param const vec3 & p1 Start point
     * @param const vec3 & p2 End point
     * @return vec3 Intersected point
     */
    vec3 getIntersection(const vec3& p1, const vec3& p2) const;

    IntersectionResult getSegmentIntersection(const vec3& start, const vec3& stop) const;

    vec3 projectPoint(const vec3&) const;

    bool isInside(const vec3&) const;

    bool perpendicularToPlane(const vec3&) const;

    void setPoint(const vec3);
    void setNormal(const vec3&);

private:
    vec3 point_;
    vec3 normal_;

};

struct IntersectionResult {
    IntersectionResult(bool intersects, vec3 intersection);
    IntersectionResult(bool intersects);
    vec3 intersection_;
    bool intersects_;
};


/** 
 * Computes ray-plane intersection point and 
 * returns true if the ray hit the plane.
 * 
 * @param const vec3 & origin Ray origin
 * @param const vec3 & dir Ray direction
 * @param const vec3 & pointInPlane Point on the plane
 * @param const vec3 & planeNormal Normalized plane normal
 * @param float & tHit Distance to hit point along ray direction if hit, otherwise undefined
 * @return bool True if ray hit the plane, false otherwise
 */
IVW_CORE_API bool rayPlaneIntersection(const vec3& origin, const vec3& dir, const vec3& pointInPlane, const vec3& planeNormal, float& tHit);

} // namespace

#endif // IVW_PLANE_H