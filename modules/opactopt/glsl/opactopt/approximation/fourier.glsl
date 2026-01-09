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

#include "opactopt/common.glsl"

#ifndef IVW_OPACTOPT_FOURIER
#define IVW_OPACTOPT_FOURIER

#ifdef FOURIER

void project(layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, int N, float depth, float val) {
    float costheta = cos(TWOPI * depth);
    float sintheta = sin(TWOPI * depth);
    float coskm1theta = 1.0;
    float sinkm1theta = 0.0;
    float cosktheta = 0.0;
    float sinktheta = 0.0;

    for (int i = 0; i < N; i++) {
        float projVal = 0.0;
        if (i == 0) {
            projVal = val;
        } else if (i % 2 == 0) {
            cosktheta = coskm1theta * costheta - sinkm1theta * sintheta;
            projVal = val * cosktheta;
        } else {
            sinktheta = sinkm1theta * costheta + coskm1theta * sintheta;
            projVal = val * sinktheta;
        }

        // increment k
        if (i % 2 == 0 && i != 0) {
            coskm1theta = cosktheta;
            sinkm1theta = sinktheta;
        }

        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        optAdd(coeffTex, coord, projVal);
    }
}

float approximate(readonly layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, int N, float depth) {
    float sum = 0.0;
    int k = 0;

    float costheta = cos(TWOPI * depth);
    float sintheta = sin(TWOPI * depth);
    float coskm1theta = 1.0;
    float sinkm1theta = 0.0;
    float cosktheta = 0.0;
    float sinktheta = 0.0;

    for (int i = 0; i < N; i++) {
        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        float coeff = optLoad(coeffTex, coord);

        if (i == 0) {
            sum += coeff * depth;
        } else if (i % 2 == 0) {
            sinktheta = sinkm1theta * costheta + coskm1theta * sintheta;
            sum += (coeff / (PI * k)) * sinktheta;
        } else {
            cosktheta = coskm1theta * costheta - sinkm1theta * sintheta;
            sum += (coeff / (PI * k)) * (1 - cosktheta);
        }
        if (i % 2 == 0) {
            k++;
            if (i != 0) {
                coskm1theta = cosktheta;
                sinkm1theta = sinktheta;
            }
        }
    }

    return sum;
}

float total(readonly layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, int N) {
    return optLoad(coeffTex, ivec3(gl_FragCoord.xy, 0));
}

#endif  // FOURIER

#endif  // IVW_OPACTOPT_FOURIER
