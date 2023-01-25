/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#include "oit/mycommons.glsl"

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

struct abufferMeshPixel {
    uint previous;
    float depth;
    vec4 color;
};

struct abufferVolumePixel {
    uint previous;
    float depth;
    vec3 position;
    uint id;
};

/*
MESH:
data.x: uint prev;
data.y: float depth;
data.z: float alpha;
data.w: float RGB (10bits each) + 1 bit unused + 1 bit type

VOLUME:
data.x: uint prev;
data.y: float depth;
data.z: R(20bits) + G(12bits)
data.w: G(8bits) + B(20bits) + id(3bits) + type(1bit)

*/
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

// MESH
abufferMeshPixel uncompressMeshPixelData(vec4 data) {
    abufferMeshPixel p;
    p.previous = floatBitsToUint(data.x);
    p.depth = data.y;
    p.color.a = data.z;
    p.color.rgb = uncompressColor10bits(floatBitsToUint(data.w));
    return p;
}

vec4 compressMeshPixelData(abufferMeshPixel p) { // compress mesh data
    vec4 data;
    data.x = uintBitsToFloat(p.previous);
    data.y = p.depth;
    p.color = clamp(p.color, 0, 1);
    data.z = p.color.a;
    data.w = uintBitsToFloat(compressColor10bits(p.color.rgb)); // 10bit rgb + type
    //data.w |= 0x0;
    return data;
}

// VOLUME
abufferVolumePixel uncompressVolumePixelData(vec4 data) {
    abufferVolumePixel p;
    p.previous = floatBitsToUint(data.x);
    p.depth = data.y;
    uvec2 res = uvec2(floatBitsToUint(data.z), floatBitsToUint(data.w));
    p.position.rgb = uncompressColor20bits(res);
    p.id = (res.y >> 1) & 0x7;
   return p;
}

vec4 compressVolumePixelData(abufferVolumePixel p) { // compress volume data
    vec4 data;
    data.x = uintBitsToFloat(p.previous);
    data.y = p.depth;
    p.position = clamp(p.position, 0, 1);
    uvec2 res = compressColor20bits(p.position.xyz);
    data.z = uintBitsToFloat(res.x);
    data.w = uintBitsToFloat(res.y);
    //    data.w |= (p.id & 0x7) << 1; // Sets id
    //    data.w |= 0x1; // Sets type
    res.y |= (p.id & 0x7u) << 1u;
    res.y |= 0x1u;
    data.w = uintBitsToFloat(res.y);
    return data;
}

// Done
int getPixelDataType(vec4 data) {
    int iData = floatBitsToInt(data.w);
    return (iData & 0x01);
}
// Rendering function

// The central function for the user-code
uint abufferMeshRender(ivec2 coords, float depth, vec4 color) {
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
    abufferMeshPixel p;
    p.previous = prevIdx;
    p.depth = depth;
    p.color = color;
    writePixelStorage(pixelIdx, compressMeshPixelData(p));
    return pixelIdx;
}

// The central function for the user-code
uint abufferVolumeRender(ivec2 coords, float depth, vec3 position, uint volumeId) {
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
    abufferVolumePixel p;
    p.previous = prevIdx;
    p.depth = depth;
    p.id = volumeId;
    p.position = position;
    
    writePixelStorage(pixelIdx, compressVolumePixelData(p));
    return pixelIdx;
}

#endif  // ABUFFERLINKEDLIST_HGLSL