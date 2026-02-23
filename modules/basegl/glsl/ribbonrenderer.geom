/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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
#include "utils/trafo.glsl"

#if !defined(SUBDIVISIONS)
#define SUBDIVISIONS 0
#endif
#if !defined(MAX_VERTICES_OUT)  // should be equal to 4 + SUBDIVISIONS * 2
#define MAX_VERTICES_OUT 4
#endif

#if defined(HAS_ADJACENCY)
layout(lines_adjacency) in;
#else
layout(lines) in;
#endif

layout(triangle_strip, max_vertices = MAX_VERTICES_OUT) out;

uniform GeometryParameters geometry;
uniform CameraParameters camera;

in LineVert {
    vec4 color;
    vec3 binormal;
    float width;
    flat uint pickID;
} inRibbon[];

out RibbonGeom {
    vec3 worldPosition;
    vec4 color;
    vec3 normal;
    flat uint pickID;
} outRibbon;

struct Vertex {
    vec4 pos;
    vec4 color;
    vec3 normal;
    vec3 binormal;
    vec2 texCoord;
};

Vertex createVertex(in vec3 pos, in vec4 color, in vec3 normal, in vec3 binormal,
                    in vec2 texCoord) {
    return Vertex(vec4(pos, 1), color, normal, binormal, texCoord);
}

void emit(in Vertex vertex, in uint pickID) {
    gl_Position = camera.worldToClip * vertex.pos;
    outRibbon.worldPosition = vertex.pos.xyz;
    outRibbon.color = vertex.color;
    outRibbon.normal = vertex.normal;
    outRibbon.pickID = pickID;
    EmitVertex();
}

// Determine the enclosed angle between vectors a and b so that a is rotated on top of b around
// the orthogonal axis, i.e. cross(a, b).
float enclosedAngle(in vec3 a, in vec3 b, in vec3 normal) {
    float angle = acos(clamp(dot(a, b), -1, 1));
    return angle * sign(-dot(b, normal));
}

float tiltAngle(in float angle, in float r, in float height) {
    if (height < 0.0001) return 0.0;
    return atan(angle * r / height);
}

struct RibbonSegment {
    vec3 tangent;
    vec3 binormal[2];
    vec3 normal[2];
    float width[2];
    float tiltAngle[2];
    float dist;
    float rotationAngle;
};

float getTiltAngle(in int index1, in int index2, in int offset) {
    vec3 tangent = vec3(gl_in[index2].gl_Position - gl_in[index1].gl_Position);
    float dist = length(tangent);
    tangent = normalize(tangent);

    vec3 binormal[2] = vec3[2](normalize(inRibbon[index1].binormal),
                               normalize(inRibbon[index2].binormal));
    vec3 normal[2] = vec3[2](normalize(cross(binormal[0], tangent)),
                             normalize(cross(binormal[1], tangent)));
    binormal[0] = normalize(cross(tangent, normal[0]));
    binormal[1] = normalize(cross(tangent, normal[1]));

    float rotationAngle = enclosedAngle(binormal[0], binormal[1], normal[0]);
    float width = (offset == 0 ? inRibbon[index1].width : inRibbon[index2].width);

    return tiltAngle(rotationAngle, width, dist);
}

RibbonSegment createSegment(in int index1, in int index2) {
    RibbonSegment segment;

    vec3 tangent = vec3(gl_in[index2].gl_Position - gl_in[index1].gl_Position);
    segment.dist = length(tangent);
    segment.tangent = normalize(tangent);

    segment.binormal = vec3[2](normalize(inRibbon[index1].binormal),
                               normalize(inRibbon[index2].binormal));

    segment.normal = vec3[2](normalize(cross(segment.binormal[0], segment.tangent)),
                             normalize(cross(segment.binormal[1], segment.tangent)));
    segment.binormal[0] = normalize(cross(segment.tangent, segment.normal[0]));
    segment.binormal[1] = normalize(cross(segment.tangent, segment.normal[1]));

    segment.width = float[2](inRibbon[index1].width, inRibbon[index2].width);
    segment.rotationAngle = enclosedAngle(segment.binormal[0], segment.binormal[1], segment.normal[0]);

    segment.tiltAngle = float[2](tiltAngle(segment.rotationAngle, segment.width[0], segment.dist),
                                 tiltAngle(segment.rotationAngle, segment.width[1], segment.dist));
    return segment;
}

