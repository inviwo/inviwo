/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

uniform ImageParameters outportParameters_;

uniform sampler2D inport0_;
uniform sampler2D inport1_;
uniform float weight_ = 0.5;

// make sure there is a fall-back if COLOR_BLENDING wasn't set before
#ifndef COLOR_BLENDING
    // if only b is given, regular color mixing will be performed
#  define COLOR_BLENDING(a, b) (b)
#endif

/*
vec4 colorMix(vec4 colorA, vec4 colorB, vec4 param) {
    // f(a,b) = a * (1 - alpha) + b * alpha
    return mix(colorA, colorB, param);
}
*/

vec4 over(vec4 colorB, vec4 colorA) {
    // f(a,b) = b, b over a, regular front-to-back blending
    vec3 col = mix(colorB.rgb * colorB.a, colorA.rgb, colorA.a);
    float alpha = colorA.a + (1.0 - colorA.a) * colorB.a;
    return vec4(col, alpha);
}

vec4 multiply(vec4 colorA, vec4 colorB) {
    // f(a,b) = a * b
    return vec4(colorA.rgb * colorB.rgb, 1.0);
}

vec4 screen(vec4 colorA, vec4 colorB) {
    // f(a,b) = 1 - (1 - a) * (1 - b)
    return vec4(1.0 - (1.0 - colorA.rgb) * (1.0 - colorB.rgb), 1.0);
}

vec4 overlay(vec4 colorA, vec4 colorB) {
    // f(a,b) = 2 * a *b, if a < 0.5,   
    //        = 1 - 2(1 - a)(1 - b), otherwise (combination of Multiply and Screen)
    bvec3 less = lessThan(colorA.rgb, vec3(0.5));
    vec3 high =  1.0 - 2.0 * (1.0 - colorA.rgb) * (1.0 - colorB.rgb);
    vec3 low = 2.0 * colorA.rgb * colorB.rgb;

    return vec4(mix(high, low, less), 1.0);
}

vec4 divide(vec4 colorA, vec4 colorB) {
    // f(a,b) = a/b
    return vec4(colorA.rgb / colorB.rgb, 1.0);
}

vec4 addition(vec4 colorA, vec4 colorB) {
    // f(a,b) = a + b, clamped to [0,1]
    return vec4(min(colorA.rgb + colorB.rgb, 1.0), 1.0);
}

vec4 subtraction(vec4 colorA, vec4 colorB) {
    // f(a,b) = a - b, clamped to [0,1]
    return vec4(max(colorA.rgb - colorB.rgb, 0.0), 1.0);
}

vec4 difference(vec4 colorA, vec4 colorB) {
    //  f(a,b) = |a - b|
    return vec4(abs(colorA.rgb - colorB.rgb), 1.0);
}

vec4 darkenOnly(vec4 colorA, vec4 colorB) {
    //  f(a,b) = min(a, b), per component
    return vec4(min(colorA.rgb, colorB.rgb), 1.0);
}

vec4 brightenOnly(vec4 colorA, vec4 colorB) {
    //  f(a,b) = max(a, b), per component
    return vec4(max(colorA.rgb, colorB.rgb), 1.0);
}


void main() {
    vec2 texCoords = gl_FragCoord.xy * outportParameters_.reciprocalDimensions;
    vec4 color0 = texture(inport0_, texCoords);
    vec4 color1 = texture(inport1_, texCoords);
    vec4 result = COLOR_BLENDING(color0, color1);
    // mix result with original color,
    // if (weight_ == 1) the final color will be the result of the blending operation
    FragData0 = mix(color0, result, weight_);
}
