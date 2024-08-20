/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

// Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

#ifdef DEBUG
uniform ivec2 debugCoords;

layout(std430, binding = 10) buffer debugCoeffs {
    float debugImportanceCoeffs[N_IMPORTANCE_SUM_COEFFICIENTS];
    float debugOpticalCoeffs[N_OPTICAL_DEPTH_COEFFICIENTS];
};
#endif

uniform ImageParameters imageParameters;
uniform sampler2D imageColor;
uniform layout(r32i) iimage2DArray importanceSumCoeffs[2];
uniform layout(r32i) iimage2DArray opticalDepthCoeffs;

#ifdef FOURIER
    #include "opactopt/approximate/fourier.glsl"
#endif
#ifdef LEGENDRE
    #include "opactopt/approximate/legendre.glsl"
#endif
#ifdef PIECEWISE
    #include "opactopt/approximate/piecewise.glsl"
#endif

void main(void) {
    // normalise colour
    vec4 color = texelFetch(imageColor, ivec2(gl_FragCoord.xy), 0);
    color.rgb /= color.a;

    // set alpha using optical depth
    float tauall = total(opticalDepthCoeffs, N_OPTICAL_DEPTH_COEFFICIENTS);
    if (color.a != 0.0)
        color.a = 1.0 - exp(-tauall);

    #ifdef DEBUG
        if (ivec2(gl_FragCoord.xy) == debugCoords) {
            for (int i = 0; i < N_IMPORTANCE_SUM_COEFFICIENTS; i++)
                debugImportanceCoeffs[i] = imageLoad(importanceSumCoeffs[0], ivec3(debugCoords, i)).x / COEFF_TEX_FIXED_POINT_FACTOR;
            for (int i = 0; i < N_OPTICAL_DEPTH_COEFFICIENTS; i++)
                debugOpticalCoeffs[i] = imageLoad(opticalDepthCoeffs, ivec3(debugCoords, i)).x / COEFF_TEX_FIXED_POINT_FACTOR;
        }
    #endif

    FragData0 = color;
    if (color.a != 0) PickingData = vec4(0.0, 0.0, 0.0, 1.0);
}
