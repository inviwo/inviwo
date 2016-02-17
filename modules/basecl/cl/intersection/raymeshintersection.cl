/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef RAY_MESH_INTERSECTION_CL
#define RAY_MESH_INTERSECTION_CL

#include "intersection/raytriangleintersection.cl"

/** 
 * Test intersection between a ray and an indexed triangle mesh.
 * If intersecting, t0 will contain the first point of intersection along the ray and t1 the last.
 * If ray is inside the box, t0 will be set to 0.
 * @note Assumes that at least one triangle is present.
 * @param vertices  xyz position 
 * @param indices   Triangle indices into vertex list.
 * @param nIndices  Number of triangle indices. 
 * @param rayOrigin Ray origin
 * @param rayOrigin Ray direction (not normalized)
 * @param t0        Start point along ray
 * @param t1        End point along ray
 * @return true if an intersection is found, false otherwise.
 */ 
bool rayMeshIntersection(

#ifdef RAY_MESH_LOCAL_MEM
    __global float const * __restrict verticesG
    , __global int const * __restrict indicesG
#else
    __global float const * __restrict vertices
    , __global int const * __restrict indices
#endif
    , int nIndices
    , float3 rayOrigin
    , float3 rayDirection
    , float* __restrict  t0, float* __restrict  t1)
{
    float tNear = FLT_MAX; float tFar = 0.f;
#ifdef RAY_MESH_LOCAL_MEM
    // A simple test to see what would be the 
    // fastest possible using local memory.
    // Only works with a cube mesh.
    __local float vertices[3 * 8];
    __local int indices[3 * 14];
    int localId = get_local_id(0);
    uint currIndex = localId;
    uint triangleCount = (nIndices - 2);
    if (localId < 14 * 3) {
        *(indices + localId) = *(indicesG + localId);
    }
    if (localId < 8 * 3) {
        *(vertices + localId) = *(verticesG + localId);
    }
    barrier(CLK_LOCAL_MEM_FENCE);
#endif
    // Assume that at least one triangle is present
    int3 triangle = 3 * (int3)(indices[0], indices[1], indices[2]);
    // Triangle strip
    float3 v0 = (float3)(vertices[triangle.x], vertices[triangle.x + 1], vertices[triangle.x + 2]);
    float3 v1 = (float3)(vertices[triangle.y], vertices[triangle.y + 1], vertices[triangle.y + 2]);
    float3 v2 = (float3)(vertices[triangle.z], vertices[triangle.z + 1], vertices[triangle.z + 2]);
    float t;
    bool iSect = rayTriangleIntersection(rayOrigin, rayDirection, v0, v1, v2, &t);
    if (iSect) {
        tNear = fmin(t, tNear);
        tFar = fmax(t, tFar);
    }
    for (int i = 3; i < nIndices; ++i) {
        // Fetch new vertex in triangle strip
        int newTriangleStripId = 3 * indices[i];
        // Swap location of the vertices
        v0 = v1; v1 = v2;
        v2 = (float3)(vertices[newTriangleStripId], vertices[newTriangleStripId + 1], vertices[newTriangleStripId + 2]);

        iSect = rayTriangleIntersection(rayOrigin, rayDirection, v0, v1, v2, &t);
        if (iSect) {
            tNear = fmin(t, tNear);
            tFar = fmax(t, tFar);
        }
    }

    if (tFar != 0.f) {
        // We are inside the geometry if the
        // closest hit point is equal
        // to the farthest
        if (tNear == tFar) {
            tNear = 0.f;
        }
        *t0 = fmax(*t0, tNear);
        *t1 = fmin(*t1, tFar);

        return (*t0 < *t1);
    } else {
        return false;
    } 
}


#endif
