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

/**
 * Resolves the neighborhood of a fragment and sets the initial conditions for the silhouettes+halos
 */

// this is important for the occlusion query
layout(early_fragment_tests) in;
#include "illustration/illustrationbuffer.glsl"

// Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;
// Input interpolated fragment position
smooth in vec4 fragPos;

layout(std430, binding = 0) buffer surfaceInfoBufferIn {
    vec2 surfaceInfoIn[];  // depth + gradient
};
layout(std430, binding = 1) buffer neighborBufferOut {
    ivec4 neighborsOut[];  // neighbors
};
layout(std430, binding = 2) buffer smoothingBufferOut {
    vec2 smoothingOut[];  // beta + gamma
};

// Temporal buffer for storing the depths to avoid buffer access
float currentDepths[ABUFFER_SIZE];
float neighborDepths[ABUFFER_SIZE];

const ivec2 neighborPositions[4] = {ivec2(-1, 0), ivec2(+1, 0), ivec2(0, -1), ivec2(0, +1)};

void main(void) {
    const ivec2 coords = ivec2(gl_FragCoord.xy);

    const ivec2 index = getStartAndCount(coords); // start index, fragment count 
    if (index.y == 0) discard;

    // Load fragments of the current pixel
    // init the flags for the initial smoothing values
    for (int i = 0; i < index.y; ++i) {
        currentDepths[i] = surfaceInfoIn[index.x + i].x; 
        smoothingOut[index.x + i] = vec2(0);
    }

    // now process the four neighbors
    for (int n = 0; n < 4; ++n) {
        // load that neighbor
        const ivec2 neighborIndex = getStartAndCount(coords + neighborPositions[n]);
        if (neighborIndex.y == 0) {
            // no neighbors at all, all current fragments are silhouette
            for (int i = 0; i < index.y; ++i) {
                neighborsOut[index.x + i][n] = -1;
                smoothingOut[index.x + i].x = 1;
            }
            continue;
        }
        // load the neighbors
        for (int i = 0; i < neighborIndex.y; ++i) {
            neighborDepths[i] = surfaceInfoIn[neighborIndex.x + i].x;
        }
        
        // front propagation
        int a = 0;
        int b = 0;
        while (a < index.y && b < neighborIndex.y) {
            float eNeighbor = abs(currentDepths[a] - neighborDepths[b]);

            if (b + 1 < neighborIndex.y) {
                const float e = abs(currentDepths[a] - neighborDepths[b + 1]);
                if (e < eNeighbor) {
                    // next neighbor is better
                    b++;
                    eNeighbor = e;
                    continue;
                }
            }
            if (a + 1 < index.y) {
                const float e = abs(currentDepths[a + 1] - neighborDepths[b]);
                if (e < eNeighbor) {
                    // next neighbor wants to connect to next current -> current is
                    // silhouette
                    neighborsOut[index.x + a][n] = -1;
                    smoothingOut[index.x + a].x = 1;
                    a++;
                    eNeighbor = e;
                    continue;
                }
            }
            // this is the best connection
            neighborsOut[index.x + a][n] = neighborIndex.x + b;
            if (b > a) smoothingOut[index.x + a].y = 1;  // halo
            // advance to next pair
            a++;
            b++;
            
        }
    }

    discard;
}

