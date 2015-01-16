/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef RAY_PLANE_INTERSECTION_CL
#define RAY_PLANE_INTERSECTION_CL

float scalarTripleProduct(float3 u, float3 v, float3 w) {
    return dot(cross(u, v), w);
}

// Intersection test of a ray with a plane. 
// If intersecting, t1 is the point of intersection along the ray
bool rayPlaneIntersection(const float3 planePos, const float3 planeNormal, const float3 o, const float3 d, float * __restrict t0, float* __restrict t1) { 
    // http://en.wikipedia.org/wiki/Line-plane_intersection
    // http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-plane-and-ray-disk-intersection/
    float denom = dot(planeNormal, d);
    // If denominator == 0, then segment is parallel to plane
    // Otherwise, it points to or away from the plane
    if( fabs(denom) > 1e-6) {
        float numerator = dot((planePos - o), planeNormal);

        float tHit =  numerator / denom;
        // Check if within ray bounds
        if (tHit >= *t0 && tHit <= *t1) {
            *t1 = tHit;
            return true;
        }
    } 
    return false;  
}

// Intersects a ray with a Quad.
// If intersecting, t1 is the point of intersection along the ray
// D-C
// | |
// A-B
bool rayQuadIntersection(const float3 A, const float3 B, const float3 C, const float3 D, const float3 o, const float3 d, float * __restrict t0, float* __restrict t1) { 
    // Test if intersection is within bounds
    float3 oa = A - o;
    float3 ob = B - o;
    float3 oc = C - o;

    // Determine which triangle to test against by testing diagonal first
    float3 m = cross(oc, d);
    float v = dot(oa, m);
    float3 hitPoint;
    if( v >= 0.f ) {
        // Test intersection against triangle abc
        float u = -dot(ob, m);
        if(u < 0.f) return false;

        float w = scalarTripleProduct(d, ob, oa);
        if( w < 0.f ) return false;

        // Compute intersection point
        float denom = 1.f /(u+v+w);
        u *= denom; v *= denom; w *= denom;
        hitPoint = u*A+v*B+w*C;
    } else {
        float3 od = D - o;
        float u = dot(od, m);
        if(u < 0.f) return false;

        float w = scalarTripleProduct(d, oa, od);
        if( w < 0.f) return false;

        v = -v;
        // Compute intersection point
        float denom = 1.f /(u+v+w);
        u *= denom; v *= denom; w *= denom;
        hitPoint = u*A+v*D+w*C;
        
    }
    float3 hitDir = hitPoint-o;
    if (dot(hitDir, d) >= 0) {
        float t = length(hitDir);
        if (t >= *t0 && t <= *t1) {
            *t1 = t; 
            return true;
        } 
    }
    return false;

}

#endif
