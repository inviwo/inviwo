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

#include "utils/shading.glsl"

#if !defined(WEIGHTING_FUNC)
#  define WEIGHTING_FUNC evalLinearFunc
#endif  // WEIGHTING_FUNC

#if !defined(BLEND_FUNC)
#  define BLEND_FUNC blendPremultiplied
#endif  // BLEND_FUNC

uniform CameraParameters camera;

in TFGeom {
    flat vec2 center;
    flat vec4 color;
    flat vec4 pickColor;
    flat float radius;
    flat int primitive;
    smooth vec2 pos;
} in_frag;

// enum PrimitiveFunc { Box = 0, Linear, Smooth, Gaussian };


float evalBoxFunc(in vec2 pos) {
    float dist = distance(pos, in_frag.center);
    dist /= in_frag.radius;
    float opacity = (dist < 1.0) ? (1.0 - smoothstep(0.8, 1.0, dist)) : 0.0;
    return opacity;
}

float evalLinearFunc(in vec2 pos) {
    float dist = distance(pos, in_frag.center);
    dist /= in_frag.radius;
    dist = clamp(dist, 0.0, 1.0);
    float opacity = (1.0 - dist);
    return opacity;
}

float evalSmoothFunc(in vec2 pos) {
    float dist = distance(pos, in_frag.center);
    dist /= in_frag.radius;
    float opacity = (1.0 - smoothstep(0.0, 1.0, dist));
    return opacity;
}

float evalGaussianFunc(in vec2 pos) {
    float dist = distance(pos, in_frag.center);
    dist /= in_frag.radius;
    float distSq = dot(dist, dist);
    float kernelSizeSq = 1.0 / 9.0;

    float opacity = clamp(exp(- 0.693147180559945 * distSq / kernelSizeSq), 0.0, 1.0);
    return  opacity;
}

vec4 blendRegular(in vec4 color, in float weight) {
    vec4 result = mix(color, vec4(0, 0, 0, 0), 1.0 - weight);

    // color.rgb *= color.a;
    // result = color;
    
    result.a *= weight;
    return result;

    // // color.rgb *= color.a;
    // color.a *= weight;
    // // color.a = 0.2;
    // return color;
}

vec4 blendPremultiplied(in vec4 color, in float weight) {
    color.rgb *= color.a;
    color.a *= weight;
    return vec4(color.rgb, color.a);
}

void main() {
    //float weight = WEIGHTING_FUNC(in_frag.pos);
    float weight = 0.0f;

    switch (in_frag.primitive) {
        case 0:
            weight = evalBoxFunc(in_frag.pos);
            break;
        case 1:
            weight = evalLinearFunc(in_frag.pos);
            break;
        case 2:
            weight = evalSmoothFunc(in_frag.pos);
            break;
        case 3:
            weight = evalGaussianFunc(in_frag.pos);
            break;
        default:
            weight = evalLinearFunc(in_frag.pos);
            break;
    }

    // float alpha = weight;
    // alpha *= in_frag.color.a;
    // FragData0 = vec4(in_frag.color.rgb * alpha, alpha);
    // FragData0 = vec4(in_frag.color.rgb, weight * in_frag.color.a);
    // FragData0 = vec4(in_frag.color * alpha);

    FragData0 = BLEND_FUNC(in_frag.color, weight);

    PickingData = in_frag.pickColor;
}
