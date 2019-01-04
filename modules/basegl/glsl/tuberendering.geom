/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 24) out;

uniform GeometryParameters geometry;
uniform CameraParameters camera;

in vec4 vColor_[4];
flat in float vRadius_[4];
flat in uint pickID_[4];

out vec4 color_;
flat out vec4 pickColor_;
out vec3 worldPos_;
out vec3 startPos_;
out vec3 endPos_;
out vec3 gEndplanes[2];
out float radii[2];

vec3 prismoid[8];
vec4 pickColor;
vec3 startPos;
vec3 endPos;
vec3 capNormals[2];

void emitVertex(int a) { 
    gl_Position = camera.worldToClip * vec4(prismoid[a], 1.0);  
    color_ = vColor_[a <= 3 ? 1 : 2];
    pickColor_ = pickColor;
    worldPos_ = prismoid[a];
    startPos_ = startPos;
    endPos_ = endPos;
    gEndplanes[0] = capNormals[0];
    gEndplanes[1] = capNormals[1];
    radii[0] = vRadius_[1];
    radii[1] = vRadius_[2];
    EmitVertex();
}

void emitFace(int a, int b, int c, int d) {
    emitVertex(a);
    emitVertex(b);
    emitVertex(c);
    emitVertex(d);
    EndPrimitive(); 
}

// v should be normalized
vec3 findOrthogonalVector(vec3 v) {
    vec3 A = normalize((camera.viewToWorld * vec4(1,0,0,0)).xyz);
    if (abs(dot(v,A)) > 0.5) {
        return cross(v,A);
    } else {
        vec3 B = normalize((camera.viewToWorld * vec4(0,0,1,0)).xyz);
        return cross(v,B);
    }
}

/*  Corners of the prismoid;
 *   
 *        7------ 4                     
 *       /|  p2  /|                 
 *      / |  *  / |                 
 *     /  6----/--5                 
 *    3-------0  /                  
 *    | / p1  | /                   
 *    |/  *   |/                    
 *    2-------1
 *
 * if we let p2-p1 -> z i -> x, and k -> y
 */

void main() {
    // Compute orientation vectors for the two connecting faces:
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;
    vec3 p3 = gl_in[3].gl_Position.xyz;
    if (p1 == p2) return; // zero size segment

    startPos = p1;
    endPos = p2;

    pickColor = vec4(pickingIndexToColor(pickID_[0]), pickID_[0] == 0 ? 0.0 : 1.0);

    vec3 prevDir = p1-p0;
    vec3 tubeDir = normalize(p2-p1);
    vec3 nextDir = p3-p2;
    capNormals[0] = normalize(tubeDir + (prevDir != vec3(0) ? normalize(prevDir) : prevDir));
    capNormals[1] = normalize(tubeDir + (nextDir != vec3(0) ? normalize(nextDir) : nextDir));

    vec3 radialDir = findOrthogonalVector(tubeDir);

    // Compute face 1 of 2:
    vec3 k = vRadius_[1] * normalize(cross(radialDir, capNormals[0])); 
    vec3 i = vRadius_[1] * normalize(cross(k, capNormals[0])); 
    prismoid[0] = p1 + i + k;
    prismoid[1] = p1 + i - k;
    prismoid[2] = p1 - i - k;
    prismoid[3] = p1 - i + k;

    // Compute face 2 of 2:
    k = vRadius_[2] * normalize(cross(radialDir, capNormals[1])); 
    i = vRadius_[2] * normalize(cross(k, capNormals[1])); 
    prismoid[4] = p2 + i + k;
    prismoid[5] = p2 + i - k;
    prismoid[6] = p2 - i - k;
    prismoid[7] = p2 - i + k;

    // Emit the six faces of the prismoid:
    emitFace(0,1,3,2); 
    emitFace(5,4,6,7);
    emitFace(4,5,0,1); 
    emitFace(3,2,7,6);
    emitFace(0,3,4,7); 
    emitFace(2,1,6,5);
}
