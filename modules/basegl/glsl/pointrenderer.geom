/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

// Owned by the pointRenderer Processor

#include "utils/structs.glsl"
#include "utils/pickingutils.glsl"

uniform GeometryParameters geometry;
uniform CameraParameters camera;

uniform ivec3 repeat = ivec3(1, 1, 1);
uniform vec3 shift = vec3(0.0, 0.0, 0.0);
uniform mat4 basis = mat4(1.0);
uniform float duplicateCutoff = 0.0;

struct Cam {
    vec3 pos;
    vec3 right;
    vec3 up;
};

struct Point {
    vec4 center;
    vec4 color;
    vec4 pickColor;
    float radius;
    uint index;
};

layout(points) in;

#if defined(ENABLE_DUPLICATE)
// account for potential duplication of the point billboard (4 vertices) along each axis
layout(triangle_strip, max_vertices = 32) out;
#else
layout(triangle_strip, max_vertices = 4) out;
#endif

in PointVert {
    vec4 color;
    flat float radius;
    flat uint pickID;
    flat uint index;
}
inPoint[];

#if defined(ENABLE_PERIODICITY)
flat in uint instance[];
#endif

out PointGeom {
    vec4 center;
    vec4 color;
    flat vec4 pickColor;
    vec3 camPos;
    float radius;
    flat uint index;
}
outPoint;

void emit(in vec3 pos, in Point point, in vec3 camPos) {
    vec4 projPos = camera.worldToClip * vec4(pos, 1.0);
    projPos /= projPos.w;

    gl_Position = vec4(projPos.xyz, 1.0);

    outPoint.radius = point.radius;
    outPoint.camPos = camPos;
    outPoint.center = point.center;
    outPoint.color = point.color;
    outPoint.pickColor = point.pickColor;
    outPoint.index = point.index;

    EmitVertex();
}

void emitQuad(in Point point, in Cam cam) {
    cam.right *= point.radius * 1.41421356;
    cam.up *= point.radius * 1.41421356;

    emit(point.center.xyz + cam.right - cam.up, point, cam.pos);
    emit(point.center.xyz - cam.right - cam.up, point, cam.pos);
    emit(point.center.xyz + cam.right + cam.up, point, cam.pos);
    emit(point.center.xyz - cam.right + cam.up, point, cam.pos);

    EndPrimitive();
}

ivec3 repeatPosition() {
#if defined(ENABLE_PERIODICITY)
    return ivec3((instance[0] % (repeat.x * repeat.y)) % repeat.x,
                 (instance[0] % (repeat.x * repeat.y)) / repeat.x,
                 instance[0] / (repeat.x * repeat.y));
#else
    return ivec3(0, 0, 0);
#endif
}

vec4 data2World(vec4 pos) {
#if defined(ENABLE_PERIODICITY)
    vec3 repeatPos = vec3(repeatPosition());

    mat4 trans =
        mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, repeatPos[0], repeatPos[1], repeatPos[2], 1);

    return geometry.dataToWorld * basis * trans * vec4(pos.xyz, 1.0);
#else
    return geometry.dataToWorld * pos;
#endif
}

vec4 shiftFractional(vec4 pos) {
#if defined(ENABLE_PERIODICITY)
    pos = inverse(basis) * pos;
    return vec4(mod((pos.xyz + shift), vec3(1, 1, 1)), pos.w);
#else
    return pos;
#endif
}

bool outOfView(vec3 camPos, vec3 camDir, float radius) {
    // glyph intersects with the near plane of the camera, discard entire glyph, i.e. no output
    return dot(camPos, camDir) < camera.nearPlane;
}

void main(void) {
    if (inPoint[0].radius <= 0 || inPoint[0].color.a <= 0) {
        EndPrimitive();
        return;
    }

    Point point;

    point.color = inPoint[0].color;
    point.pickColor.xyz = pickingIndexToColor(inPoint[0].pickID);
    point.pickColor.w = inPoint[0].pickID == 0 ? 0.0 : 1.0;
    point.radius = inPoint[0].radius;
    point.index = inPoint[0].index;

    // TODO handle orthographic cam...
    Cam cam;
    vec3 camDir = normalize((camera.viewToWorld[2]).xyz);
    vec3 camPosWorld = camera.viewToWorld[3].xyz;
    // camera coordinate system in object space
    cam.right = normalize(cross(camDir, camera.viewToWorld[1].xyz));
    cam.up = normalize(cross(camDir, cam.right));
    
#if defined(ENABLE_DUPLICATE)
    vec4 pos = shiftFractional(gl_in[0].gl_Position);
    ivec3 repeatPos = repeatPosition();

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            for (int z = -1; z <= 1; ++z) {

                if (x < 0 && repeatPos.x != 0) continue;
                if (x > 0 && repeatPos.x != repeat.x - 1) continue;

                if (y < 0 && repeatPos.y != 0) continue;
                if (y > 0 && repeatPos.y != repeat.y - 1) continue;

                if (z < 0 && repeatPos.z != 0) continue;
                if (z > 0 && repeatPos.z != repeat.z - 1) continue;

                vec3 newPos = pos.xyz + vec3(x, y, z);
                if (all(greaterThan(newPos, vec3(-duplicateCutoff))) &&
                    all(lessThan(newPos, vec3(1.0 + duplicateCutoff)))) {
                    point.center = data2World(vec4(newPos, pos.w));
                     // calculate cam position (in model space of the point)
                    cam.pos = camPosWorld - point.center.xyz;
                    if (!outOfView(cam.pos, camDir, point.radius)) {
                        emitQuad(point, cam);
                    }
                }
            }
        }
    }
    EndPrimitive();
#else

    point.center = data2World(shiftFractional(gl_in[0].gl_Position));
    // calculate cam position (in model space of the point)
    cam.pos = camPosWorld - point.center.xyz;
    if (outOfView(cam.pos, camDir, point.radius)) {
        EndPrimitive();
    } else {
        emitQuad(point, cam);
    }
#endif
}
