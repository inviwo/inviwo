/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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

#ifndef IVW_GLYPHS_GLSL
#define IVW_GLYPHS_GLSL

#include "utils/structs.glsl" //! #include "./structs.glsl"
#include "utils/shading.glsl"

// shade the visible cut surface of a glyph when it gets clipped by the near clip plane
//
// @param glyphCenter     center of the glyph in object space
// @param lighting        lighting params
// @param srcColor        glyph color (non-shaded)
// @param dstColor        color of the clipped surface (shading depends on `DISCARD_CLIPPED_GLYPHS`
//                        and `SHADE_CLIPPED_AREA`)
// @param depth           depth in normalized device coords [0,1]
// @return true if the fragment should be discarded (clipped surface not visible)
//
bool clipGlypNearPlane(in vec4 glyphCenter, in vec3 camDir,
                       in LightParameters lighting, in vec3 srcColor, 
                       inout vec4 dstColor, inout float depth) {
    // first intersection lies behind the camera
#ifdef DISCARD_CLIPPED_GLYPHS
    if (depth <= 0.0) {
        return true;
    }
#elif defined(SHADE_CLIPPED_AREA)
    if (depth <= 0.0) {
        // need to compute proper ray-near plane intersection for shading.
        // Use initial glyph coordinate for now since this should be precise enough for smaller glyphs.

        // clip surface is orthogonal to view direction of the camera, use camera direction as normal
        vec3 normal = normalize(camDir);
        ShadingParameters shadingParams = shading(srcColor, normal, glyphCenter.xyz);
        dstColor.rgb = applyLighting(lighting, shadingParams, normal);
        depth += 0.000001;
    }
#else
    if (depth <= 0.0) {
        // no shading
        dstColor.rgb = srcColor;
        depth += 0.000001;
    }
#endif // DISCARD_CLIPPED_GLYPHS
    return false;
}

// compute the correct non-linear depth of a glyph in normalized device coordinates
float glyphDepth(in vec3 posWorld, in mat4 worldToClip) {
    mat4 mvpTranspose = transpose(worldToClip);

    vec4 pos = vec4(posWorld, 1.0);
    float depth = dot(mvpTranspose[2], pos);
    float depthW = dot(mvpTranspose[3], pos);

    return ((depth / depthW) + 1.0) * 0.5;
}

#endif // IVW_GLYPHS_GLSL
