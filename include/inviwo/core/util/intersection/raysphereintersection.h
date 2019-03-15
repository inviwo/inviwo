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

#ifndef IVW_RAYSPHEREINTERSECTION_H
#define IVW_RAYSPHEREINTERSECTION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

/**
 * \brief Intersects a ray with a sphere.
 *
 * @param sphereCenter Center of sphere
 * @param radius Sphere radius
 * @param origin Ray origin
 * @param direction Ray direction
 * @param t0 Parameterized start position along ray
 * @param t1 Parameterized end position along ray
 * @return True if intersecting, otherwise false. result.second will contain the point of
 * intersection along the ray if intersecting.
 */

template <typename T, glm::precision P>
std::pair<bool, float> raySphereIntersection(const glm::tvec3<T, P>& sphereCenter, const T radius,
                                             const glm::tvec3<T, P>& origin,
                                             const glm::tvec3<T, P>& direction, const T t0,
                                             const T t1) {
    glm::tvec3<T, P> m = origin - sphereCenter;
    T b = glm::dot(m, direction);
    T c = glm::dot(m, m) - radius * radius;
    // Exit if ray origin is outside of sphere and pointing away from sphere
    if (c > 0.f && b > 0.f) return {false, T{0.0}};

    T discr = b * b - c;
    // Negative discriminant means that ray misses sphere
    if (discr < 0.0) return {false, T{0.0}};

    // Ray intersects sphere, compute first intersection point (smallest t1)
    T tHit = -b - glm::sqrt(discr);
    // If t is negative, ray started inside sphere, so we clamp it to zero
    if (tHit < 0.f) tHit = T{0.0};
    // Check if intersection was behind start point
    if (tHit >= t0 && tHit <= t1) {
        return {true, tHit};
    } else {
        return {false, T{0.0}};
    }
}

}  // namespace inviwo

#endif  // IVW_RAYSPHEREINTERSECTION_H
