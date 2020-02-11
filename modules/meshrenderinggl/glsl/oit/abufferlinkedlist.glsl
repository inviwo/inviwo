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
 * Fast Single-pass A-Buffer using OpenGL 4.0 V2.0
 * Copyright Cyril Crassin, July 2010
 *
 * Modified by Sebastian Weiss, 2017
 **/
#ifndef ABUFFERLINKEDLIST_HGLSL
#define ABUFFERLINKEDLIST_HGLSL

#include "oit/commons.glsl"

// Uniforms changed from the C++ side
struct AbufferParameters {
    int screenWidth;
    int screenHeight;
    vec4 backgroundColor;
    uint storageSize;
};
uniform AbufferParameters AbufferParams;

// Macros (maybe) changed from the C++ side

#define ABUFFER_USE_TEXTURES 1
#define ABUFFER_SHAREDPOOL_USE_TEXTURES 1

#if ABUFFER_USE_TEXTURES
// A-Buffer fragments storage
// coherent uniform layout(binding=5, size1x32) uimage2D abufferIdxImg;
coherent uniform layout(size1x32) uimage2D abufferIdxImg;
#else
#error Buffer access not supported
// coherent uniform uint *d_abufferPageIdx;
// coherent uniform uint *d_abufferFragCount;
// coherent uniform uint *d_semaphore;
#endif

// TODO: remove hardcoded binding point
layout(binding = 6, offset = 0) uniform atomic_uint abufferCounter;
layout(std430, binding = 7) buffer abufferStorage {
    // x: previous chain element, uint, +1
    // y: depth, float
    // z: alpha, float
    // w: color, 10/10/10 rgb
    vec4 abufferPixelData[];
};
struct abufferPixel {
    uint previous;
    float depth;
    vec4 color;
};

// Fragment linked list

#if ABUFFER_USE_TEXTURES

uint setPixelLink(ivec2 coords, uint val) {
    return imageAtomicExchange(abufferIdxImg, coords, val);
}

uint getPixelLink(ivec2 coords) { return uint(imageLoad(abufferIdxImg, coords).x); }

#else

#error Buffer access not supported

#endif

// Pixel data storage

uint dataCounterAtomicInc() { return atomicCounterIncrement(abufferCounter); }

vec4 readPixelStorage(uint idx) { return abufferPixelData[int(idx)]; }

void writePixelStorage(uint idx, vec4 value) { abufferPixelData[int(idx)] = value; }

abufferPixel uncompressPixelData(vec4 data) {
    abufferPixel p;
    p.previous = floatBitsToUint(data.x);
    p.depth = data.y;
    p.color.a = data.z;
    p.color.rgb = uncompressColor(floatBitsToUint(data.w));
    return p;
}

vec4 compressPixelData(abufferPixel p) {
    vec4 data;
    data.x = uintBitsToFloat(p.previous);
    data.y = p.depth;
    p.color = clamp(p.color, 0, 1);
    data.z = p.color.a;
    data.w = uintBitsToFloat(compressColor(p.color.rgb));
    return data;
}

// Rendering function

// The central function for the user-code
uint abufferRender(ivec2 coords, float depth, vec4 color) {
    // coords.x=0; coords.y=0;
    // reserve space for pixel
    uint pixelIdx = dataCounterAtomicInc();
    if (pixelIdx >= AbufferParams.storageSize) {
        // we are out of space
        return uint(-1);
    }
    // write index
    uint prevIdx = setPixelLink(coords, pixelIdx + 1);
    // assemble and write pixel
    abufferPixel p;
    p.previous = prevIdx;
    p.depth = depth;
    p.color = color;
    writePixelStorage(pixelIdx, compressPixelData(p));
    return pixelIdx;
}

#endif  // ABUFFERLINKEDLIST_HGLSL