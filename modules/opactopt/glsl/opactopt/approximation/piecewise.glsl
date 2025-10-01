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

#ifdef PIECEWISE
void project(layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, int N, float depth, float val) {
    if (depth < 0.0 || depth > 1.0) return;
    int bin = min(int(depth * N), N - 1);
    // pre integrate
    for (int i = bin; i <= N - 1; i++) {
        ivec3 coord = ivec3(gl_FragCoord.xy, i);
        optAdd(coeffTex, coord, val);
    }
}

float approximate(layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, int N, float depth) {
    int bin = min(int(depth * N), N - 1);
    ivec3 coord = ivec3(gl_FragCoord.xy, bin);
    ivec3 prevbincoord = ivec3(gl_FragCoord.xy, bin - 1);
    float binsum = optLoad(coeffTex, coord);
    float prevbinsum = 0.0;
    if (bin > 0) prevbinsum = optLoad(coeffTex, prevbincoord).x;
    return prevbinsum + fract(depth * N) * (binsum - prevbinsum);
}

float total(layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, int N) {
    return optLoad(coeffTex, ivec3(gl_FragCoord.xy, N - 1));
}
#endif
