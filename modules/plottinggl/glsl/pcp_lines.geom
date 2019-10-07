/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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


#include "pcp_common.glsl"
#include "utils/pickingutils.glsl"

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in float vScalarMeta[2];
flat in uint vPicking[2];

vec4 triverts[4];
float signValues[4];

out vec4 lPickColor;
out float lScalarMeta;
out float lPixelsFromCenter; // Distance from line center [pixel]

uniform float lineWidth = 3; // line width [pixel]
uniform float antialiasing = 1.0; // width of antialised edged [pixel]

void emit(in vec4 pos, in float scalarMeta, in float d, in uint picking) {
    gl_Position = pos;
    lPixelsFromCenter = d;
    lPickColor = vec4(pickingIndexToColor(picking), picking == 0 ? 0.0 : 1.0);
    lScalarMeta = scalarMeta;
    EmitVertex();
}

void main() {
    // Compute orientation vectors for the two connecting faces:
    vec4 p[2];

    p[0] = gl_in[0].gl_Position;
    p[1] = gl_in[1].gl_Position;

    // Create a vector that is orthogonal to the line
    vec3 orthogonalLine = p[0].xyz - p[1].xyz;
    orthogonalLine = normalize(vec3(orthogonalLine.y, -orthogonalLine.x, orthogonalLine.z));

    // Scale the linewidth with the window dimensions
    float w = (lineWidth * 0.5 + 1.0 * antialiasing);
    float r1 = w * getPixelSpacing().x;
    float r2 = w * getPixelSpacing().y;

    // Scale the orthogonal vector with the linewidth
    vec3 j = vec3(orthogonalLine.x * r1, orthogonalLine.y * r2, orthogonalLine.z);
    
    
    // Rectangle enclosing the line
    // Left top
    emit(vec4(p[0].xyz + j, 1.0), vScalarMeta[0], w, vPicking[0]);
    // Left bottom
    emit(vec4(p[0].xyz - j, 1.0), vScalarMeta[0], -w, vPicking[0]);
    // Right top
    emit(vec4(p[1].xyz + j, 1.0), vScalarMeta[1], w, vPicking[1]);
    // Right bottom
    emit(vec4(p[1].xyz - j, 1.0), vScalarMeta[1], -w, vPicking[1]);
    
    EndPrimitive();
}
