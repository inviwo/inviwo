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

#ifndef RAY_TRIANGLE_INTERSECTION_CL
#define RAY_TRIANGLE_INTERSECTION_CL


// Intersects a ray with a triangle defined by a vertex and two edges:
// float3 e1 = v1-v0;
// float3 e2 = v2-v0;
// If intersecting, t is the point of intersection along the ray
bool rayTriangleEdgeIntersection(float3 o, float3 dir, float3 v0, float3 e1, float3 e2, float* t) {
    float3 p = cross(dir, e2);
    float a = dot(e1, p);
    if (a > -0.00001f && a < 0.00001f) {
        return false;
    }
    float f = 1.f / a;
    // Distance from v0 to ray origin
    float3 s = o - v0;
    // u parameter
    float u = f * dot(s, p);
    if (u < 0.f || u > 1.f) {
        return false;
    }

    float3 q = cross(s, e1);
    float v = f * dot(dir, q);

    if (v < 0.f || u + v > 1.f) {
        return false;
    }
    // Compute the intersection point t

    *t = f * dot(e2, q);

    if (*t > 0.00001f) {
        return true;
    } else {
        return false;
    }
}
// Intersects a ray with a triangle defined by three vertices.
// If intersecting, t is the point of intersection along the ray
bool rayTriangleIntersection(float3 o, float3 dir, float3 v0, float3 v1, float3 v2, float* t) {
    float3 e1 = v1-v0;
    float3 e2 = v2-v0;
    return rayTriangleEdgeIntersection(o, dir, v0, e1, e2, t);
}



#endif
