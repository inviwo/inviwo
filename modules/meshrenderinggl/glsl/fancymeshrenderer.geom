/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

#ifndef MESH_HAS_ADJACENCY
// normal mode
layout(triangles) in;
#define IDX0 0
#define IDX1 1
#define IDX2 2
#else
// triangle adjacency
layout(triangles_adjacency) in;
#define IDX0 0
#define IDX1 2
#define IDX2 4
#endif

layout(triangle_strip, max_vertices = 3) out;

#include "utils/structs.glsl"

uniform ivec2 halfScreenSize;
uniform GeometryParameters geometry;
uniform CameraParameters camera;

in vData {
    vec4 worldPosition;
    vec4 position;
    vec3 normal;
#ifdef SEND_COLOR
    vec4 color;
#endif
#ifdef SEND_TEX_COORD
    vec2 texCoord;
#endif
#ifdef SEND_SCALAR
    float scalar;
#endif
}
vertices[];

out fData {
    vec4 worldPosition;
    vec4 position;
    vec3 normal;
#ifdef SEND_COLOR
    vec4 color;
#endif
#ifdef SEND_TEX_COORD
    vec2 texCoord;
#endif
#ifdef SEND_SCALAR
    float scalar;
#endif
    float area;
#ifdef ALPHA_SHAPE
    vec3 sideLengths;
#endif
#if defined(DRAW_EDGES) || defined(DRAW_SILHOUETTE)
    vec3 edgeCoordinates;
#endif
#ifdef DRAW_SILHOUETTE
    flat bvec3 silhouettes;
#endif
}
frag;

struct GeometrySettings {
    float edgeWidth;
    bool triangleNormal;
};
uniform GeometrySettings geomSettings;

// Guard against missconfigurations
#if defined(DRAW_SILHOUETTE) && !defined(MESH_HAS_ADJACENCY)
#error "Silhouettes need adjacency information!"
#endif

bool isFront(int a, int b, int c) {

    vec3 A = vertices[a].position.xyz / vertices[a].position.w;
    vec3 B = vertices[b].position.xyz / vertices[b].position.w;
    vec3 C = vertices[c].position.xyz / vertices[c].position.w;
    float area = (A.x * B.y - B.x * A.y) + (B.x * C.y - C.x * B.y) + (C.x * A.y - A.x * C.y);
    return area > 0;
}

