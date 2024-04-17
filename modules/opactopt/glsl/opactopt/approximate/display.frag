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

#ifdef BACKGROUND_AVAILABLE
uniform ImageParameters bgParameters;
uniform sampler2D bgColor;
uniform sampler2D bgDepth;
uniform vec2 reciprocalDimensions;
#endif  // BACKGROUND_AVAILABLE

uniform bool smoothing;
uniform float znear;
uniform float zfar;

// Opacity optimisation settings
uniform float q;
uniform float r;
uniform float lambda;

uniform layout(size1x32) image2DArray importanceSumCoeffs[2]; // ping pong buffering for gaussian filtering
uniform layout(size1x32) image2DArray opticalDepthCoeffs;

// Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

// Input interpolated fragment position
smooth in vec4 fragPos;

#include "opactopt/approximate/filter.glsl"
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

// Computes only the number of fragments
int getFragmentCount(uint pixelIdx);

// Keeps only closest fragment
vec4 resolveClosest(uint idx);

// Resolve A-Buffer and blend sorted fragments
void main() {
    ivec2 coords = ivec2(gl_FragCoord.xy);

    if (coords.x < 0 || coords.y < 0 || coords.x >= AbufferParams.screenWidth ||
        coords.y >= AbufferParams.screenHeight) {
        discard;
    }

    uint pixelIdx = getPixelLink(coords);

    if (pixelIdx > 0) {
#if ABUFFER_DISPNUMFRAGMENTS == 1
        FragData0 = vec4(getFragmentCount(pixelIdx) / float(ABUFFER_SIZE));
        PickingData = vec4(0.0, 0.0, 0.0, 1.0);
#elif ABUFFER_RESOLVE_USE_SORTING == 0
        // If we only want the closest fragment
        vec4 p = resolveClosest(pixelIdx);
        FragData0 = uncompressPixelData(p).color;
        PickingData = vec4(0.0, 0.0, 0.0, 1.0);
        gl_FragDepth = p.depth;
#else
        float backgroundDepth = 1.0;
        lineariseDepths(pixelIdx);

        // Clear coefficient buffers
        for (int i = 0; i < N_APPROXIMATION_COEFFICIENTS; i++) {
            imageStore(importanceSumCoeffs[0], ivec3(coords, i), vec4(0.0));
            imageStore(importanceSumCoeffs[1], ivec3(coords, i), vec4(0.0));
            imageStore(opticalDepthCoeffs, ivec3(coords, i), vec4(0.0));
        }

        // Perform opacity optimisation and compositing
        uint idx = pixelIdx;

        // Project importance sum coefficients
        while (idx != 0) {
            projectImportanceSum(idx);
            memoryBarrierImage();
            idx = floatBitsToUint(readPixelStorage(idx - 1).x);
        }

        if (smoothing)
            filterImportanceSum();

        // Optimise opacities and project optical depth coefficients
        idx = pixelIdx;
        float totalImportanceSum = imageLoad(importanceSumCoeffs[int(smoothing)], ivec3(coords, 0)).x;
        while (idx != 0) {
            abufferPixel pixel = uncompressPixelData(readPixelStorage(idx - 1));
            vec4 c = pixel.color;
            float importanceSq = c.a * c.a;
            float importanceAtDepth = approxImportanceSum(pixel.depth) + 0.5 * importanceSq; // correct for importance sum approximation at discontinuity

            float alpha = clamp(1 /
                            (1 + pow(1 - c.a, 2 * lambda)
                            * (r * (importanceAtDepth - importanceSq)
                            + q * (totalImportanceSum - importanceAtDepth))),
                            0.0, 0.9999); // set pixel alpha using opacity optimisation
            pixel.color.a = alpha;
            writePixelStorage(idx - 1, compressPixelData(pixel)); // replace importance g_i in alpha channel with actual alpha

            projectOpticalDepth(idx);
            memoryBarrierImage();
            idx = pixel.previous;
        }

        // Composite
        vec3 numerator = vec3(0);
        float denominator = 0.0;
        idx = pixelIdx;
        float totalOpticalDepth = imageLoad(opticalDepthCoeffs, ivec3(coords, 0)).x;
        float minDepth = backgroundDepth;

        while (idx != 0) {
            abufferPixel pixel = uncompressPixelData(readPixelStorage(idx - 1));
            minDepth = min(pixel.depth, minDepth);
            vec4 c = pixel.color;
            float opticalDepth = approxOpticalDepth(pixel.depth) - 0.5 * log(1 - c.a); // correct for optical depth approximation at discontinuity

            float weight = c.a / (1 - c.a) * exp(-opticalDepth);
            numerator += c.rgb * weight;
            denominator += weight;
            idx = pixel.previous;
        }

        vec4 color = vec4(0);
        color.rgb = numerator / denominator;
        color.a = 1.0 - exp(-totalOpticalDepth);
        gl_FragDepth = minDepth;

        FragData0 = color;
        PickingData = vec4(0.0, 0.0, 0.0, 1.0);
#endif

    } else {  // no pixel found
#if ABUFFER_DISPNUMFRAGMENTS == 0
        // If no fragment, write nothing
        discard;
#else
        FragData0 = vec4(0.0f);
        PickingData = vec4(0.0, 0.0, 0.0, 1.0);
#endif
    }
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

int getFragmentCount(uint pixelIdx) {
    int counter = 0;
    while (pixelIdx != 0 && counter < ABUFFER_SIZE) {
        vec4 val = readPixelStorage(pixelIdx - 1);
        counter++;
        pixelIdx = floatBitsToUint(val.x);
    }
    return counter;
}

vec4 resolveClosest(uint pixelIdx) {

    // Search smallest z
    vec4 minFrag = vec4(0.0f, 1000000.0f, 1.0f, uintBitsToFloat(1024 * 1023));
    int ip = 0;

    while (pixelIdx != 0 && ip < ABUFFER_SIZE) {
        vec4 val = readPixelStorage(pixelIdx - 1);

        if (val.y < minFrag.y) {
            minFrag = val;
        }

        pixelIdx = floatBitsToUint(val.x);

        ip++;
    }
    // Output final color for the frame buffer
    return minFrag;
}
