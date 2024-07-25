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

void project(layout(size1x32) image2DArray coeffTex, int N, float depth, float val) {
    if (depth < 0.0 || depth > 1.0) return;
    int bin = min(int(depth * N), N - 1);
    ivec3 coord = ivec3(gl_FragCoord.xy, bin);
    float currVal = imageLoad(coeffTex, coord).x;
    imageStore(coeffTex, coord, vec4(currVal + val));
}

float approximate(layout(size1x32) image2DArray coeffTex, int N, float depth) {
    float sum = 0.0;
    int i = 0;
    while (depth * N >= i + 1 && i < N - 1) {
        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        float coeff = imageLoad(coeffTex, coord).x;
        sum += coeff;
        i++;
    }
    ivec3 coord = ivec3(gl_FragCoord.xy, i);
    float coeff = imageLoad(coeffTex, coord).x;
    sum += fract(depth * N) * coeff;

    return sum;
}

float total(layout(size1x32) image2DArray coeffTex, int N) {
    float sum = 0.0;
    for (int i = 0; i < N; i++) sum += imageLoad(coeffTex, ivec3(gl_FragCoord.xy, i)).x;
    return sum;
}
