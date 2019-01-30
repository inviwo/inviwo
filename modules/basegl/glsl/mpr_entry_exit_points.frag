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
uniform float aspect_ratio; // aspect ratio of screen
uniform float thickness_offset; // plane's offset along normal
uniform float thickness_offset_other; // plane's offset along normal
uniform float zoom_factor;
uniform float correction_angle;
uniform vec3 volume_dimensions;
uniform vec3 volume_spacing;

in vec2 uv; // Viewport coordinates in [0,1]

vec2 rotate2D(vec2 pt, float angle)
{
    return mat2(cos(angle), -sin(angle), sin(angle), cos(angle)) * pt;
}

float maxmax(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

float minmin(vec3 v)
{
    return min(min(v.x, v.y), v.z);
}

void main() {
    vec2 uv_offset = (uv - p_screen) * vec2(aspect_ratio, 1.0);
    vec2 uv_rotated = rotate2D(uv_offset, correction_angle);

    vec3 vd = volume_dimensions / maxmax(volume_dimensions);
    vec3 vs = volume_spacing / minmin(volume_spacing);

    vec3 offset_on_plane = (1.0 / vd) * (1.0 / vs) * zoom_factor * (uv_rotated.x * r + uv_rotated.y * u);
    vec3 pt_on_plane = p + offset_on_plane;
    vec3 final_volume_coord = pt_on_plane + thickness_offset * n;
    vec3 other_volume_coord = pt_on_plane + thickness_offset_other * n;

    // check if outside of volume
    // ToDo: simplify or put in separate function
    // ToDo: clamp to border of volume
    float volume_alpha = 1.0;

    if (any(lessThan(final_volume_coord, vec3(0.0))) || any(greaterThan(final_volume_coord, vec3(1.0))) ||
        any(lessThan(other_volume_coord, vec3(0.0))) || any(greaterThan(other_volume_coord, vec3(1.0)))) {
        volume_alpha = 0.0;
        final_volume_coord = vec3(0.0);
    }

    FragData0 = vec4(final_volume_coord, volume_alpha);
}
