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
 **/

// need extensions added from C++
// GL_NV_gpu_shader5, GL_EXT_shader_image_load_store, GL_NV_shader_buffer_load,
// GL_EXT_bindable_uniform

#include "oit/abufferlinkedlist.glsl"
#include "oit/sort.glsl"
#include "utils/structs.glsl"
#include "utils/depth.glsl"

// How should the stuff be rendered? (Debugging options)
#define ABUFFER_DISPNUMFRAGMENTS 0
#define ABUFFER_RESOLVE_USE_SORTING 1

#ifdef BACKGROUND_AVAILABLE
uniform ImageParameters bgParameters;
uniform sampler2D bgColor;
uniform sampler2D bgDepth;
#endif  // BACKGROUND_AVAILABLE
uniform vec2 reciprocalDimensions;

// Optional importance volume
uniform sampler3D importanceVolume;
uniform VolumeParameters importanceVolumeParameters;
uniform CameraParameters camera;

// Opacity optimisation settings
uniform float q;
uniform float r;
uniform float lambda;

// Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

// Input interpolated fragment position
smooth in vec4 fragPos;

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
        gl_FragDepth = uncompressPixelData(p).depth;
#else
        float backgroundDepth = 1.0;
        vec2 texCoord = (gl_FragCoord.xy + 0.5) * reciprocalDimensions;
#ifdef BACKGROUND_AVAILABLE
        // Assume the camera used to render the background has the same near and far plane,
        // so we can directly compare depths.
        backgroundDepth = texture(bgDepth, texCoord).x;
#endif  // BACKGROUND_AVAILABLE


        // Calculate total importance sum
        uint idx = pixelIdx;
        int counter = 0;
        float totalImportanceSum = 0.0;
        while (idx != 0 && counter < ABUFFER_SIZE) {
            vec4 data = readPixelStorage(idx - 1);
            #ifdef USE_IMPORTANCE_VOLUME
                float viewDepth = convertDepthScreenToView(camera, data.y);
                float clipDepth = convertDepthViewToClip(camera, viewDepth);
                vec4 clip = vec4(2.0 * texCoord - 1.0, clipDepth, 1.0);
                vec4 worldPos = camera.clipToWorld * clip;
                worldPos /= worldPos.w;
                vec3 texPos = (importanceVolumeParameters.worldToTexture * worldPos).xyz * importanceVolumeParameters.reciprocalDimensions;
                data.z = texture(importanceVolume, texPos).x; // sample importance from volume

                writePixelStorage(idx - 1, data);
            #endif
            float importance = data.z;
            if (data.y >= 0 && data.y <= backgroundDepth)
                totalImportanceSum += importance * importance;
            idx = floatBitsToUint(readPixelStorage(idx - 1).x);
            counter++;
        }

        // front-to-back compositing
        vec4 color = vec4(0);
        uint lastPtr = 0;
        vec4 nextFragment = selectionSortNext(pixelIdx, 0.0, lastPtr);
        abufferPixel unpackedFragment = uncompressPixelData(nextFragment);
        float currentImportanceSum = 0.0;
        
        while (unpackedFragment.depth >= 0 && unpackedFragment.depth <= backgroundDepth) {
            vec4 c = unpackedFragment.color;
            currentImportanceSum += c.a * c.a;
            c.a = 1 / (1 + pow(1 - c.a, 2 * lambda) * (r * (currentImportanceSum - c.a * c.a) + q * (totalImportanceSum - currentImportanceSum))); // set alpha by opacity optimisation
            color.rgb = color.rgb + (1 - color.a) * c.a * c.rgb;
            color.a = color.a + (1 - color.a) * c.a;

            nextFragment = selectionSortNext(pixelIdx, unpackedFragment.depth, lastPtr);
            unpackedFragment = uncompressPixelData(nextFragment);
        }

        FragData0 = vec4(color);
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
