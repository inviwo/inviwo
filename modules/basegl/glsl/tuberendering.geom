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

#ifndef GLSL_VERSION_150
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_geometry_shader4 : enable
#endif

#include "utils/structs.glsl"

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 24) out;


#include "utils/sampler3d.glsl"
#include "utils/structs.glsl"

uniform GeometryParameters geometry;
uniform CameraParameters camera;


uniform float radius;
uniform mat4 ModelviewProjection;
in vec3 worldPosition_[4]; 
in vec3 normal_[4];   
in vec4 vColor_[4];
vec4 prismoid[8]; 


out vec4 color_;
out vec3 worldPos_;
out vec3 startPos_;
out vec3 endPos_;
out vec3 gEndplanes[2];

void emitV(int a){
    color_ = vColor_[a <= 3 ? 1 : 2]; 
    worldPos_ = prismoid[a].xyz;
    gl_Position = camera.worldToClip * prismoid[a];  
    EmitVertex();
}

void emit(int a, int b, int c, int d)
{
    emitV(a);
    emitV(b);
    emitV(c);
    emitV(d);
    EndPrimitive(); 
}

vec3 getOrthogonalVector(vec3 v,vec3 A,vec3 B){
    if(abs(1-dot(v,A))>0.001){
        return normalize(cross(v,A));
    }else{
        return normalize(cross(v,B));
    }
}

void main()
{
    // Compute orientation vectors for the two connecting faces:
    vec3 p0, p1, p2, p3;

#ifndef GLSL_VERSION_150 
    p0 = gl_PositionIn[0].xyz;
    p1 = gl_PositionIn[1].xyz;
    p2 = gl_PositionIn[2].xyz;
    p3 = gl_PositionIn[3].xyz; 
#else
    p0 = gl_in[0].gl_Position.xyz;
    p1 = gl_in[1].gl_Position.xyz;
    p2 = gl_in[2].gl_Position.xyz;
    p3 = gl_in[3].gl_Position.xyz;
#endif

    vec3 n0 = normalize(p1-p0);
    vec3 n1 = normalize(p2-p1);
    vec3 n2 = normalize(p3-p2);
    vec3 u = normalize(n0+n1);
    vec3 v = normalize(n1+n2);
    gEndplanes[0] = u;
    gEndplanes[1] = v;


    startPos_ = p1;
    endPos_ = p2;


    vec3 B = normalize((camera.viewToWorld * vec4(0,0,1,0)).xyz);
    vec3 A = normalize((camera.viewToWorld * vec4(1,0,0,0)).xyz);
    vec3 N1,N2; 
    N1 = getOrthogonalVector(n1,A,B);
    N2 = getOrthogonalVector(n2,A,B);

    // Declare scratch variables for basis vectors:
    vec3 i,j,k; float r = radius;

    // Compute face 1 of 2:
    j = u; i = N1; k = normalize(cross(i, j)); i = normalize(cross(k, j)); ; i *= r; k *= r;
    prismoid[0] = vec4(p1 + i + k, 1);
    prismoid[1] = vec4(p1 + i - k, 1);
    prismoid[2] = vec4(p1 - i - k, 1);
    prismoid[3] = vec4(p1 - i + k, 1);

    // Compute face 2 of 2:
    j = v; i = N2; k = normalize(cross(i, j)); i = normalize(cross(k, j)); i *= r; k *= r;
    prismoid[4] = vec4(p2 + i + k, 1);
    prismoid[5] = vec4(p2 + i - k, 1);
    prismoid[6] = vec4(p2 - i - k, 1);
    prismoid[7] = vec4(p2 - i + k, 1);

    // Emit the six faces of the prismoid:
    emit(0,1,3,2); emit(5,4,6,7);
    emit(4,5,0,1); emit(3,2,7,6);
    emit(0,3,4,7); emit(2,1,6,5);
}


