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

#ifndef IVW_OPACTOPT_COMMON
#define IVW_OPACTOPT_COMMON

#define PI 3.1415926535897932384626433832795
#define TWOPI 6.283185307179586476925286766559

#ifdef COEFF_TEX_FIXED_POINT_FACTOR
    #define IMAGE_LAYOUT r32i
    #define IMAGE_UNIT iimage2DArray
#else
    #define IMAGE_LAYOUT size1x32
    #define IMAGE_UNIT image2DArray
#endif

void optAdd(layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, ivec3 coord, float projVal) {
#if defined(COEFF_TEX_FIXED_POINT_FACTOR)
    imageAtomicAdd(coeffTex, coord, int(projVal * COEFF_TEX_FIXED_POINT_FACTOR));
#elif defined(COEFF_TEX_ATOMIC_FLOAT)
    imageAtomicAdd(coeffTex, coord, projVal);
#else
    float currVal = imageLoad(coeffTex, coord).x;
    imageStore(coeffTex, coord, vec4(currVal + projVal));
#endif
}

float optLoad(readonly layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, ivec3 coord) {
#ifdef COEFF_TEX_FIXED_POINT_FACTOR
    return float(imageLoad(coeffTex, coord).x) / COEFF_TEX_FIXED_POINT_FACTOR;
#else
    return imageLoad(coeffTex, coord).x;
#endif
}

void optClear(writeonly layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, ivec3 coord) {
#ifdef COEFF_TEX_FIXED_POINT_FACTOR
    imageStore(coeffTex, coord, ivec4(0));
#else
    imageStore(coeffTex, coord, vec4(0));
#endif
}

#endif
