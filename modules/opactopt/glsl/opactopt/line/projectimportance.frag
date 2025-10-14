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
#include "utils/sampler3d.glsl"

#include "opactopt/common.glsl"
#include "opactopt/approximation/fourier.glsl"
#include "opactopt/approximation/legendre.glsl"
#include "opactopt/approximation/piecewise.glsl"
#include "opactopt/approximation/powermoments.glsl"
#include "opactopt/approximation/trigmoments.glsl"
#include "opactopt/debug.glsl"
#include "opactopt/importance.glsl"

uniform ivec2 screenSize = ivec2(512, 512);
uniform float antialiasing = 0.5;  // width of antialised edged [pixel]
uniform float lineWidth = 2.0;     // line width [pixel]

uniform CameraParameters camera;
uniform vec2 reciprocalDimensions;

restrict uniform layout(IMAGE_LAYOUT) IMAGE_UNIT importanceSumCoeffs[2];     // double buffering for gaussian filtering

// line stippling
uniform StipplingParameters stippling = StipplingParameters(30.0, 10.0, 0.0, 4.0);

in LineGeom {
    vec2 texCoord;        // x = distance to segment start, y = orth. distance to center
                          // (in screen coords)
    vec4 color;
    flat vec4 pickColor;
    float segmentLength;  // total length of the current line segment in screen space
    float distanceWorld;  // distance in world coords to segment start
} fragment;

void main() {
    // Prevent invisible fragments from blocking other objects (e.g., depth/picking)
    if (fragment.color.a < 0.0) discard;

    float linewidthHalf = lineWidth * 0.5;

    // make joins round by using the texture coords
    float distance = abs(fragment.texCoord.y);
    if (fragment.texCoord.x < 0.0) {
        distance = length(fragment.texCoord);
    } else if (fragment.texCoord.x > fragment.segmentLength) {
        distance = length(vec2(fragment.texCoord.x - fragment.segmentLength, fragment.texCoord.y));
    }

    float d = distance - linewidthHalf + antialiasing;

#if defined(ENABLE_ROUND_DEPTH_PROFILE)
    // correct depth for a round profile, i.e. tube like appearance
    float view_depth = convertDepthScreenToView(camera, gl_FragCoord.z);
    float maxDist = (linewidthHalf + antialiasing);
    // assume circular profile of line
    float z_v = view_depth - cos(distance / maxDist * PI) * maxDist / screenSize.x * 0.5;
#else
    // Get linear depth
    float z_v = convertDepthScreenToView(camera, gl_FragCoord.z);  // view space depth
#endif  // ENABLE_ROUND_DEPTH_PROFILE

    // linear normalised depth
    float depth = (z_v - camera.nearPlane) / (camera.farPlane - camera.nearPlane);

    // Calculate g_i^2
#ifdef USE_IMPORTANCE_VOLUME
    float gi = importance(gl_FragCoord.xy * reciprocalDimensions, depth, camera);
#else
    float gi = fragment.color.a;
#endif

    float alphamul = 1.0;

    // line stippling
#if defined(ENABLE_STIPPLING)

#if STIPPLE_MODE == 2
    // in world space
    float v = (fragment.distanceWorld * stippling.worldScale);
#else
    // in screen space
    float v = (fragment.texCoord.x + stippling.offset) / stippling.length;
#endif  // STIPPLE_MODE

    float t = fract(v) * (stippling.length) / stippling.spacing;
    if ((t > 0.0) && (t < 1.0)) {
        // renormalize t with respect to stippling length
        t = min(t, 1.0 - t) * (stippling.spacing) * 0.5;
        d = max(d, t);
    }
#endif  // ENABLE_STIPPLING

    // antialiasing around the edges
    if (d > 0) {
        // apply antialiasing by modifying the alpha [Rougier, Journal of Computer Graphics
        // Techniques 2013]
        d /= antialiasing;
        alphamul = exp(-d * d);
    }

    // Project importance
    gi *= alphamul;
    float gisq = gi * gi;
    project(importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS, depth, gisq);

    IVW_OPACTOPT_DEBUGGING(depth, gi)

    // write into intermediate image to indicate pixel is being written to
    PickingData = vec4(vec3(0), 1);
}

