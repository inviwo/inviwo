/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

in vec3 texCoord_; // Viewport coordinates in [0,1]
//in vec2 uv; // Viewport coordinates in [0,1]

uniform sampler2D img;
uniform float scale = 1.0;
uniform float angle = 0.0;
uniform vec2 offset = vec2(0.0);
uniform vec2 img_size;
uniform int flip_x = 0;
uniform int flip_y = 0;

vec2 rotate2D(vec2 pt, float psi)
{
    return mat2(cos(psi), -sin(psi), sin(psi), cos(psi)) * pt;
}

void main() {
    vec2 uv = texCoord_.xy;
    //if (flip_x == 1) uv.x = 1.0 - uv.x;
    //if (flip_y == 1) uv.y = 1.0 - uv.y;
    float aspect_ratio = img_size.x / img_size.y;
    vec2 uv_scaled = (uv - vec2(0.5)) * vec2(aspect_ratio, 1.0) * (1.0/ scale) - offset;
    vec2 uv_transformed = rotate2D(uv_scaled, angle) + 0.5;
    if (flip_x == 1) uv_transformed.x = 1.0 - uv_transformed.x;
    if (flip_y == 1) uv_transformed.y = 1.0 - uv_transformed.y;
    vec4 color = texture(img, uv_transformed);

    FragData0 = color;
}
