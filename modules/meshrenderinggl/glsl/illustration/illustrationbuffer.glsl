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
 * Extension to Fragment Lists to support post-processing effects on the fragments.
 * This follows the ideas of the paper "Smart Transparency for Illustrative Visualization of Complex
 * Flow Surfaces"
 */

#ifndef ILLUSTRATIONBUFFER_HGLSL
#define ILLUSTRATIONBUFFER_HGLSL

#include "oit/commons.glsl"

// screen size
uniform ivec2 screenSize;

// Buffers+Images

// Head pointer to the entry in the list
coherent uniform layout(size1x32) uimage2D illustrationBufferIdxImg;
// Number of entries in the list
coherent uniform layout(size1x32) uimage2D illustrationBufferCountImg;

// Returns count and start of the lists at the given position
// Keeps image boundary in mind
ivec2 getStartAndCount(ivec2 pos) {
    if (pos.x >= 0 && pos.y >= 0 && pos.x < screenSize.x && pos.y < screenSize.y) {
        uint count = imageLoad(illustrationBufferCountImg, pos).x;
        if (count > 0) {
            uint start = imageLoad(illustrationBufferIdxImg, pos).x;
            return ivec2(start, count);
        }
    }
    return ivec2(0, 0);
}


/*

//data stored per fragment, 12*4=48 byte
struct FragmentData
{
    //the fragment depth
    float depth;
    //the depth gradient, needed to determine neighbors
    float depthGradient;
    //the alpha value [0,1] + 3*b_alpha (bitfield as in the paper)
    float alpha;
    //the color in 10/10/10 rgb
    uint colors;
    //the neighbors: -x, +x, -y, +y; -1 if not existing
    ivec4 neighbors;
    //silhouette highlight field (beta) + 3*b_beta, as in the paper
    float silhouetteHighlight;
    //halo highlight field (gamma) + 3*b_gamma, as in the paper
    float haloHighlight;
    //index of this fragment in the list, saves some memory access
    //TODO: only 8 bit are used, use the remaining bits for something else
    int index;
    //unused yet, TODO: use for normal comparison
    int dummy1;
};

layout(std430, binding=4) buffer illustrationBufferStorageIn
{
    FragmentData illustrationDataIn[];
};
layout(std430, binding=5) buffer illustrationBufferStorageOut
{
    FragmentData illustrationDataOut[];
};

*/

#endif