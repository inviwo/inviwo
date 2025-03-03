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

// Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

uniform ivec2 screenSize;

#ifdef COEFF_TEX_FIXED_POINT_FACTOR
uniform layout(r32i) iimage2DArray importanceSumCoeffs[2]; // double buffering for gaussian filtering
uniform layout(r32i) iimage2DArray opticalDepthCoeffs;
#else
uniform layout(size1x32) image2DArray importanceSumCoeffs[2]; // double buffering for gaussian filtering
uniform layout(size1x32) image2DArray opticalDepthCoeffs;
#endif


void main() {
    const ivec2 coords = ivec2(gl_FragCoord.xy);

    if (coords.x >= 0 && coords.y >= 0 && coords.x < screenSize.x &&
        coords.y < screenSize.y) {
        // clear coefficient buffers
        #ifdef COEFF_TEX_FIXED_POINT_FACTOR
        for (int i = 0; i < N_IMPORTANCE_SUM_COEFFICIENTS; i++) {
            imageStore(importanceSumCoeffs[0], ivec3(coords, i), ivec4(0));
            imageStore(importanceSumCoeffs[1], ivec3(coords, i), ivec4(0));
        }
        for (int i = 0; i < N_OPTICAL_DEPTH_COEFFICIENTS; i++) {
            imageStore(opticalDepthCoeffs, ivec3(coords, i), ivec4(0));
        }
        #else
        for (int i = 0; i < N_IMPORTANCE_SUM_COEFFICIENTS; i++) {
            imageStore(importanceSumCoeffs[0], ivec3(coords, i), vec4(0));
            imageStore(importanceSumCoeffs[1], ivec3(coords, i), vec4(0));
        }
        for (int i = 0; i < N_OPTICAL_DEPTH_COEFFICIENTS; i++) {
            imageStore(opticalDepthCoeffs, ivec3(coords, i), vec4(0));
        }
        #endif
    }

    discard;
}
