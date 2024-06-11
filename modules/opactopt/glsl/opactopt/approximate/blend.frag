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
#include "oit/sort.glsl"
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

// Opacity optimisation settings
uniform float q;
uniform float r;
uniform float lambda;

uniform layout(size1x32) image2DArray importanceSumCoeffs[2]; // double buffering for gaussian filtering
uniform layout(size1x32) image2DArray opticalDepthCoeffs;

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
        // Perform opacity optimisation and compositing
        uint idx = pixelIdx;

        // Optimise opacities and project optical depth coefficients
        float gtot = total(importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS);
        idx = pixelIdx;
        while (idx != 0) {
            vec4 data = readPixelStorage(idx - 1);
            abufferPixel pixel = uncompressPixelData(data);
            vec4 c = pixel.color;
            float gi = clamp(c.a, 0.001, 0.999);
            float gisq = gi * gi;
            float Gd = approximate(importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS, pixel.depth) + 0.5 * gisq; // correct for importance sum approximation at discontinuity
            float alpha = clamp(1 /
                            (1 + pow(1 - gi, 2 * lambda)
                            * (r * (Gd - gisq)
                            + q * (gtot - Gd))),
                            0.0, 0.9999); // set pixel alpha using opacity optimisation
            pixel.color.a = alpha;
//            writePixelStorage(idx - 1, compressPixelData(pixel)); // replace importance g_i in alpha channel with actual alpha

            project(opticalDepthCoeffs, N_OPTICAL_DEPTH_COEFFICIENTS, pixel.depth, -log(1 - alpha));
            idx = pixel.previous;
        }

        // Composite
        vec3 numerator = vec3(0);
        float denominator = 0.0;
        idx = pixelIdx;
        float tauall = total(opticalDepthCoeffs, N_OPTICAL_DEPTH_COEFFICIENTS);

        while (idx != 0) {
            abufferPixel pixel = uncompressPixelData(readPixelStorage(idx - 1));
            vec4 c = pixel.color;
            float taud = approximate(opticalDepthCoeffs, N_OPTICAL_DEPTH_COEFFICIENTS, pixel.depth) - 0.5 * log(1 - c.a); // correct for optical depth approximation at discontinuity

            float weight = c.a / (1 - c.a) * exp(-taud);
            numerator += c.rgb * weight;
            denominator += weight;
            idx = pixel.previous;
        }

        vec4 color = vec4(0);
        color.rgb = numerator / denominator;
        color.a = 1.0 - exp(-tauall);

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
