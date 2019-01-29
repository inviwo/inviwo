/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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


uniform vec3 p; // plane pos. in volume space
uniform vec2 p_screen; // plane pos. in screen space
uniform vec3 n; // plane's normal
uniform vec3 u; // plane's up
uniform vec3 r; // plane's right
uniform float aspect_ratio = 1.0; // aspect ratio of screen
uniform vec2 canvas_size = vec2(0.0);
uniform float thickness_offset = 0.0; // plane's offset along normal
uniform float zoom_factor = 1.0;
uniform float correction_angle = 0.0;
uniform vec3 volume_dimensions = vec3(0.0);
uniform vec3 volume_spacing = vec3(0.0);

in vec2 uv; // Viewport coordinates in [0,1]

vec2 rotate2D(vec2 pt, float angle)
{
    return mat2(cos(angle), -sin(angle), sin(angle), cos(angle)) * pt;
}

void main() {
    //vec2 uv_offset = uv - p_screen;
    vec2 uv_offset = uv - vec2(0.5);
    uv_offset.y *= (canvas_size.y / canvas_size.x); // aspect ration
    vec2 uv_rotated = rotate2D(uv_offset, correction_angle);
    //vec2 uv_rotated = uv_offset;

    //vec3 vd = volume_dimensions / max(max(volume_dimensions.x, volume_dimensions.y), volume_dimensions.z);
    //vec3 vs = volume_spacing;

    // add thickness offset without spacing and dimensions transformation?
    //vec3 volume_coord = p + (vd/vd) * vs * (zoom_factor * (uv_rotated.x * r + uv_rotated.y * u) + thickness_offset * n);
    vec3 volume_coord = p + zoom_factor * (uv_rotated.x * r + uv_rotated.y * u) + thickness_offset * n;

    if (volume_coord.y < 0) {
        FragData0 = vec4(1, 0, 0, 1.0);
    } else {
        FragData0 = vec4(volume_coord, 1.0);
    }
}
