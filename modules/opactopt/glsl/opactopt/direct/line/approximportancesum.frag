/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#if defined(ENABLE_ROUND_DEPTH_PROFILE)
// enable conservative depth writes (supported since GLSL 4.20)
#   if defined(GLSL_VERSION_450) || defined(GLSL_VERSION_440) || defined(GLSL_VERSION_430) || defined(GLSL_VERSION_420)
        layout (depth_less) out float gl_FragDepth;
#   endif

#if !defined M_PI
#define M_PI 3.14159265358979323846
#endif
#endif

uniform vec2 screenDim = vec2(512, 512);
uniform float antialiasing = 0.5; // width of antialised edged [pixel]
uniform float lineWidth = 2.0; // line width [pixel]

// initialize camera matrices with the identity matrix to enable default rendering
// without any transformation, i.e. all lines in clip space
uniform CameraParameters camera = CameraParameters( mat4(1), mat4(1), mat4(1), mat4(1),
                                    mat4(1), mat4(1), vec3(0), 0, 1);

uniform layout(r32i) iimage2DArray importanceSumCoeffs[2]; // double buffering for gaussian filtering
uniform layout(r32i) iimage2DArray opticalDepthCoeffs;

uniform sampler3D importanceVolume;
uniform VolumeParameters importanceVolumeParameters;

// line stippling
uniform StipplingParameters stippling = StipplingParameters(30.0, 10.0, 0.0, 4.0);

in float segmentLength_; // total length of the current line segment in screen space
in float distanceWorld_; // distance in world coords to segment start
in vec2 texCoord_; // x = distance to segment start, y = orth. distance to center (in screen coords)
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

void main() {
    // Prevent invisible fragments from blocking other objects (e.g., depth/picking)
    if(color_.a < 0.01) { discard; }

    float linewidthHalf = lineWidth * 0.5;

    // make joins round by using the texture coords
    float distance = abs(texCoord_.y);
    if (texCoord_.x < 0.0) { 
        distance = length(texCoord_); 
    }
    else if(texCoord_.x > segmentLength_) { 
        distance = length(vec2(texCoord_.x - segmentLength_, texCoord_.y)); 
    }

    float d = distance - linewidthHalf + antialiasing;

#if defined(ENABLE_ROUND_DEPTH_PROFILE)
    // correct depth for a round profile, i.e. tube like appearance
    float view_depth = convertDepthScreenToView(camera, gl_FragCoord.z);
    float maxDist = (linewidthHalf + antialiasing);
    // assume circular profile of line
    float z_v = view_depth - cos(distance/maxDist * M_PI) * maxDist / screenDim.x*0.5;
#else
    // Get linear depth
    float z_v = convertDepthScreenToView(camera, gl_FragCoord.z); // view space depth
#endif // ENABLE_ROUND_DEPTH_PROFILE

    float depth = (z_v - camera.nearPlane) / (camera.farPlane - camera.nearPlane); // linear normalised depth

    // Calculate g_i^2
#ifdef USE_IMPORTANCE_VOLUME
    float viewDepth = depth * (camera.farPlane - camera.nearPlane) + camera.nearPlane;
    float clipDepth = convertDepthViewToClip(camera, viewDepth);
    vec4 clip = vec4(2.0 * texCoord - 1.0, clipDepth, 1.0);
    vec4 worldPos = camera.clipToWorld * clip;
    worldPos /= worldPos.w;
    vec3 texPos = (importanceVolumeParameters.worldToTexture * worldPos).xyz * importanceVolumeParameters.reciprocalDimensions;
    float gi = texture(importanceVolume, texPos.xyz).x; // sample importance from volume
#else
    float gi = color_.a;
#endif

    // line stippling
#if defined(ENABLE_STIPPLING)

#if STIPPLE_MODE == 2
    // in world space
    float v = (distanceWorld_ * stippling.worldScale);
#else
    // in screen space
    float v = (texCoord_.x + stippling.offset) / stippling.length;    
#endif // STIPPLE_MODE

    float t = fract(v) * (stippling.length) / stippling.spacing;
    if ((t > 0.0) && (t < 1.0)) {
        // renormalize t with respect to stippling length
        t = min(t, 1.0-t) * (stippling.spacing) * 0.5;
        d = max(d, t);
    }
#endif // ENABLE_STIPPLING

    float alphamul = 1.0;
    // antialiasing around the edges
    if( d > 0) {
        // apply antialiasing by modifying the alpha [Rougier, Journal of Computer Graphics Techniques 2013]
        d /= antialiasing;
        alphamul = exp(-d*d);
    }

    gi *= alphamul;

    // Project importance
    float gisq = gi * gi;
    float gtot = total(importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS);
    float Gd = approximate(importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS, depth) + 0.5 * gisq; // correct for importance sum approximation at discontinuity
    float alpha = clamp(1 /
                    (1 + pow(1 - gi, 2 * lambda)
                    * (r * max(0, Gd - gisq)
                    + q * max(0, gtot - Gd))),
                    0.0, 0.9999); // set pixel alpha using opacity optimisation


    project(opticalDepthCoeffs, N_OPTICAL_DEPTH_COEFFICIENTS, depth, -log(1 - alpha));

    discard;
}
