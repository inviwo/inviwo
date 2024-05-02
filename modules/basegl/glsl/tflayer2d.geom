/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

in float vs_width[];
in vec4 vs_color[];
out vec4 gs_color;

in TFVertex {
    flat vec2 center;
    flat vec4 color;
    flat float radius;
    flat int primitive;
    flat uint pickID;
} in_vert[];

out TFGeom {
    flat vec2 center;
    flat vec4 color;
    flat vec4 pickColor;
    flat float radius;
    flat int primitive;
    smooth vec2 pos;
} out_vert;


struct TFPrimitive {
    vec2 center;
    vec4 color;
    vec4 pickColor;
    float radius;
    int primitive;
};

void emit(in vec4 pos, in TFPrimitive point) {
    gl_Position = pos;

    out_vert.center = point.center;
    out_vert.color = point.color;
    out_vert.pickColor = point.pickColor;
    out_vert.radius = point.radius;
    out_vert.primitive = point.primitive;
    out_vert.pos = pos.xy;
    EmitVertex();
}

void main(void) {
    //if (in_vert[0].radius <= 0 || in_vert[0].color.a <= 0) {
    //    EndPrimitive();
    //    return;
    //}

    vec4 pos = gl_in[0].gl_Position;
    vec4 posndc = pos / pos.w;

    posndc.z = 0.5;

    vec2 center = posndc.xy;

    float r = in_vert[0].radius;
    
    vec2 topLeft = center + vec2(-r, r);
    vec2 bottomLeft = center + vec2(-r, -r);
    vec2 topRight = center + vec2(r, r);
    vec2 bottomRight = center + vec2(r, -r);

    vec4 pickColor;
    pickColor.rgb = pickingIndexToColor(in_vert[0].pickID);
    pickColor.w = in_vert[0].pickID == 0 ? 0.0 : 1.0;

    TFPrimitive tfprimitive = TFPrimitive(in_vert[0].center, in_vert[0].color, pickColor, r, in_vert[0].primitive);

    emit(vec4(topLeft, posndc.z, 1.0), tfprimitive);
    emit(vec4(bottomLeft, posndc.z, 1.0), tfprimitive);
    emit(vec4(topRight, posndc.z, 1.0), tfprimitive);
    emit(vec4(bottomRight, posndc.z, 1.0), tfprimitive);
    
    EndPrimitive();
}
