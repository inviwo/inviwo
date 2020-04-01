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
 * Convertion from the a-buffer (fragment lists) to the illustration buffers:
 * Load fragments, sort them, store them in linear memory
 */

// this is important for the occlusion query
layout(early_fragment_tests) in;

#include "oit/abufferlinkedlist.glsl"
#include "illustration/illustrationbuffer.glsl"
#include "oit/sort.glsl"
#include "utils/structs.glsl"

// atomic counter to allocate space in the illustration buffer
// just reuse the counter from the A-Buffer
#define illustrationBufferCounter abufferCounter

// We need some way to add an arbitrary count to the atomic buffer, not just increment it
#if defined(GLSL_VERSION_460)
#define atomicAdd atomicCounterAdd
#elif defined(GL_ARB_shader_atomic_counter_ops)
#define atomicAdd atomicCounterAddARB
#else
#error \
    "Sorry, but to fill the Illustration Buffer, I need to be able to add an atomic counter: OpenGL 4.6 or ARB_shader_atomic_counter_ops"
#endif

#ifdef BACKGROUND_AVAILABLE
uniform ImageParameters bgParameters;
uniform sampler2D bgColor;
uniform sampler2D bgDepth;
uniform vec2 reciprocalDimensions;
#endif  // BACKGROUND_AVAILABLE

// Whole number pixel offsets (not necessary just to test the layout keyword !)
layout(pixel_center_integer) in vec4 gl_FragCoord;

// Input interpolated fragment position
smooth in vec4 fragPos;

layout(std430, binding = 0) buffer colorBufferOut {
    vec2 colorOut[];  // alpha + color
};
layout(std430, binding = 1) buffer surfaceInfoBufferOut {
    vec2 surfaceInfoOut[];  // depth + gradient
};

void main(void) {
    ivec2 coords = ivec2(gl_FragCoord.xy);

    if (coords.x >= 0 && coords.y >= 0 && coords.x < AbufferParams.screenWidth &&
        coords.y < AbufferParams.screenHeight) {

        uint pixelIdx = getPixelLink(coords);
        if (pixelIdx > 0) {
            float backgroundDepth = 1.0;
#ifdef BACKGROUND_AVAILABLE
            // Assume the camera used to render the background has the same near and far plane,
            // so we can directly compare depths.
            vec2 texCoord = (gl_FragCoord.xy + 0.5) * reciprocalDimensions;
            backgroundDepth = texture(bgDepth, texCoord).x;
#endif  // BACKGROUND_AVAILABLE

            // we have fragments
            // 1. load and sort them
            int numFrag = 0;
            fillFragmentArraySortedUntilDepth(pixelIdx, backgroundDepth, numFrag);
            // 2. write them back
            uint start = atomicAdd(illustrationBufferCounter, numFrag);
            for (int i = 0; i < numFrag; ++i) {
                colorOut[start + i] = vec2(fragmentList[i].z, fragmentList[i].w);
                surfaceInfoOut[start + i] = vec2(fragmentList[i].y, 0);  // TODO: gradient
            }
            imageStore(illustrationBufferIdxImg, coords, ivec4(start));
            imageStore(illustrationBufferCountImg, coords, ivec4(numFrag));
        } else {
            // no fragments, clear texture
            imageStore(illustrationBufferIdxImg, coords, ivec4(0));
            imageStore(illustrationBufferCountImg, coords, ivec4(0));
        }
    }
    discard;
}