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

#define PI      3.141592653
#define TWOPI   6.283185307

void projectImportanceSum(uint idx) {
    abufferPixel p = uncompressPixelData(readPixelStorage(idx - 1));
    float importanceSq = p.color.a * p.color.a;
    int k = 0;

    for (int i = 0; i < N_IMPORTANCE_SUM_COEFFICIENTS; i++) {
        float val = 0.0;

        if (i == 0) {
            val += importanceSq;
        } else if (i % 2 == 0) {
            val = importanceSq * cos(TWOPI * k * p.depth);
        } else {
            val = importanceSq * sin(TWOPI * k * p.depth);
        }
        if (i % 2 == 0) k++;

        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        float currentVal = imageLoad(importanceSumCoeffs[0], coord).x;
        imageStore(importanceSumCoeffs[0], coord, vec4(currentVal + val));
    }
}

void projectOpticalDepth(uint idx) {
    abufferPixel p = uncompressPixelData(readPixelStorage(idx - 1));
    float log1ma = log(1 - p.color.a);
    int k = 0;

    for (int i = 0; i < N_OPTICAL_DEPTH_COEFFICIENTS; i++) {
        float val = 0.0;

        if (i == 0) {
            val += -log1ma;
        } else if (i % 2 == 0) {
            val = -log1ma * cos(TWOPI * k * p.depth); 
        } else {
            val = -log1ma * sin(TWOPI * k * p.depth);
        }
        if (i % 2 == 0) k++;

        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        float currentVal = imageLoad(opticalDepthCoeffs, coord).x;
        imageStore(opticalDepthCoeffs, coord, vec4(currentVal + val));
    }
}

float approxImportanceSum(float depth) {
    float sum = 0.0;
    int k = 0;

    for (int i = 0; i < N_IMPORTANCE_SUM_COEFFICIENTS; i++) {
        ivec3 coord = ivec3(gl_FragCoord.xy, i);

        float coeff = imageLoad(importanceSumCoeffs[0], coord).x;
        if (i == 0) {
            sum += coeff * depth;
        } else if (i % 2 == 0) {
            sum += (coeff / (PI * k)) * sin(TWOPI * k * depth);
        } else {
            sum += (coeff / (PI * k)) * (1 - cos(TWOPI * k * depth));
        }
        if (i % 2 == 0) k++;
    }

    return sum;
}

float approxOpticalDepth(float depth) {
    float sum = 0.0;
    int k = 0;

    for (int i = 0; i < N_OPTICAL_DEPTH_COEFFICIENTS; i++) {
        ivec3 coord = ivec3(gl_FragCoord.xy, i);

        float coeff = imageLoad(opticalDepthCoeffs, coord).x;
        if (i == 0) {
            sum += coeff * depth;
        } else if (i % 2 == 0) {
            sum += (coeff / (PI * k)) * sin(TWOPI * k * depth);
        } else {
            sum += (coeff / (PI * k)) * (1 - cos(TWOPI * k * depth));
        }
        if (i % 2 == 0) k++;
    }

    return sum;
}
