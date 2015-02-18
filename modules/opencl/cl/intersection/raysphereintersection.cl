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

#ifndef RAY_SPHERE_INTERSECTION_CL
#define RAY_SPHERE_INTERSECTION_CL

// Intersects a ray with a sphere.
// If intersecting, t1 is the point of intersection along the ray
bool raySphereIntersection(const float3 sphereCenter, const float radius, const float3 o, const float3 d, float * __restrict t0, float* __restrict t1) {
    
    float3 m = o-sphereCenter;
    float b = dot(m, d);
    float c = dot(m, m)-radius*radius;
    // Exit if ray origin is outside of sphere and pointing away from sphere
    if(c > 0.f && b > 0.f) return false;

    float discr = b*b-c;
    // Negative discriminat means that ray misses sphere
    if(discr < 0.f) return false;

    // Ray intersects sphere, compute first intersection point (smallest t1)
    *t1 = -b - native_sqrt(discr);
    // If t is negative, ray started inside sphere, so we clamp it to zero
    if( *t1 < 0.f ) *t1 = 0.f;

    return true;
}


#endif
