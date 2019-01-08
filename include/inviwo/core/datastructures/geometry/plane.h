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
     * \brief Get intersection point with plane and line segment.
     * Intersects if plane is in between start and stop.
     * Segement start point will be returned if segment lies in the plane.
     *
     * Returned intersection point is invalid if no intersection exist.
     * @param start Start point of segment
     * @param stop End point of segment
     * @return Intersected point and true if intersecting.
     */
    IntersectionResult getIntersection(const vec3& start, const vec3& stop) const;

    /**
     * Return signed distance from plane to point, i.e. dot(x - p, normal).
     *
     *      Plane
     *        |
     *        p-> normal
     * x <----|----> x (point)
     *    -d     d
     * @return Negative distance if behind plane, positive otherwise.
     */
    float distance(const vec3& x) const;

    /**
     * Project point onto plane.
     *          Plane
     *            |
     *            |-> normal
     * projection |<---- x (point)
     *
     * @param x Point to project
     * @return point on the plane
     */
    vec3 projectPoint(const vec3& x) const;

    /**
     * Check if point is on positive side of plane.
     * \verbatim
              Plane
                |
                |-> normal
      (outside) |  (inside)
     \endverbatim
     *
     * @see Plane::distance
     * @param point to check
     * @return true if on positive side of normal or on the plane, otherwise false
     */
    bool isInside(const vec3& point) const;

    bool perpendicularToPlane(const vec3&) const;

    void setPoint(const vec3);
    void setNormal(const vec3&);

    std::string getDataInfo() const;

    static uvec3 COLOR_CODE;
    static const std::string CLASS_IDENTIFIER;

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

}  // namespace inviwo

#endif  // IVW_PLANE_H