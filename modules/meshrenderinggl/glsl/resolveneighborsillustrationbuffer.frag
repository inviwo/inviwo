/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include "illustrationbuffer.glsl"

// Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;
// Input interpolated fragment position
smooth in vec4 fragPos;

layout(std430, binding = 0) buffer surfaceInfoBufferIn {
    vec2 surfaceInfoIn[];  // depth+gradient
};
layout(std430, binding = 1) buffer neighborBufferOut { ivec4 neighborsOut[]; };
layout(std430, binding = 2) buffer smoothingBufferOut {
    vec2 smoothingOut[];  // beta + gamma
};

// Temporal buffer for storing the depths to avoid buffer access
float fragmentsCurrent[ABUFFER_SIZE];
float fragmentsNeighbor[ABUFFER_SIZE];

// Returns count and start of the lists at the given position
// Keeps image boundary in mind
ivec2 getCountAndStart(ivec2 pos);

const ivec2 neighborPositions[4] = ivec2[4](ivec2(-1, 0), ivec2(+1, 0), ivec2(0, -1), ivec2(0, +1));

void main(void) {
    ivec2 coords = ivec2(gl_FragCoord.xy);

    if (coords.x >= 0 && coords.y >= 0 && coords.x < screenSize.x && coords.y < screenSize.y) {

        int count = int(imageLoad(illustrationBufferCountImg, coords).x);
        if (count > 0) {
            int start = int(imageLoad(illustrationBufferIdxImg, coords).x);

            // Load fragments of the current pixel
            // init the flags for the initial smoothing values
            for (int i = 0; i < count; ++i) {
                fragmentsCurrent[i] = surfaceInfoIn[start + i].x;
                smoothingOut[start + i] = vec2(0);
            }

            // now process the four neighbors
            for (int n = 0; n < 4; ++n) {
                // load that neighbor
                ivec2 cb = getCountAndStart(coords + neighborPositions[n]);
                if (cb.x == 0) {
                    // no neighbors at all, all current fragments are silhouette
                    for (int i = 0; i < count; ++i) {
                        neighborsOut[start + i][n] = -1;
                        smoothingOut[start + i].x = 1;
                    }
                    continue;
                }
                // load the neighbors
                for (int i = 0; i < cb.x; ++i) {
                    fragmentsNeighbor[i] = surfaceInfoIn[cb.y + i].x;
                }
                // front propagation
                int a = 0;
                int b = 0;
                float eNeighbor = abs(fragmentsCurrent[0] - fragmentsNeighbor[0]);
                while (a < count && b < cb.x) {
                    float e;
                    if (b + 1 < cb.x) {
                        e = abs(fragmentsCurrent[a] - fragmentsNeighbor[b + 1]);
                        if (e < eNeighbor) {
                            // next neighbor is better
                            b++;
                            eNeighbor = e;
                            continue;
                        }
                    }
                    if (a + 1 < count) {
                        e = abs(fragmentsCurrent[a + 1] - fragmentsNeighbor[b]);
                        if (e < eNeighbor) {
                            // next neighbor wants to connect to next current -> current is
                            // silhouette
                            neighborsOut[start + a][n] = -1;
                            smoothingOut[start + a].x = 1;
                            a++;
                            eNeighbor = e;
                            continue;
                        }
                    }
                    // this is the best connection
                    neighborsOut[start + a][n] = b + cb.y;
                    if (b > a) smoothingOut[start + a].y = 1;  // halo
                    // advance to next pair
                    a++;
                    b++;
                    eNeighbor = abs(fragmentsCurrent[a] - fragmentsNeighbor[b]);
                }
            }
        }
    }
    discard;
}

ivec2 getCountAndStart(ivec2 pos) {
    if (pos.x >= 0 && pos.y >= 0 && pos.x < screenSize.x && pos.y < screenSize.y) {

        uint count = imageLoad(illustrationBufferCountImg, pos).x;
        if (count > 0) {
            uint start = imageLoad(illustrationBufferIdxImg, pos).x;
            return ivec2(count, start);
        }
    }
    return ivec2(0, 0);
}
