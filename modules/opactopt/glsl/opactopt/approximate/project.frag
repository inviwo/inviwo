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
#include "utils/depth.glsl"
#include "oit/commons.glsl"

// How should the stuff be rendered? (Debugging options)
#define ABUFFER_DISPNUMFRAGMENTS 0
#define ABUFFER_RESOLVE_USE_SORTING 1

uniform vec2 reciprocalDimensions;

uniform layout(size1x32) image2DArray importanceSumCoeffs[2]; // double buffering for gaussian filtering
#ifndef USE_EXACT_BLENDING
uniform layout(size1x32) image2DArray opticalDepthCoeffs;
#endif

uniform sampler3D importanceVolume;
uniform CameraParameters camera;
uniform VolumeParameters importanceVolumeParameters;

uniform ivec2 screenSize;

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


// Converts depths from screen space to clip space
void lineariseDepths(uint pixelIdx);

// Resolve A-Buffer and blend sorted fragments
void main() {
    ivec2 coords = ivec2(gl_FragCoord.xy);

    if (coords.x < 0 || coords.y < 0 || coords.x >= screenSize.x ||
        coords.y >= screenSize.y) {
        discard;
    }

    vec2 texCoord = (gl_FragCoord.xy + 0.5) * reciprocalDimensions;

    uint pixelIdx = getPixelLink(coords);
    if (pixelIdx > 0) {
        lineariseDepths(pixelIdx);
        uint idx = pixelIdx;

        // Project importance sum coefficients
        int counter = 0;
        while (idx != 0 && counter < ABUFFER_SIZE) {
            vec4 data = readPixelStorage(idx - 1);
            #ifdef USE_IMPORTANCE_VOLUME
                float viewDepth = data.y * (camera.farPlane - camera.nearPlane) + camera.nearPlane;
                float clipDepth = convertDepthViewToClip(camera, viewDepth);
                vec4 clip = vec4(2.0 * texCoord - 1.0, clipDepth, 1.0);
                vec4 worldPos = camera.clipToWorld * clip;
                worldPos /= worldPos.w;
                vec3 texPos = (importanceVolumeParameters.worldToTexture * worldPos).xyz * importanceVolumeParameters.reciprocalDimensions;
                data.z = clamp(texture(importanceVolume, texPos.xyz).x, 0.0, 1.0); // sample importance from volume
                writePixelStorage(idx - 1, data);
            #endif
            project(importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS, data.y, data.z * data.z);
            idx = floatBitsToUint(data.x);
        }
    }

    discard;
}

void lineariseDepths(uint pixelIdx) {
    int counter = 0;
    while (pixelIdx != 0 && counter < ABUFFER_SIZE) {
        vec4 val = readPixelStorage(pixelIdx - 1);
        float z_v = convertDepthScreenToView(camera, val.y); // view space depth
        val.y = (z_v - camera.nearPlane) / (camera.farPlane - camera.nearPlane); // linear normalised depth;
        writePixelStorage(pixelIdx - 1, val); 
        counter++;
        pixelIdx = floatBitsToUint(val.x);
    }
}
