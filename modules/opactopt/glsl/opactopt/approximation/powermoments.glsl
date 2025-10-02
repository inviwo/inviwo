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

#include "opactopt/common.glsl"

#ifndef IVW_OPACTOPT_POWER_MOMENTS
#define IVW_OPACTOPT_POWER_MOMENTS

#ifdef POWER_MOMENTS

#include "opactopt/approximation/powermomentmaths.glsl"

#define TEX_OVERESTIMATION texelFetch(momentSettings, 1, 0).y

void project(layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, int N, float depth, float val) {
    float m_n = 1;
    for (int i = 0; i < N; i++) {
        float projVal = val * m_n;
        m_n *= depth;

        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        optAdd(coeffTex, coord, projVal);
    }
}

float approximate(layout(IMAGE_LAYOUT) IMAGE_UNIT  coeffTex, int N, float depth) {
    if (N == 5) {
        float b_0;
        vec2 b_even;
        vec2 b_odd;

        for (int i = 0; i < N; i++) {
            ivec3 coord = ivec3(gl_FragCoord.xy, i);
            float coeff = optLoad(coeffTex, coord);

            if (i == 0) {
                b_0 = coeff;
            } else if (i % 2 == 1) {
                b_odd[(i - 1) / 2] = coeff / b_0;
            } else {
                b_even[(i - 2) / 2] = coeff / b_0;
            }
        }

        float bias = 5e-7;
        const vec4 bias_vector = vec4(0, 0.375, 0, 0.375);
        return approximate4PowerMoments(b_0, b_even, b_odd, depth, bias, TEX_OVERESTIMATION,
                                        bias_vector);
    } else if (N == 7) {
        float b_0;
        vec3 b_even;
        vec3 b_odd;

        for (int i = 0; i < N; i++) {
            ivec3 coord = ivec3(gl_FragCoord.xy, i);
            float coeff = optLoad(coeffTex, coord);

            if (i == 0) {
                b_0 = coeff;
            } else if (i % 2 == 1) {
                b_odd[(i - 1) / 2] = coeff / b_0;
            } else {
                b_even[(i - 2) / 2] = coeff / b_0;
            }
        }

        float bias = 5e-6;
        const float bias_vector[6] = {0, 0.48, 0, 0.451, 0, 0.45};
        return approximate6PowerMoments(b_0, b_even, b_odd, depth, bias, TEX_OVERESTIMATION,
                                        bias_vector);
    } else if (N == 9) {
        float b_0;
        vec4 b_even;
        vec4 b_odd;

        for (int i = 0; i < N; i++) {
            ivec3 coord = ivec3(gl_FragCoord.xy, i);
            float coeff = optLoad(coeffTex, coord);

            if (i == 0) {
                b_0 = coeff;
            } else if (i % 2 == 1) {
                b_odd[(i - 1) / 2] = coeff / b_0;
            } else {
                b_even[(i - 2) / 2] = coeff / b_0;
            }
        }

        float bias = 5e-5;
        const float bias_vector[8] = {0, 0.75, 0, 0.67666666666666664,
                                      0, 0.63, 0, 0.60030303030303034};
        return approximate8PowerMoments(b_0, b_even, b_odd, depth, bias, TEX_OVERESTIMATION,
                                        bias_vector);
    } else {
        return 0;
    }
}

float total(layout(IMAGE_LAYOUT) IMAGE_UNIT  coeffTex, int N) {
    return optLoad(coeffTex, ivec3(gl_FragCoord.xy, 0));
}

#endif  // POWER_MOMENTS

#endif  // IVW_OPACTOPT_POWER_MOMENTS
