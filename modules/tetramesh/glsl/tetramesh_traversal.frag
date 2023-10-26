/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include "utils/structs.glsl"
#include "utils/shading.glsl"
#include "utils/pickingutils.glsl"
#include "utils/compositing.glsl"
#include "utils/classification.glsl"

#if !defined(REF_SAMPLING_INTERVAL)
#define REF_SAMPLING_INTERVAL 150.0
#endif

#define ERT_THRESHOLD 0.99  // threshold for early ray termination

/**
 * Data structures for tetrahedra indexing and face enumeration based on
 *    M. Lage, T. Lewiner, H. Lopes, and L. Velho.
 *    CHF: A scalable topological data structure for tetrahedral meshes.
 *    In Brazilian Symposium on Computer Graphics and Image Processing
 *    (SIBGRAPI'05), pp. 349-356, 2005, doi: 10.1109/SIBGRAPI.2005.18
 */

uniform GeometryParameters geometry;
uniform CameraParameters camera;
uniform LightParameters lighting;

uniform sampler2D transferFunction;
uniform int shaderOutput = 0;
uniform int maxSteps = 20;

// scalar scaling and offset are used to map scalar values from [min,max] to [0,1]
uniform float tfScalarScaling = 1.0;
uniform float tfScalarOffset = 0.0;

uniform float opacityScaling = 1.0;

uniform ImageParameters backgroundParameters;
uniform sampler2D backgroundColor;
uniform bool useBackground = false;

struct VertexPosition {
    vec3 pos;
    float scalar;
};

layout(std430, binding=0) readonly buffer nodeBuffer {
    VertexPosition vertexPositions[];
};
layout(std430, binding=1) readonly buffer nodeIdsBuffer {
    ivec4 vertexIds[];
};
layout(std430, binding=2) readonly buffer opposingFaceIdsBuffer {
    ivec4 faceIds[];
};

in Fragment {
    smooth vec4 worldPosition;
    smooth vec3 position;
    flat vec4 color;
    flat int tetraFaceId;

    flat vec3 camPos;
} in_frag;


const ivec3 triIndices[4] = ivec3[4](ivec3(1, 2, 3), ivec3(2, 0, 3), 
                                     ivec3(3, 0, 1), ivec3(0, 2, 1));

struct Tetra {
    mat4x3 v; // vertices
    vec4 s; // scalar values

    mat4x3 fA; // oriented face areas (in negative normal direction) as used in barycentricWeights(), 
               // their magnitude is equivalent to two times the face area.
    float jacobyDetInv; // 1 over determinant of the Jacobian, where det(Jacobian) = 6 vol(tetra)
};

mat4x3 getFaceAreas(in Tetra t);

Tetra getTetra(in int tetraId) {
    ivec4 vertices = vertexIds[tetraId];

    VertexPosition[4] p = VertexPosition[](vertexPositions[vertices[0]],
                                           vertexPositions[vertices[1]],
                                           vertexPositions[vertices[2]],
                                           vertexPositions[vertices[3]]);

    Tetra t;
    t.v = mat4x3(p[0].pos, p[1].pos, p[2].pos, p[3].pos);
    t.s = vec4(p[0].scalar, p[1].scalar, p[2].scalar, p[3].scalar);

    t.fA = getFaceAreas(t);

    // the determinant of the Jacobian of the tetrahedra is det = 6 V, where V is its volume
    t.jacobyDetInv = 1.0 / dot(cross(t.v[2] - t.v[0], t.v[3] - t.v[2]), t.v[1] - t.v[0]);

    return t;
}

// Compute the oriented face areas (in negative normal direction) as used in barycentric 
// interpolation. Their magnitude is equivalent to two times the face area.
//
// @param t   input tetraehdron
// @return oriented face areas fA
mat4x3 getFaceAreas(in Tetra t) {
    const vec3 v_01 = t.v[1] - t.v[0];
    const vec3 v_02 = t.v[2] - t.v[0];
    const vec3 v_03 = t.v[3] - t.v[0];
    const vec3 v_12 = t.v[2] - t.v[1];
    const vec3 v_13 = t.v[3] - t.v[1];

    return mat4x3(cross(v_13, v_12),
                  cross(v_02, v_03),
                  cross(v_03, v_01),
                  cross(v_01, v_02));
}

// Compute the face normals for tetrahedron \p t
//
// @param t   input tetraehdron with oriented face areas (in negative normal direction)
// @return face normals, that is normalized(fA[0]), ..., normalized(fA[3])
mat4x3 getFaceNormals(in Tetra t) {
    return mat4x3(-normalize(t.fA[0]), -normalize(t.fA[1]), -normalize(t.fA[2]), -normalize(t.fA[3]));
}

// Interpolate scalars of tetrahedron \p tetra using barycentric coordinates for position \p p within
//
// @param p      position of the barycentric coords
// @param tetra  input tetrahedron
// @return interpolated scalar value
// 
// see https://www.iue.tuwien.ac.at/phd/nentchev/node30.html
// and https://www.iue.tuwien.ac.at/phd/nentchev/node31.html
float barycentricInterpolation(in vec3 p, in Tetra tetra) {
    const vec3 v_0p = p - tetra.v[0];
    const vec3 v_1p = p - tetra.v[1];

    // barycentric volumes, correct volumes obtained by scaling with 1/6
    float vol0 = dot(tetra.fA[0], v_1p);
    float vol1 = dot(tetra.fA[1], v_0p);
    float vol2 = dot(tetra.fA[2], v_0p);
    float vol3 = dot(tetra.fA[3], v_0p);

    return dot(vec4(vol0, vol1, vol2, vol3) * tetra.jacobyDetInv, tetra.s);
}

