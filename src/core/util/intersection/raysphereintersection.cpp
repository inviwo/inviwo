/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
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

#include <inviwo/core/util/intersection/raysphereintersection.h>

namespace inviwo {

IVW_CORE_API std::pair<bool, float> raySphereIntersection(const vec3& sphereCenter,
                                                          const float radius, const vec3& origin,
                                                          const vec3& direction, const float t0,
                                                          const float t1) {
    vec3 m = origin - sphereCenter;
    float b = glm::dot(m, direction);
    float c = glm::dot(m, m) - radius * radius;
    // Exit if ray origin is outside of sphere and pointing away from sphere
    if (c > 0.f && b > 0.f) return {false, 0.0f};

    float discr = b * b - c;
    // Negative discriminant means that ray misses sphere
    if (discr < 0.0f) return {false, 0.0f};

    // Ray intersects sphere, compute first intersection point (smallest t1)
    float tHit = -b - glm::sqrt(discr);
    // If t is negative, ray started inside sphere, so we clamp it to zero
    if (tHit < 0.f) tHit = 0.f;
    // Check if intersection was behind start point
    if (tHit >= t0 && tHit <= t1) {
        return {true, tHit};
    } else {
        return {false, 0.0f};
    }
}

} // namespace

