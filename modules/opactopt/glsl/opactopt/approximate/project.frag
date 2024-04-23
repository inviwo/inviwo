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

/**
 * Fast Single-pass A-Buffer using OpenGL 4.0 V2.0
 * Copyright Cyril Crassin, July 2010
 *
 * Edited by the Inviwo Foundation.
 * Edited by Aritra Bhakat
 **/

// need extensions added from C++
// GL_NV_gpu_shader5, GL_EXT_shader_image_load_store, GL_NV_shader_buffer_load,
// GL_EXT_bindable_uniform

#include "oit/abufferlinkedlist.glsl"
#include "utils/structs.glsl"

// How should the stuff be rendered? (Debugging options)
#define ABUFFER_DISPNUMFRAGMENTS 0
#define ABUFFER_RESOLVE_USE_SORTING 1

uniform float znear;
uniform float zfar;

coherent uniform layout(size1x32) image2DArray importanceSumCoeffs[2]; // double buffering for gaussian filtering
coherent uniform layout(size1x32) image2DArray opticalDepthCoeffs;

// Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

// Input interpolated fragment position
smooth in vec4 fragPos;

#ifdef FOURIER
    #include "opactopt/approximate/fourier.glsl"
#endif
#ifdef LEGENDRE
    #include "opactopt/approximate/legendre.glsl"
#endif
#ifdef CHEBYSHEV
    #include "opactopt/approximate/chebyshev.glsl"
#endif
#ifdef PIECEWISE
    #include "opactopt/approximate/piecewise.glsl"
#endif

// Converts depths from screen space to clip space
void lineariseDepths(uint pixelIdx);

// Resolve A-Buffer and blend sorted fragments
void main() {
    ivec2 coords = ivec2(gl_FragCoord.xy);

    if (coords.x < 0 || coords.y < 0 || coords.x >= AbufferParams.screenWidth ||
        coords.y >= AbufferParams.screenHeight) {
        discard;
    }

    uint pixelIdx = getPixelLink(coords);
    if (pixelIdx > 0) {
        lineariseDepths(pixelIdx);
        uint idx = pixelIdx;

        // Project importance sum coefficients
        while (idx != 0) {
            projectImportanceSum(idx);
            memoryBarrierImage();
            idx = floatBitsToUint(readPixelStorage(idx - 1).x);
        }
    }

    discard;
}

void lineariseDepths(uint pixelIdx) {
    int counter = 0;
    while (pixelIdx != 0 && counter < ABUFFER_SIZE) {
        vec4 val = readPixelStorage(pixelIdx - 1);
        float z_w = znear * zfar / (zfar + val.y * (znear - zfar)); // world space depth
        val.y = (z_w - znear) / (zfar - znear); // linear normalised depth;
        writePixelStorage(pixelIdx - 1, val); 
        counter++;
        pixelIdx = floatBitsToUint(val.x);
    }
}
