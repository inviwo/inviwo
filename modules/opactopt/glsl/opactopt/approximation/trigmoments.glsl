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

#ifdef TRIG_MOMENTS

#include "opactopt/approximation/trigmomentmaths.glsl"

#define TEX_WRAPPING_ZONE_PARAMETERS texelFetch(momentSettings, 0, 0)
#define TEX_OVERESTIMATION texelFetch(momentSettings, 1, 0).y

void project(layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, int N, float depth, float val) {
    vec4 wrapping_zone_parameters = TEX_WRAPPING_ZONE_PARAMETERS;
    float phase = fma(wrapping_zone_parameters.y, depth, wrapping_zone_parameters.y);
    float costheta = cos(phase);
    float sintheta = sin(phase);
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

float approximate(layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, int N, float depth) {
    vec4 wrapping_zone_parameters = TEX_WRAPPING_ZONE_PARAMETERS;

    float b_0;
    int k = 0;
    float bias;

    if (N == 5) {
        vec2 trig_b[2];

        for (int i = 0; i < N; i++) {
            ivec3 coord = ivec3(gl_FragCoord.xy, i);
            float coeff = optLoad(coeffTex, coord);

            if (i == 0) {
                b_0 = coeff;
            } else if (i % 2 == 1) {
                trig_b[k - 1].y = coeff / b_0;
            } else {
                trig_b[k - 1].x = coeff / b_0;
            }

            if (i % 2 == 0) k++;
        }

        bias = 4e-7;
        return approximate2TrigonometricMoments(b_0, trig_b, depth, bias, TEX_OVERESTIMATION,
                                                TEX_WRAPPING_ZONE_PARAMETERS);
    } else if (N == 7) {
        vec2 trig_b[3];

        for (int i = 0; i < N; i++) {
            ivec3 coord = ivec3(gl_FragCoord.xy, i);
            float coeff = optLoad(coeffTex, coord);

            if (i == 0) {
                b_0 = coeff;
            } else if (i % 2 == 1) {
                trig_b[k - 1].y = coeff / b_0;
            } else {
                trig_b[k - 1].x = coeff / b_0;
            }

            if (i % 2 == 0) k++;
        }

        bias = 8e-7;
        return approximate3TrigonometricMoments(b_0, trig_b, depth, bias, TEX_OVERESTIMATION,
                                                TEX_WRAPPING_ZONE_PARAMETERS);
    } else if (N == 9) {
        vec2 trig_b[4];

        for (int i = 0; i < N; i++) {
            ivec3 coord = ivec3(gl_FragCoord.xy, i);
            float coeff = optLoad(coeffTex, coord);

            if (i == 0) {
                b_0 = coeff;
            } else if (i % 2 == 1) {
                trig_b[k - 1].y = coeff / b_0;
            } else {
                trig_b[k - 1].x = coeff / b_0;
            }

            if (i % 2 == 0) k++;
        }

        bias = 1.5e-6;
        return approximate4TrigonometricMoments(b_0, trig_b, depth, bias, TEX_OVERESTIMATION,
                                                TEX_WRAPPING_ZONE_PARAMETERS);
    } else {
        return 0;
    }
}

float total(layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, int N) {
    return optLoad(coeffTex, ivec3(gl_FragCoord.xy, 0));
}

#endif
