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
#include "utils/structs.glsl"
#include "utils/depth.glsl"

#ifndef IVW_OPACTOPT_IMPORTANCE
#define IVW_OPACTOPT_IMPORTANCE

#ifdef USE_IMPORTANCE_VOLUME
uniform sampler3D importanceVolume;
uniform VolumeParameters importanceVolumeParameters;
#endif

// Opacity optimisation settings
uniform float q;
uniform float r;
uniform float lambda;

#ifdef USE_IMPORTANCE_VOLUME
float importance(vec2 texCoord, float depth, CameraParameters camera) {
    float viewDepth = depth * (camera.farPlane - camera.nearPlane) + camera.nearPlane;
    float clipDepth = convertDepthViewToClip(camera, viewDepth);
    vec4 clip = vec4(2.0 * texCoord - 1.0, clipDepth, 1.0);
    vec4 worldPos = camera.clipToWorld * clip;
    worldPos /= worldPos.w;
    vec3 texPos = (importanceVolumeParameters.textureToIndex * importanceVolumeParameters.worldToTexture * worldPos).xyz;
    // sample importance from volume
    float gi = getNormalizedVoxel(importanceVolume, importanceVolumeParameters, texPos.xyz).x;

    return gi;
}
#endif

float approximateAlpha(float gi, float depth,
                        layout(IMAGE_LAYOUT) IMAGE_UNIT coeffTex, int N) {
    float gisq = gi * gi;
    float gtot = total(coeffTex, N);
    float Gd = approximate(coeffTex, N, depth);

    Gd += 0.5 * gisq;  // correct for importance sum approximation at discontinuity

    // set pixel alpha using opacity optimisation
    float Gdgi2 = max(0, Gd - gisq);
    float gtotGd = max(0, gtot - Gd);
    float alpha = 1.0 / (1.0 + pow(1.0 - gi, 2 * lambda) * (r * Gdgi2 + q * gtotGd));
    return clamp(alpha, 0.0, 0.9999);
}

#endif  // IVW_OPACTOPT_IMPORTANCE