void main() {
#if defined(HAS_ADJACENCY)
    // Get the four vertices passed to the shader
    const int index1 = 1;
    const int index2 = 2;

    vec4 p0in = gl_in[0].gl_Position;
    vec4 p1in = gl_in[1].gl_Position;
    vec4 p2in = gl_in[2].gl_Position;
    vec4 p3in = gl_in[3].gl_Position;
#else
    // no adjacency information available, duplicate first and second vertex
    const int index1 = 0;
    const int index2 = 1;

    vec4 p0in = gl_in[0].gl_Position;
    vec4 p1in = gl_in[0].gl_Position;
    vec4 p2in = gl_in[1].gl_Position;
    vec4 p3in = gl_in[1].gl_Position;
#endif

    vec4 color1 = inRibbon[index1].color;
    vec4 color2 = inRibbon[index2].color;

    RibbonSegment segment = createSegment(index1, index2);

#if defined(HAS_ADJACENCY)
    float previousTiltAngle = segment.tiltAngle[0];
    if (p1in != p0in) {
        previousTiltAngle = getTiltAngle(0, 1, 1);
    }
    float nextTiltAngle = segment.tiltAngle[1];
    if (p2in != p3in) {
        nextTiltAngle = getTiltAngle(2, 3, 0);
    }
    segment.tiltAngle[0] = mix(previousTiltAngle, segment.tiltAngle[0], 0.5);
    segment.tiltAngle[1] = mix(segment.tiltAngle[1], nextTiltAngle, 0.5);
#endif

    // TODO: consider tail and head pivot

#if SUBDIVISIONS > 0
    float delta = 1.0 / float(SUBDIVISIONS + 1);
    for (int i = 0; i <= SUBDIVISIONS + 1; ++i) {
        float t = float(i) * delta;

        vec4 color = mix(color1, color2, t);
        float angle = mix(0, segment.rotationAngle, t);

        vec3 binormal = mat3(rotate(segment.tangent, angle)) * segment.binormal[0];
        vec3 normal = cross(binormal, segment.tangent);

        float width = mix(segment.width[0], segment.width[1], t);

        float tiltAngle = tiltAngle(segment.rotationAngle, width, segment.dist);

        vec3 leftNormal = mat3(rotate(binormal, tiltAngle)) * normal;
        vec3 rightNormal = mat3(rotate(binormal, -tiltAngle)) * normal;

        vec3 center = mix(p1in.xyz, p2in.xyz, t);
        vec3 left = center - binormal * width;
        vec3 right = center + binormal * width;

        emit(createVertex(left, color, leftNormal, binormal, vec2(0, t)), inRibbon[index1].pickID);
        emit(createVertex(right, color, rightNormal, binormal, vec2(1, t)), inRibbon[index1].pickID);
    }
#else
    vec3 offset1 = segment.binormal[0] * segment.width[0];
    vec3 offset2 = segment.binormal[1] * segment.width[1];

    emit(createVertex(p1in.xyz - offset1, color1, segment.normal[0], segment.binormal[0], vec2(0, 0)),
         inRibbon[index1].pickID);
    emit(createVertex(p1in.xyz + offset1, color1, segment.normal[0], segment.binormal[0], vec2(1, 0)),
         inRibbon[index1].pickID);

    emit(createVertex(p2in.xyz - offset2, color2, segment.normal[1], segment.binormal[1], vec2(0, 1)),
         inRibbon[index1].pickID);
    emit(createVertex(p2in.xyz + offset2, color2, segment.normal[1], segment.binormal[1], vec2(1, 1)),
         inRibbon[index1].pickID);
#endif

    EndPrimitive();
}
