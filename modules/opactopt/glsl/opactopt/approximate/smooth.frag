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

#ifdef USE_ABUFFER
    #include "oit/abufferlinkedlist.glsl"
#endif

uniform ivec2 screenSize;
uniform int radius;
layout(std430, binding = 8) buffer gaussianKernelBuffer {
    float kernel[];
};

#ifdef COEFF_TEX_FIXED_POINT_FACTOR
uniform layout(r32i) iimage2DArray importanceSumCoeffs[2];
#else
uniform layout(size1x32) image2DArray importanceSumCoeffs[2];
#endif

#ifdef USE_PICK_IMAGE
#include "utils/structs.glsl"

uniform ImageParameters imageParameters;
uniform sampler2D imageColor;
uniform sampler2D imagePicking;
#endif

// Whole number pixel offsets (not necessary just to test the layout keyword !)
in vec4 gl_FragCoord;

// Input interpolated fragment position
smooth in vec4 fragPos;

// Resolve A-Buffer and blend sorted fragments
void main() {
    ivec2 coords = ivec2(gl_FragCoord.xy);

    #if HORIZONTAL == 1
        ivec2 dir = ivec2(1, 0);
    #else
        ivec2 dir = ivec2(0, 1);
    #endif

    #if defined(USE_ABUFFER)
        if (getPixelLink(coords) == 0) return;
    #elif defined(USE_PICK_IMAGE)
        if (texelFetch(imagePicking, coords, 0).a == 0.0) return;
    #else
        if (imageLoad(importanceSumCoeffs[0], ivec3(coords, 0)).x == 0) return;
    #endif

    for (int i = 0; i < N_IMPORTANCE_SUM_COEFFICIENTS; i++) {
        ivec3 layer_coord = ivec3(coords, i);

        float val = 0.0;
        float kernel_sum = 0.0;
        for (int j = -radius; j <= radius; j++) {
            #if HORIZONTAL == 1
                if (coords.x + j < 0 || coords.x + j >= screenSize.x) continue;
            #else
                if (coords.y + j < 0 || coords.y + j >= screenSize.y) continue;
            #endif

            #if defined(USE_ABUFFER)
                if (getPixelLink(coords + j * dir) == 0) continue;
            #elif defined(USE_PICK_IMAGE)
                if (texelFetch(imagePicking, coords + j * dir, 0).a == 0.0) continue;
            #else
                if (imageLoad(importanceSumCoeffs[0], ivec3(coords + j * dir, 0)).x == 0) continue;
            #endif

            float coeff = imageLoad(importanceSumCoeffs[1 - HORIZONTAL], layer_coord + ivec3(j * dir, 0)).x;

            val += kernel[abs(j)] * coeff;
            kernel_sum += kernel[abs(j)];
        }
        val /= kernel_sum;

        #ifdef COEFF_TEX_FIXED_POINT_FACTOR
        imageStore(importanceSumCoeffs[HORIZONTAL], layer_coord, ivec4(val));
        #else
        imageStore(importanceSumCoeffs[HORIZONTAL], layer_coord, vec4(val));
        #endif
    }

    discard;
}
