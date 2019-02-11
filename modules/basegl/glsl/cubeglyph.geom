/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

// Owned by the CubeRenderer Processor

#include "utils/structs.glsl"
#include "utils/pickingutils.glsl"

uniform GeometryParameters geometry;
uniform CameraParameters camera;

uniform bool pickingEnabled = false;

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

in vec4 vColor[1];
flat in float vSize[1];
flat in uint vPickID[1];

out vec4 color;
flat out vec4 picking;
out vec4 worldPosition;
out vec3 normal;

vec4 corners[8];
vec4 pickColor;

void emitVertex(int v, vec3 n) {
    gl_Position = corners[v];
    color = vColor[0];
    picking = pickColor;
    worldPosition = gl_in[0].gl_Position;
    normal = n;
    EmitVertex();
}

void emitFace(int v0, int v1, int v2, int v3, vec3 n) {
    emitVertex(v0, n);
    emitVertex(v1, n);
    emitVertex(v2, n);
    emitVertex(v3, n);
    EndPrimitive();
}

void main(void) {
    worldPosition = gl_in[0].gl_Position;
    pickColor = vec4(pickingIndexToColor(vPickID[0]), vPickID[0] == 0 ? 0.0 : 1.0);

    mat4 worldToViewMatrixInv = inverse(camera.worldToView);

    vec3 camDir = normalize((worldToViewMatrixInv[2]).xyz);     
    vec3 camPosModel =  worldToViewMatrixInv[3].xyz;
    // calculate cam position (in model space of the cube)
    vec3 camPos_ = camPosModel - worldPosition.xyz;

    if (dot(camPos_, camDir) < camera.nearPlane + vSize[0]) {
        // glyph intersects with the near plane of the camera, discard entire glyph, 
        // i.e. no output
        EndPrimitive();
        return;
    }
    
    vec3 center = worldPosition.xyz;
    float d = 0.5f * vSize[0];

    float cx_m = center.x - d;
    float cx_p = center.x + d;
    float cy_m = center.y - d;
    float cy_p = center.y + d;
    float cz_m = center.z - d;
    float cz_p = center.z + d;

    corners[0] = (camera.worldToClip * vec4(cx_m, cy_p, cz_p, 1.f)); corners[0] /= corners[0].w;
    corners[1] = (camera.worldToClip * vec4(cx_m, cy_m, cz_p, 1.f)); corners[1] /= corners[1].w;
    corners[2] = (camera.worldToClip * vec4(cx_p, cy_p, cz_p, 1.f)); corners[2] /= corners[2].w;
    corners[3] = (camera.worldToClip * vec4(cx_p, cy_m, cz_p, 1.f)); corners[3] /= corners[3].w;
    corners[4] = (camera.worldToClip * vec4(cx_m, cy_p, cz_m, 1.f)); corners[4] /= corners[4].w;
    corners[5] = (camera.worldToClip * vec4(cx_p, cy_p, cz_m, 1.f)); corners[5] /= corners[5].w;
    corners[6] = (camera.worldToClip * vec4(cx_m, cy_m, cz_m, 1.f)); corners[6] /= corners[6].w;
    corners[7] = (camera.worldToClip * vec4(cx_p, cy_m, cz_m, 1.f)); corners[7] /= corners[7].w;

    emitFace(0, 1, 2, 3, vec3(0.f, 0.f, 1.f));
    emitFace(4, 0, 5, 2, vec3(0.f, 1.f, 0.f));
    emitFace(6, 4, 7, 5, vec3(0.f, 0.f, -1.f));
    emitFace(1, 6, 3, 7, vec3(0.f, -1.f, 0.f));
    emitFace(4, 6, 0, 1, vec3(-1.f, 0.f, 0.f));
    emitFace(2, 3, 5, 7, vec3(1.f, 0.f, 0.f));  
}
