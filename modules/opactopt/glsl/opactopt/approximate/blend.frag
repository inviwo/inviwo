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
#ifdef USE_EXACT_BLENDING
#include "oit/sort.glsl"
#endif

#ifdef DEBUG
uniform ivec2 debugCoords;

layout(std430, binding = 10) buffer debugCoeffs {
    float debugImportanceCoeffs[N_IMPORTANCE_SUM_COEFFICIENTS];
    float debugOpticalCoeffs[N_OPTICAL_DEPTH_COEFFICIENTS];
};

struct FragmentInfo {
    float depth;
    float importance;
};

layout(std430, binding = 11) buffer debugFragments {
    int nFragments;
    FragmentInfo debugFrags[];
};

layout(std430, binding = 12) buffer debugApproxVals {
    int nApproxSamples;
    float approxSamples[];
};
#endif

uniform ivec2 screenSize;

// Opacity optimisation settings
uniform float q;
uniform float r;
uniform float lambda;

uniform layout(size1x32) image2DArray importanceSumCoeffs[2]; // double buffering for gaussian filtering
#ifndef USE_EXACT_BLENDING
uniform layout(size1x32) image2DArray opticalDepthCoeffs;
#endif

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
#ifdef PIECEWISE
    #include "opactopt/approximate/piecewise.glsl"
#endif
#ifdef POWER_MOMENTS
    #include "opactopt/approximate/powermoments.glsl"
#endif
#ifdef TRIG_MOMENTS
    #include "opactopt/approximate/trigmoments.glsl"
#endif

// Resolve A-Buffer and blend sorted fragments
void main() {
    ivec2 coords = ivec2(gl_FragCoord.xy);

    if (coords.x < 0 || coords.y < 0 || coords.x >= screenSize.x ||
        coords.y >= screenSize.y) {
        discard;
    }

    uint pixelIdx = getPixelLink(coords);

    if (pixelIdx > 0) {
        // Perform opacity optimisation and compositing
        uint idx = pixelIdx;
        vec4 color = vec4(0);

        // Optimise opacities and project optical depth coefficients
        float gtot = total(importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS);

#ifdef USE_EXACT_BLENDING
        // front-to-back compositing
        uint lastPtr = 0;
        vec4 nextFragment = selectionSortNext(pixelIdx, 0.0, lastPtr);
        abufferPixel unpackedFragment = uncompressPixelData(nextFragment);
        
        while (unpackedFragment.depth >= 0 && unpackedFragment.depth <= 1.0) {
            vec4 c = unpackedFragment.color;

            float gi = c.a;
            float gisq = gi * gi;
            float Gd = approximate(importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS, unpackedFragment.depth);
            Gd += 0.5 * gisq; // correct for importance sum approximation at discontinuity
            float alpha = clamp(1 /
                            (1 + pow(1 - gi, 2 * lambda)
                            * (r * max(0, Gd - gisq)
                            + q * max(0, gtot - Gd))),
                            0.0, 0.9999); // set pixel alpha using opacity optimisation

            color.rgb += (1 - color.a) * alpha * c.rgb;
            color.a += (1 - color.a) * alpha;

            nextFragment = selectionSortNext(pixelIdx, unpackedFragment.depth, lastPtr);
            unpackedFragment = uncompressPixelData(nextFragment);
        }
#else
        int counter = 0;
        while (idx != 0 && counter < ABUFFER_SIZE) {
            vec4 data = readPixelStorage(idx - 1);
            abufferPixel pixel = uncompressPixelData(data);

            #ifdef DEBUG
            if (coords == debugCoords) {
                FragmentInfo fi;
                fi.depth = pixel.depth;
                fi.importance = pixel.color.a;
                debugFrags[nFragments] = fi;
                nFragments++;
            }
            #endif

            vec4 c = pixel.color;
            float gi = c.a;
            float gisq = gi * gi;
            float Gd = approximate(importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS, pixel.depth);
            Gd += 0.5 * gisq; // correct for importance sum approximation at discontinuity
            float alpha = clamp(1 /
                            (1 + pow(1 - gi, 2 * lambda)
                            * (r * max(0, Gd - gisq)
                            + q * max(0, gtot - Gd))),
                            0.0, 0.9999); // set pixel alpha using opacity optimisation
            project(opticalDepthCoeffs, N_OPTICAL_DEPTH_COEFFICIENTS, pixel.depth, -log(1 - alpha));
            idx = pixel.previous;
            counter++;
        }

        // Composite
        vec3 numerator = vec3(0);
        float denominator = 0.0;
        idx = pixelIdx;
        float tauall = total(opticalDepthCoeffs, N_OPTICAL_DEPTH_COEFFICIENTS);

        counter = 0;
        while (idx != 0 && counter < ABUFFER_SIZE) {
            abufferPixel pixel = uncompressPixelData(readPixelStorage(idx - 1));

            vec4 c = pixel.color;
            float gi = c.a;
            float gisq = gi * gi;
            float Gd = approximate(importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS, pixel.depth);
            Gd += 0.5 * gisq; // correct for importance sum approximation at discontinuity
            float alpha = clamp(1 /
                            (1 + pow(1 - gi, 2 * lambda)
                            * (r * max(0, Gd - gisq)
                            + q * max(0, gtot - Gd))),
                            0.0, 0.9999); // set pixel alpha using opacity optimisation

            float taud = approximate(opticalDepthCoeffs, N_OPTICAL_DEPTH_COEFFICIENTS, pixel.depth); 
            float weight = alpha / sqrt(1 - alpha) * exp(-taud); // correct for optical depth approximation at discontinuity
            numerator += weight * c.rgb;
            denominator += weight;
            idx = pixel.previous;
            counter++;
        }

        #ifdef DEBUG
        if (coords == debugCoords) {
            for (int i = 0; i < N_IMPORTANCE_SUM_COEFFICIENTS; i++)
                debugImportanceCoeffs[i] = imageLoad(importanceSumCoeffs[0], ivec3(coords, i)).x;
            for (int i = 0; i < N_OPTICAL_DEPTH_COEFFICIENTS; i++)
                debugOpticalCoeffs[i] = imageLoad(opticalDepthCoeffs, ivec3(coords, i)).x;

            for (int i = 0; i < nApproxSamples; i++) {
                float sampleDepth = float(i + 1) / float(nApproxSamples);
                approxSamples[i] = approximate(importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS, sampleDepth);
                approxSamples[nApproxSamples + i] = approximate(opticalDepthCoeffs, N_OPTICAL_DEPTH_COEFFICIENTS, sampleDepth);
            }
        }
        #endif

        if (denominator != 0.0)
            color.a = 1 - exp(-tauall);

        color.rgb = numerator;
        #ifdef NORMALISE
            color.rgb /= denominator;
        #else
            color.rgb /= color.a; // no alpha for non normalised blending
        #endif

#endif // !ifdef USE_EXACT_BLENDING

//        color = vec4(abs(imageLoad(opticalDepthCoeffs, ivec3(ivec2(gl_FragCoord.xy), 3)).x), 0, 0, 1);
        FragData0 = color;
        PickingData = vec4(0.0, 0.0, 0.0, 1.0);
    } else {  // no pixel found
        FragData0 = vec4(0);
        PickingData = vec4(0.0, 0.0, 0.0, 0.0);
    }
}
