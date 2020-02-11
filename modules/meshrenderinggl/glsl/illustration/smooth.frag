/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

/*
 * Smoothes the silhouette and halo borders
 */

// this is important for the occlusion query
layout(early_fragment_tests) in;
#include "illustration/illustrationbuffer.glsl"

// Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;
// Input interpolated fragment position
smooth in vec4 fragPos;

layout(std430, binding = 0) buffer neighborBufferIn {
    ivec4 neighborsIn[];  // neighbors
};
layout(std430, binding = 1) buffer smoothingBufferIn {
    vec2 smoothingIn[];  // beta + gamma
};
layout(std430, binding = 2) buffer smoothingBufferOut {
    vec2 smoothingOut[];  // beta + gamma
};

// diffusion coefficient
uniform float lambdaBeta = 0.4;   // Edge
uniform float lambdaGamma = 0.1;  // Halo

void main(void) {
    ivec2 coords = ivec2(gl_FragCoord.xy);

    const ivec2 index = getStartAndCount(coords);  // start index, fragment count
    if (index.y == 0) discard;

    const vec2 lambda = vec2(lambdaBeta, lambdaGamma);

    for (int i = 0; i < index.y; ++i) {
        const ivec4 neighbors = neighborsIn[index.x + i];
        vec2 smoothing = smoothingIn[index.x + i];

        for (int j = 0; j < 4; ++j) {
            if (neighbors[j] >= 0) smoothing = max(smoothing, smoothingIn[neighbors[j]] * (1 - lambda));
        }

        smoothingOut[index.x + i] = smoothing;
    }
    discard;
}