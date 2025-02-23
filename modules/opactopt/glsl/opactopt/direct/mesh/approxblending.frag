/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

// Owned by the MeshRenderProcessorGL Processor

#include "utils/shading.glsl"
#include "utils/depth.glsl"
#include "utils/sampler3d.glsl"

#if !defined(TEXCOORD_LAYER) && !defined(NORMALS_LAYER) \
    && !defined(VIEW_NORMALS_LAYER) && !defined(COLOR_LAYER)
#  define COLOR_LAYER
#endif

uniform LightParameters lighting;
uniform CameraParameters camera;
uniform vec2 reciprocalDimensions;

#ifdef COEFF_TEX_FIXED_POINT_FACTOR
uniform layout(r32i) iimage2DArray importanceSumCoeffs[2]; // double buffering for gaussian filtering
uniform layout(r32i) iimage2DArray opticalDepthCoeffs;
#else
uniform layout(size1x32) image2DArray importanceSumCoeffs[2]; // double buffering for gaussian filtering
uniform layout(size1x32) image2DArray opticalDepthCoeffs;
#endif

#ifdef USE_IMPORTANCE_VOLUME
uniform sampler3D importanceVolume;
uniform VolumeParameters importanceVolumeParameters;
#endif

in vec4 worldPosition_;
in vec3 normal_;
in vec3 viewNormal_;
in vec3 texCoord_;
in vec4 color_;
flat in vec4 pickColor_;

// Opacity optimisation settings
uniform float q;
uniform float r;
uniform float lambda;

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


void main() {
    // Prevent invisible fragments from blocking other objects (e.g., depth/picking)
    if(color_.a == 0) { discard; }

    // Get linear depth
    float z_v = convertDepthScreenToView(camera, gl_FragCoord.z); // view space depth
    float depth = (z_v - camera.nearPlane) / (camera.farPlane - camera.nearPlane); // linear normalised depth

    vec4 c = color_;
    vec3 toCameraDir_ = camera.position - worldPosition_.xyz;
    c.rgb = APPLY_LIGHTING(lighting, color_.rgb, color_.rgb, vec3(1.0f), worldPosition_.xyz,
                                   normalize(normal_), normalize(toCameraDir_));

    // Calculate g_i^2
    #ifdef USE_IMPORTANCE_VOLUME
        vec2 texCoord = gl_FragCoord.xy * reciprocalDimensions;
        float viewDepth = depth * (camera.farPlane - camera.nearPlane) + camera.nearPlane;
        float clipDepth = convertDepthViewToClip(camera, viewDepth);
        vec4 clip = vec4(2.0 * texCoord - 1.0, clipDepth, 1.0);
        vec4 worldPos = camera.clipToWorld * clip;
        worldPos /= worldPos.w;
        vec3 texPos = (importanceVolumeParameters.worldToTexture * worldPos).xyz * importanceVolumeParameters.reciprocalDimensions;
        float gi = getNormalizedVoxel(importanceVolume, importanceVolumeParameters, texPos.xyz).x; // sample importance from volume
    #else
        float gi = color_.a;
    #endif

    // find alpha
    float gisq = gi * gi;
    float gtot = total(importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS);
    float Gd = approximate(importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS, depth);
    Gd += 0.5 * gisq; // correct for importance sum approximation at discontinuity
    float alpha = clamp(1 /
                    (1 + pow(1 - gi, 2 * lambda)
                    * (r * max(0, Gd - gisq)
                    + q * max(0, gtot - Gd))),
                    0.0, 0.9999); // set pixel alpha using opacity optimisation

    // find optical depth
    float taud = approximate(opticalDepthCoeffs, N_OPTICAL_DEPTH_COEFFICIENTS, depth);
    float weight = alpha / sqrt(1 - alpha) * exp(-taud); // correct for optical depth approximation at discontinuity

    c.rgb = weight * c.rgb;
    c.a = weight; // save sum of weights in alpha channel for later division

    FragData0 = c;
    PickingData = pickColor_;
}
