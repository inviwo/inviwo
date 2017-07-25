/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2017 Inviwo Foundation
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
#include "utils/pickingutils.glsl"

uniform GeometryParameters geometry;
uniform CameraParameters camera;

uniform bool pickingEnabled = false;

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

in vec4 worldPosition_[];
in vec4 cubeColor_[];
flat in float cubeSize_[];
flat in uint pickID_[];

out vec4 color_;
flat out vec4 pickColor_;
out vec4 worldPosition;

out vec3 normal;

void emit(vec4 v0, vec4 v1, vec4 v2, vec4 v3) {
    gl_Position = v0;
    EmitVertex();
    gl_Position = v1;
    EmitVertex();
    gl_Position = v2;
    EmitVertex();
    gl_Position = v3;
    EmitVertex();
    EndPrimitive();
}

void main(void) {
    worldPosition = worldPosition_[0];

    mat4 worldToViewMatrixInv = inverse(camera.worldToView);

    vec3 camDir = normalize((worldToViewMatrixInv[2]).xyz);     
    vec3 camPosModel =  worldToViewMatrixInv[3].xyz;
    // calculate cam position (in model space of the cube)
    vec3 camPos_ = camPosModel - worldPosition_[0].xyz;

    if (dot(camPos_, camDir) < camera.nearPlane + cubeSize_[0]) {
        // glyph intersects with the near plane of the camera, discard entire glyph, i.e. no output
        EndPrimitive();
        return;
    }
    
    color_ = cubeColor_[0];
    pickColor_ = vec4(pickingIndexToColor(pickID_[0]), pickingEnabled ? 1.f : 0.f);

    vec3 center = worldPosition_[0].xyz;
    float d = 0.5f * cubeSize_[0];

    float cx_m = center.x - d;
    float cx_p = center.x + d;
    float cy_m = center.y - d;
    float cy_p = center.y + d;
    float cz_m = center.z - d;
    float cz_p = center.z + d;

    vec4 v0 = (camera.worldToClip * vec4(cx_m, cy_p, cz_p, 1.f)); v0 /= v0.w;
    vec4 v1 = (camera.worldToClip * vec4(cx_m, cy_m, cz_p, 1.f)); v1 /= v1.w;
    vec4 v2 = (camera.worldToClip * vec4(cx_p, cy_p, cz_p, 1.f)); v2 /= v2.w;
    vec4 v3 = (camera.worldToClip * vec4(cx_p, cy_m, cz_p, 1.f)); v3 /= v3.w;
    vec4 v4 = (camera.worldToClip * vec4(cx_m, cy_p, cz_m, 1.f)); v4 /= v4.w;
    vec4 v5 = (camera.worldToClip * vec4(cx_p, cy_p, cz_m, 1.f)); v5 /= v5.w;
    vec4 v6 = (camera.worldToClip * vec4(cx_m, cy_m, cz_m, 1.f)); v6 /= v6.w;
    vec4 v7 = (camera.worldToClip * vec4(cx_p, cy_m, cz_m, 1.f)); v7 /= v7.w;

    normal = vec3(0.f, 0.f, 1.f);
    emit(v0,v1,v2,v3);

    normal = vec3(0.f, 1.f, 0.f);
    emit(v4,v0,v5,v2);

    normal = vec3(0.f, 0.f, -1.f);
    emit(v6,v4,v7,v5);
    
    normal = vec3(0.f, -1.f, 0.f);
    emit(v1,v6,v3,v7);
    
    normal = vec3(-1.f, 0.f, 0.f);
    emit(v4,v6,v0,v1);
    
    normal = vec3(1.f, 0.f, 0.f);
    emit(v2,v3,v5,v7);  
}
