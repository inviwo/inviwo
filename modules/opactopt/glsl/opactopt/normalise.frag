/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025-2026 Inviwo Foundation
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

// Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

#include "utils/structs.glsl"
#include "utils/blend.glsl"

#include "opactopt/common.glsl"
#include "opactopt/approximation/fourier.glsl"
#include "opactopt/approximation/legendre.glsl"
#include "opactopt/approximation/piecewise.glsl"
#include "opactopt/approximation/powermoments.glsl"
#include "opactopt/approximation/trigmoments.glsl"


uniform ImageParameters imageParameters;
uniform sampler2D imageColor;
uniform sampler2D imageDepth;

#ifdef BACKGROUND_AVAILABLE
uniform ImageParameters bgParameters;
uniform sampler2D bgColor;
uniform sampler2D bgDepth;
uniform sampler2D bgPicking;
#endif

uniform layout(IMAGE_LAYOUT) IMAGE_UNIT importanceSumCoeffs[2];     // double buffering for gaussian filtering
uniform layout(IMAGE_LAYOUT) IMAGE_UNIT opticalDepthCoeffs;

void main(void) {
    // normalize color
    vec4 imageColorValue = texelFetch(imageColor, ivec2(gl_FragCoord.xy), 0);
    float imageDepthValue = texelFetch(imageDepth, ivec2(gl_FragCoord.xy), 0).x;

    // divide by normalization weight
    if (imageColorValue.a != 0.0) imageColorValue.rgb /= imageColorValue.a;  

    // set alpha using optical depth
    float tauall = total(opticalDepthCoeffs, N_OPTICAL_DEPTH_COEFFICIENTS);
    imageColorValue.a = imageColorValue.a == 0.0 ? 0.0 : 1.0 - exp(-tauall);

    vec4 picking = vec4(0.0, 0.0, 0.0, 1.0);

#ifdef BACKGROUND_AVAILABLE
    vec4 bgColorValue = texelFetch(bgColor, ivec2(gl_FragCoord.xy), 0);
    float bgDepthValue = texelFetch(bgDepth, ivec2(gl_FragCoord.xy), 0).x;

    vec4 color = imageDepthValue < bgDepthValue ?
                    alphaBlend(imageColorValue, bgColorValue) :
                    alphaBlend(bgColorValue, imageColorValue);

    if (bgDepthValue < imageDepthValue) {
        picking = texelFetch(bgPicking, ivec2(gl_FragCoord.xy), 0);
    }

    float depth = min(imageDepthValue, bgDepthValue);
#else
    float depth = imageDepthValue;
    vec4 color = imageColorValue;
#endif

    FragData0 = color;
    PickingData = picking;
    gl_FragDepth = depth;
}