// Determine barycentric gradients at each vertex of \p tetra
//
// @param tetra  input tetrahedron with oriented face areas
// @return barycentric gradients (direction matches face normals)
mat4x3 getBarycentricGradients(in Tetra tetra) {
    return tetra.fA * tetra.jacobyDetInv;    
}

// Compute the constant gradient within tetrahedron \p tetra
//
// @param tetra  input tetrahedron withoriented face areas
// @return gradient of \p tetra
vec3 getTetraGradient(in Tetra tetra) {
    // accumulate the barycentric gradients (fA / det(Jacobian)) weighted by the 
    // corresponding scalar values
    return -normalize(tetra.fA * tetra.jacobyDetInv * tetra.s);
}



float absorption(in float opacity, in float tIncr) {
    return 1.0 - pow(1.0 - opacity, tIncr * REF_SAMPLING_INTERVAL);
}

float normalizeScalar(float scalar) {
    return (scalar + tfScalarOffset) * tfScalarScaling;
}

vec4 sampleTF(float normalizedScalar) {
    return texture(transferFunction, vec2(normalizedScalar, 0.5));
}

void main() {

    vec3 rayDirection = normalize(in_frag.position - in_frag.camPos);

    int tetraFaceId = in_frag.tetraFaceId;
    vec3 pos = in_frag.position;

    int tetraId = tetraFaceId / 4;
    int localFaceId = tetraFaceId % 4;
    ivec4 vertices = vertexIds[tetraId];

    // determine scalar value at entry position
    Tetra tetra = getTetra(tetraId);
    float prevScalar = normalizeScalar(barycentricInterpolation(pos, tetra));

    vec4 dvrColor = vec4(0);
    int steps = 0;
    while (tetraFaceId > -1 && steps < maxSteps && dvrColor.a < ERT_THRESHOLD) {
        // find next tetra
        tetraId = tetraFaceId / 4;
        localFaceId = tetraFaceId % 4;
        vertices = vertexIds[tetraId];

        // query data of current tetrahedron
        tetra = getTetra(tetraId);
        mat4x3 faceNormal = getFaceNormals(tetra);

        // intersect ray at current position with all tetra faces
        vec4 vdir = vec4(dot(faceNormal[0], rayDirection),
                         dot(faceNormal[1], rayDirection),
                         dot(faceNormal[2], rayDirection),
                         dot(faceNormal[3], rayDirection));
        vec4 vt = vec4(dot(tetra.v[1] - pos, faceNormal[0]),
                       dot(tetra.v[2] - pos, faceNormal[1]),
                       dot(tetra.v[3] - pos, faceNormal[2]),
                       dot(tetra.v[0] - pos, faceNormal[3])) / vdir;

        const float invalidDist = 1.0e6;
        // only consider intersections on the inside of the current triangle faces, that is t > 0.
        // Also ignore intersections being parallel to a face
        vt = mix(vt, vec4(invalidDist), lessThan(vdir, vec4(0.0)));

        // ignore self-intersection with current face ID, set distance to max
        vt[localFaceId] = invalidDist;

        // closest intersection
        // face ID of closest intersection
        const int vface1 = vt.x < vt.y ? 0 : 1;
        const int vface2 = vt.z < vt.w ? 2 : 3;
        const int vface = vt[vface1] < vt[vface2] ? vface1 : vface2;        
        const float tmin = vt[vface];

        vec3 endPos = pos + rayDirection * tmin;

        float scalar = normalizeScalar(barycentricInterpolation(endPos, tetra));
        vec3 gradient = getTetraGradient(tetra);
        
        const int numSteps = 100;
        float tDelta = tmin / float(numSteps);
        for (int i = 1; i <= numSteps; ++i) {
            float s = mix(prevScalar, scalar, i / float(numSteps));

            vec4 color = sampleTF(s);
#if defined(SHADING_ENABLED)
            color.rgb = APPLY_LIGHTING(lighting, color.rgb, color.rgb, vec3(1.0),
                                       pos + rayDirection * tDelta * i,
                                       gradient, -rayDirection);
#endif

            // volume integration along current segment
            color.a = absorption(color.a, tDelta * opacityScaling * 0.1);
            // front-to-back blending
            color.rgb *= color.a;
            dvrColor += (1.0 - dvrColor.a) * color;
        }

        prevScalar = scalar;

        // update position
        pos = endPos;

        // determine the half face opposing the half face with the found intersection
        tetraFaceId = faceIds[tetraId][vface];
        ++steps;
    }

    if (useBackground) {
        // blend result with background
        vec4 background = texture(backgroundColor, gl_FragCoord.xy * backgroundParameters.reciprocalDimensions); 
        background.rgb *= background.a;
        dvrColor += (1.0 - dvrColor.a) * background;
    }

    FragData0 = dvrColor;
}
