/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include "opactopt/common.glsl"
#include "opactopt/approximation/fourier.glsl"
#include "opactopt/approximation/legendre.glsl"
#include "opactopt/approximation/piecewise.glsl"
#include "opactopt/approximation/powermoments.glsl"
#include "opactopt/approximation/trigmoments.glsl"

#ifdef DEBUG
uniform ivec2 debugCoords;

layout(std430, binding = 10) buffer debugCoeffs {
    float debugImportanceCoeffs[N_IMPORTANCE_SUM_COEFFICIENTS];
    float debugOpticalCoeffs[N_OPTICAL_DEPTH_COEFFICIENTS];
};
#endif

uniform ImageParameters imageParameters;
uniform sampler2D imageColor;
uniform sampler2D imageDepth;

#ifdef BACKGROUND_AVAILABLE
uniform ImageParameters bgParameters;
uniform sampler2D bgColor;
#endif

uniform layout(IMAGE_LAYOUT) IMAGE_UNIT importanceSumCoeffs[2];     // double buffering for gaussian filtering
uniform layout(IMAGE_LAYOUT) IMAGE_UNIT opticalDepthCoeffs;

void main(void) {
    // normalise colour
    vec4 color = texelFetch(imageColor, ivec2(gl_FragCoord.xy), 0);

#ifdef NORMALISE
    if (color.a != 0.0) color.rgb /= color.a;  // divide by normalisation weight
#endif

    // set alpha using optical depth
    float tauall = total(opticalDepthCoeffs, N_OPTICAL_DEPTH_COEFFICIENTS);

    color.a = color.a == 0.0 ? 0.0 : 1.0 - exp(-tauall);

#ifdef BACKGROUND_AVAILABLE
#ifdef NORMALISE
    color.rgb =
        color.a * color.rgb + (1 - color.a) * texelFetch(bgColor, ivec2(gl_FragCoord.xy), 0).rgb;
#else
    color.rgb = color.rgb + (1 - color.a) * texelFetch(bgColor, ivec2(gl_FragCoord.xy), 0).rgb;
#endif
#else
#ifdef NORMALISE
    color.rgb = color.a * color.rgb;
#endif
#endif

#ifdef DEBUG
    if (ivec2(gl_FragCoord.xy) == debugCoords) {
#ifdef COEFF_TEX_FIXED_POINT_FACTOR
        for (int i = 0; i < N_IMPORTANCE_SUM_COEFFICIENTS; i++)
            debugImportanceCoeffs[i] = imageLoad(importanceSumCoeffs[0], ivec3(debugCoords, i)).x /
                                       COEFF_TEX_FIXED_POINT_FACTOR;
        for (int i = 0; i < N_OPTICAL_DEPTH_COEFFICIENTS; i++)
            debugOpticalCoeffs[i] = imageLoad(opticalDepthCoeffs, ivec3(debugCoords, i)).x /
                                    COEFF_TEX_FIXED_POINT_FACTOR;
#else
        for (int i = 0; i < N_IMPORTANCE_SUM_COEFFICIENTS; i++)
            debugImportanceCoeffs[i] = imageLoad(importanceSumCoeffs[0], ivec3(debugCoords, i)).x;
        for (int i = 0; i < N_OPTICAL_DEPTH_COEFFICIENTS; i++)
            debugOpticalCoeffs[i] = imageLoad(opticalDepthCoeffs, ivec3(debugCoords, i)).x;
#endif
    }
#endif

    FragData0 = vec4(color.xyz, 1.0);
    if (color.a != 0) PickingData = vec4(0.0, 0.0, 0.0, 1.0);
    gl_FragDepth = texelFetch(imageDepth, ivec2(gl_FragCoord.xy), 0).x;
}
