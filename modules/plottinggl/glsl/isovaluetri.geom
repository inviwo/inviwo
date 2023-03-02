/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

layout(points) in;
layout(triangle_strip, max_vertices=18) out;

#if !defined MAX_ISOVALUE_COUNT
#  define MAX_ISOVALUE_COUNT 1
#endif // MAX_ISOVALUE_COUNT

// need to ensure there is always at least one isovalue due to the use of the macro
// as array size in IsovalueParameters which will cause an error for size = 0.
#if MAX_ISOVALUE_COUNT < 1
#  undef MAX_ISOVALUE_COUNT
#  define MAX_ISOVALUE_COUNT 1
#endif

struct IsovalueParameters {
    float values[MAX_ISOVALUE_COUNT];
    vec4 colors[MAX_ISOVALUE_COUNT];
};

uniform IsovalueParameters isovalues;

uniform int borderWidth = 1;

uniform float triSize = 10; // in pixel
uniform vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
uniform mat4 trafo = mat4(1);
uniform vec2 screenDim;
uniform vec2 screenDimInv;

flat in int instanceID[];
out vec4 color_;

vec4 convertScreenToNDC(vec2 v, float z) {
    return vec4(v / screenDim * 2.0 - 1.0, z, 1.0);
}

void emit(in vec2 pos, in vec4 color, float z) {
    // gl_Position = trafo * vec4(pos * screenDimInv + vec2(position, 0), 0, 1);
    color_ = color;
    gl_Position = trafo * convertScreenToNDC(pos, z);
    // gl_Position = trafo * vec4(pos * screenDimInv, 0, 1);
    EmitVertex();
}

void main() {
    const float sqrt3half = 0.8660254037844386;

    float pos = isovalues.values[instanceID[0]] * screenDim.x;
    vec4 isocolor = vec4(isovalues.colors[instanceID[0]].rgb, 1.0);

    // render border of two triangles
    float a = triSize * 0.5 + 2.0 * borderWidth;
    float h = min(screenDim.y * 0.5, a * 2.0 * sqrt3half);

    float vpos = 0.0;
    emit(vec2(pos - a, vpos), color, 0.5);
    emit(vec2(pos + a, vpos), color, 0.5);
    emit(vec2(pos, vpos + h), color, 0.5);
    EndPrimitive();

    vpos = screenDim.y;
    emit(vec2(pos + a, vpos), color, 0.5);
    emit(vec2(pos - a, vpos), color, 0.5);
    emit(vec2(pos, vpos - h), color, 0.5);
    EndPrimitive();

    // render a quad connecting the triangles
    vec2 p1 = vec2(pos, h - borderWidth * 2.0);
    vec2 p2 = vec2(pos, screenDim.y - (h - borderWidth * 2.0));
    vec2 offset = vec2(0.5 * borderWidth, 0);

    emit(p1 - offset, color, 0.5);
    emit(p1 + offset, color, 0.5);
    emit(p2 + offset, color, 0.5);
    EndPrimitive();

    emit(p2 + offset, color, 0.5);
    emit(p2 - offset, color, 0.5);
    emit(p1 - offset, color, 0.5);
    EndPrimitive();

    // draw two triangles filled with the isovalue color, 
    // one at the bottom and one at the top
    a = triSize * 0.5;
    h = min(screenDim.y * 0.5 - 2.0*borderWidth, 2.0 * a * sqrt3half);

    vpos = borderWidth;
    emit(vec2(pos - a, vpos), isocolor, 0.0);
    emit(vec2(pos + a, vpos), isocolor, 0.0);
    emit(vec2(pos, vpos + h), isocolor, 0.0);
    EndPrimitive();

    vpos = screenDim.y - borderWidth;
    emit(vec2(pos + a, vpos), isocolor, 0.0);
    emit(vec2(pos - a, vpos), isocolor, 0.0);
    emit(vec2(pos, vpos - h), isocolor, 0.0);
    EndPrimitive();
}
