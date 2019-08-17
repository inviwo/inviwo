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

#include "plotting/common.glsl"
#include "utils/structs.glsl"
#include "utils/sampler2d.glsl"

layout(location = 0) in float X;
layout(location = 1) in float Y;
layout(location = 2) in float C;
layout(location = 3) in float R;
layout(location = 4) in uint in_PickId;

uniform sampler2D transferFunction;

out vec4 vColor;
out float vRadius;
out float vDepth;
flat out uint pickID_;

uniform vec2 minmaxX;
uniform vec2 minmaxY;
uniform vec2 minmaxC;
uniform vec2 minmaxR;
uniform vec4 default_color;

uniform float minRadius;
uniform float maxRadius;

uniform int has_color = 0;
uniform int has_radius = 0;

uniform bool pickingEnabled = false;

float norm(in float v, in vec2 mm) { 
    return (v - mm.x) / (mm.y - mm.x); 
}

void main(void) {
    if (has_color == 1) {
        float c = norm(C, minmaxC);
        vColor = texture(transferFunction, vec2(c, 0.5));
    } else {
        vColor = default_color;
    }

    if (has_radius == 1) {
        float r = norm(R, minmaxR);
        vRadius = minRadius + r * (maxRadius - minRadius);
    } else {
        vRadius = maxRadius;
    }
    vDepth = 0.5;

    float x = norm(X, minmaxX);
    float y = norm(Y, minmaxY);
    gl_Position = vec4(x, y, 0.5, 1);
    
    pickID_ = pickingEnabled ? in_PickId : 0;
}
