/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include "utils/pickingutils.glsl"
#include "plotting/common.glsl"

#ifndef GLSL_VERSION_150
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_geometry_shader4 : enable
#endif

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform vec2 pixelSize;
uniform float borderWidth = 1;
uniform float antialiasing = 1.5; // [pixel]

in vec4 vColor[1];
in float vRadius[1];
in float vDepth[1];
flat in uint pickID_[];

out vec4 gColor;
out vec2 gPos;
out float gR;
flat out vec4 pickColor_;

void emit(vec2 c, float r, int x, int y) {

    float outerRadius = r + 0.5 * borderWidth;

    vec2 pos =  vec2((outerRadius + antialiasing) * x, (outerRadius + antialiasing) * y);

    gl_Position = vec4(getGLPositionFromPixel(c + pos), vDepth[0], 1);
    gColor = vColor[0];
    gR = outerRadius;
    gPos = pos;
    EmitVertex();
}

bool insideBox(vec2 pos, vec2 bottomLeft, vec2 topRight) {
    vec2 s = step(bottomLeft, pos) - (1 - step(pos, topRight));
    return s.x * s.y > 0.0;
}

void main(void) {

#ifndef GLSL_VERSION_150
    vec2 c = gl_PositionIn[0].xy;
#else
    vec2 c = gl_in[0].gl_Position.xy;
#endif

    // discard primitive if outside unit square
    if (!insideBox(c, vec2(0.0), vec2(1.0))) {
        EndPrimitive();
        return;
    }

    c = getPixelCoordsWithSpacing(c);

    pickColor_ = vec4(pickingIndexToColor(pickID_[0]), pickID_[0] == 0 ? 0.0 : 1.0);

    emit(c, vRadius[0], 1, 1);
    emit(c, vRadius[0], 1, -1);
    emit(c, vRadius[0], -1, 1);
    emit(c, vRadius[0], -1, -1);

    EndPrimitive();
}