void main(void) {
    //==================================================
    // MEASURES ON THE TRIANGLES
    //==================================================
    // compute the area of the triangle,
    // needed for several shading computations
    float area =
        length(cross(vertices[IDX1].worldPosition.xyz - vertices[IDX0].worldPosition.xyz,
                     vertices[IDX2].worldPosition.xyz - vertices[IDX0].worldPosition.xyz)) *
        0.5f;

    // compute side lengths of the triangles
#ifdef ALPHA_SHAPE
    vec3 sideLengths;
    sideLengths.x = length(vertices[IDX1].position.xyz - vertices[IDX0].position.xyz);
    sideLengths.y = length(vertices[IDX2].position.xyz - vertices[IDX0].position.xyz);
    sideLengths.z = length(vertices[IDX2].position.xyz - vertices[IDX1].position.xyz);
#endif

    // compute the per-triangle normal
    vec3 triNormal =
        normalize(cross(vertices[IDX1].worldPosition.xyz - vertices[IDX0].worldPosition.xyz,
                        vertices[IDX2].worldPosition.xyz - vertices[IDX0].worldPosition.xyz));
    triNormal = geometry.dataToWorldNormalMatrix * triNormal;

    //==================================================
    // EDGES
    //==================================================
    // edge coordinates for edge highlighting
#if defined(DRAW_EDGES) || defined(DRAW_SILHOUETTE)
    // vertices coordinates in pixel space
    vec2 screenA = halfScreenSize * vertices[IDX0].position.xy / vertices[IDX0].position.w;
    vec2 screenB = halfScreenSize * vertices[IDX1].position.xy / vertices[IDX1].position.w;
    vec2 screenC = halfScreenSize * vertices[IDX2].position.xy / vertices[IDX2].position.w;
    // side lengths in pixel coordinates
    float ab = length(screenB - screenA);
    float ac = length(screenC - screenA);
    float bc = length(screenC - screenB);
    // cosines angles at the vertices
    float angleACos = dot((screenB - screenA) / ab, (screenC - screenA) / ac);
    float angleBCos = dot((screenA - screenB) / ab, (screenC - screenB) / bc);
    float angleCCos = dot((screenA - screenC) / ac, (screenB - screenC) / bc);
    // sines at the vertices
    float angleASin = sqrt(1 - angleACos * angleACos);
    float angleBSin = sqrt(1 - angleBCos * angleBCos);
    float angleCSin = sqrt(1 - angleCCos * angleCCos);

    // desired edge width in pixels
    float edgeWidthGlobal = geomSettings.edgeWidth;
#ifdef DRAW_EDGES_DEPTH_DEPENDENT
    float edgeWidthScale =
        2;  // experiments, this gives the most similar result to non-depth dependent thickness
    vec3 edgeWidth = vec3(edgeWidthGlobal * edgeWidthScale /
                              length(vertices[IDX0].worldPosition.xyz - camera.position.xyz),
                          edgeWidthGlobal * edgeWidthScale /
                              length(vertices[IDX1].worldPosition.xyz - camera.position.xyz),
                          edgeWidthGlobal * edgeWidthScale /
                              length(vertices[IDX2].worldPosition.xyz - camera.position.xyz));
#else
    vec3 edgeWidth = vec3(edgeWidthGlobal);
#endif
    // compute edge coordinates
    vec3 edgeCoordinates[3];
    edgeCoordinates[0] = vec3(0, 1 / (1 - min(0.99999, edgeWidth.x / (ab * angleASin))),
                              1 / (1 - min(0.99999, edgeWidth.x / (ac * angleASin))));
    edgeCoordinates[1] = vec3(1 / (1 - min(0.99999, edgeWidth.y / (ab * angleBSin))), 0,
                              1 / (1 - min(0.99999, edgeWidth.y / (bc * angleBSin))));
    edgeCoordinates[2] = vec3(1 / (1 - min(0.99999, edgeWidth.z / (ac * angleCSin))),
                              1 / (1 - min(0.99999, edgeWidth.z / (bc * angleCSin))), 0);
#ifdef DRAW_SILHOUETTE
    // additionally compute which edge lies on a boundary
    bool orientation = isFront(0, 2, 4);
    bvec3 silhouettes = bvec3(isFront(2, 3, 4) != orientation, isFront(4, 5, 0) != orientation,
                              isFront(0, 1, 2) != orientation);
#endif
#endif

    //==================================================
    // pass-through other parameters
    //==================================================
    ivec3 vIdx = ivec3(IDX0, IDX1, IDX2);
    for (int j = 0; j < 3; ++j) {
        int i = vIdx[j];
        frag.worldPosition = vertices[i].worldPosition;
        frag.position = vertices[i].position;
        if (geomSettings.triangleNormal) {
            frag.normal = triNormal;
        } else {
            frag.normal = vertices[i].normal;
        }
#ifdef SEND_COLOR
        frag.color = vertices[i].color;
#endif
#ifdef SEND_TEX_COORD
        frag.texCoord = vertices[i].texCoord;
#endif
#ifdef SEND_SCALAR
        frag.scalar = vertices[i].scalar;
#endif
        frag.area = area;
#ifdef ALPHA_SHAPE
        frag.sideLengths = sideLengths;
#endif
#if defined(DRAW_EDGES) || defined(DRAW_SILHOUETTE)
        frag.edgeCoordinates = edgeCoordinates[j];
#endif
#ifdef DRAW_SILHOUETTE
        frag.silhouettes = silhouettes;
#endif
        gl_Position = vertices[i].position;
        EmitVertex();
    }
    EndPrimitive();
}
