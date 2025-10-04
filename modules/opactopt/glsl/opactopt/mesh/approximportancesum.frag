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
#include "opactopt/importance.glsl"

uniform CameraParameters camera;
uniform vec2 reciprocalDimensions;

uniform layout(IMAGE_LAYOUT) IMAGE_UNIT  importanceSumCoeffs[2];  // double buffering for gaussian filtering
uniform layout(IMAGE_LAYOUT) IMAGE_UNIT  opticalDepthCoeffs;

in vec4 color_;

void main() {
    // Prevent invisible fragments from blocking other objects (e.g., depth/picking)
    if (color_.a == 0.0) discard;

    // Get linear depth
    float z_v = convertDepthScreenToView(camera, gl_FragCoord.z);  // view space depth
     // linear normalised depth
    float depth = (z_v - camera.nearPlane) / (camera.farPlane - camera.nearPlane);

    // Calculate g_i^2
#ifdef USE_IMPORTANCE_VOLUME
    float gi = importance(gl_FragCoord.xy * reciprocalDimensions, depth, camera);
#else
    float gi = color_.a;
#endif

    float alpha = approximateAlpha(gi, depth, importanceSumCoeffs[0], N_IMPORTANCE_SUM_COEFFICIENTS);

    project(opticalDepthCoeffs, N_OPTICAL_DEPTH_COEFFICIENTS, depth, -log(1 - alpha));

    discard;
}
