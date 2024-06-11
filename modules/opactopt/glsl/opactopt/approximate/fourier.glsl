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

#define PI 3.141592653
#define TWOPI 6.283185307

void project(layout(size1x32) image2DArray coeffTex, int N, float depth, float val) {
    int k = 0;
    for (int i = 0; i < N; i++) {
        float projVal = 0.0;
        if (i == 0) {
            projVal = val;
        } else if (i % 2 == 0) {
            projVal = val * cos(TWOPI * k * depth);
        } else {
            projVal = val * sin(TWOPI * k * depth);
        }
        if (i % 2 == 0) k++;

        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        float currVal = imageLoad(coeffTex, coord).x;
        imageStore(coeffTex, coord, vec4(currVal + projVal));
    }
}

float approximate(layout(size1x32) image2DArray coeffTex, int N, float depth) {
    float sum = 0.0;
    int k = 0;
    for (int i = 0; i < N; i++) {
        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        float coeff = imageLoad(coeffTex, coord).x;
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

float total(layout(size1x32) image2DArray coeffTex, int N) {
    return imageLoad(coeffTex, ivec3(gl_FragCoord.xy, 0)).x;
}
