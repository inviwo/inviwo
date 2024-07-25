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

layout(std430, binding = 9) buffer legendreCoefficientBuffer { float pcoeffs[]; };

void project(layout(size1x32) image2DArray coeffTex, int N, float depth, float val) {
    int coeffIdx = 0;

    for (int i = 0; i < N; i++) {
        float projVal = 0.0;
        float P = 0.0;
        float powxk = 1.0;
        for (int k = 0; k <= i; k++) {
            P += pcoeffs[coeffIdx] * powxk;
            powxk *= depth;
            coeffIdx++;
        }
        projVal = (2 * i + 1) * val * P;

        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        float currentVal = imageLoad(coeffTex, coord).x;
        imageStore(coeffTex, coord, vec4(currentVal + projVal));
    }
}

float approximate(layout(size1x32) image2DArray coeffTex, int N, float depth) {
    float sum = 0.0;
    int coeffIdx = 0;

    for (int i = 0; i < N; i++) {
        float Q = 0.0;
        float powxk = depth;
        for (int k = 0; k <= i; k++) {
            Q += pcoeffs[coeffIdx] * powxk / (k + 1);
            powxk *= depth;
            coeffIdx++;
        }
        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        float coeff = imageLoad(coeffTex, coord).x;
        sum += coeff * Q;
    }

    return sum;
}

float total(layout(size1x32) image2DArray coeffTex, int N) {
    return imageLoad(coeffTex, ivec3(gl_FragCoord.xy, 0)).x;
}
