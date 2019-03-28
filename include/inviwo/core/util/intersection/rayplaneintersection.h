/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_RAYPLANEINTERSECTION_H
#define IVW_RAYPLANEINTERSECTION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

/**
 * \brief Intersection test of a ray with a plane.
 *
 * @param planePos Point in the plane
 * @param planeNormal Plane normal
 * @param origin Ray origin
 * @param direction Ray direction
 * @param t0 Parameterized start position along ray
 * @param t1 Parameterized end position along ray
 * @return True if intersecting, otherwise false. result.second will contain the point of
 * intersection along the ray if intersecting.
 */
template <typename T, glm::precision P>
std::pair<bool, T> rayPlaneIntersection(const glm::tvec3<T, P>& planePos,
                                        const glm::tvec3<T, P>& planeNormal,
                                        const glm::tvec3<T, P>& origin,
                                        const glm::tvec3<T, P>& direction, const T t0, const T t1) {
    // http://en.wikipedia.org/wiki/Line-plane_intersection
    // http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-plane-and-ray-disk-intersection/
    T denom = glm::dot(planeNormal, direction);
    // If denominator == 0, then segment is parallel to plane
    // Otherwise, it points to or away from the plane
    if (std::abs(denom) > std::numeric_limits<T>::epsilon()) {
        T numerator = glm::dot(planePos - origin, planeNormal);

        T tHit = numerator / denom;
        // Check if within ray bounds
        if (tHit >= t0 && tHit <= t1) {
            return {true, tHit};
        }
    }
    return {false, T{0.0}};
}

}  // namespace inviwo

#endif  // IVW_RAYPLANEINTERSECTION_H
