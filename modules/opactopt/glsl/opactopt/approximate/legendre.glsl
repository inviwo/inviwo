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

#define MAX_ORDER_P 5
#define MAX_ORDER_Q 6


void FillPowers(float depth, out float[N_APPROXIMATION_COEFFICIENTS] x) {
    x[0] = 1.0;
    for (int i = 1; i < N_APPROXIMATION_COEFFICIENTS; i++)
        x[i] = depth * x[i - 1];
}

void FillPowers(float depth, out float[N_APPROXIMATION_COEFFICIENTS + 1] x) {
    x[0] = 1.0;
    for (int i = 1; i < N_APPROXIMATION_COEFFICIENTS + 1; i++)
        x[i] = depth * x[i - 1];
}

// Legendre polynomial coefficients
const float pcoeffs[MAX_ORDER_P * MAX_ORDER_P] = float[MAX_ORDER_P * MAX_ORDER_P](
     1,    0,      0,      0,      0,
    -1,    2,      0,      0,      0,
     1,   -6,      6,      0,      0,
    -1,   12,    -30,     20,      0,
     1,  -20,     90,   -140,     70
);

// Integral of Legendre polynomial coefficients
const float qcoeffs[MAX_ORDER_P * MAX_ORDER_Q] = float[MAX_ORDER_P * MAX_ORDER_Q](
    0,    1,      0,      0,      0,     0,
    0,   -1,      1,      0,      0,     0,
    0,    1,     -3,      1,      0,     0,
    0,   -1,      6,    -10,      5,     0,
    0,    1,    -10,     30,    -35,    14
);

float P(int order, float[N_APPROXIMATION_COEFFICIENTS] x) {
    float res = 0.0;
    for (int i = 0; i < order + 1; i++) {
        res += pcoeffs[MAX_ORDER_P * order + i] * x[i];
    }
    return res;
}

float Q(int order, float[N_APPROXIMATION_COEFFICIENTS + 1] x) {
    float res = 0.0;
    for (int i = 1; i < order + 2; i++) {
        res += qcoeffs[MAX_ORDER_Q * order + i] * x[i];
    }
    return res;
}

void projectImportanceSum(uint idx) {
    abufferPixel p = uncompressPixelData(readPixelStorage(idx - 1));
    float x[N_APPROXIMATION_COEFFICIENTS];
    FillPowers(p.depth, x);

    for (int i = 0; i < N_APPROXIMATION_COEFFICIENTS; i++) {
        float val = 0.0;
        val = (2 * i + 1) * p.color.a * p.color.a * P(i, x);

        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        float currentVal = imageLoad(importanceSumCoeffs[0], coord).x;
        imageStore(importanceSumCoeffs[0], coord, vec4(currentVal + val));
    }
}

void projectOpticalDepth(uint idx) {
    abufferPixel p = uncompressPixelData(readPixelStorage(idx - 1));
    float x[N_APPROXIMATION_COEFFICIENTS];
    FillPowers(p.depth, x);

    for (int i = 0; i < N_APPROXIMATION_COEFFICIENTS; i++) {
        float val = 0.0;
        val = (2 * i + 1) * -log(1 - p.color.a) * P(i, x);

        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        float currentVal = imageLoad(opticalDepthCoeffs, coord).x;
        imageStore(opticalDepthCoeffs, coord, vec4(currentVal + val));
    }
}

float approxImportanceSum(float depth) {
    float sum = 0.0;
    float x[N_APPROXIMATION_COEFFICIENTS + 1];
    FillPowers(depth, x);

    for (int i = 0; i < N_APPROXIMATION_COEFFICIENTS; i++) {
        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        float coeff = imageLoad(importanceSumCoeffs[int(smoothing)], coord).x;
        sum += coeff * Q(i, x);
    }

    return sum;
}

float approxOpticalDepth(float depth) {
    float sum = 0.0;
    float x[N_APPROXIMATION_COEFFICIENTS + 1];
    FillPowers(depth, x);

    for (int i = 0; i < N_APPROXIMATION_COEFFICIENTS; i++) {
        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        float coeff = imageLoad(opticalDepthCoeffs, coord).x;
        sum += coeff * Q(i, x);
    }

    return sum;
}
