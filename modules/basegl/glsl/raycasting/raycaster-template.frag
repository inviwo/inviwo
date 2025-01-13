/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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
#include "utils/sampler3d.glsl"
#include "utils/depth.glsl"

#pragma IVW_SHADER_SEGMENT_PLACEHOLDER_INCLUDE

#if !defined APPLY_LIGHTING_FUNC
#  define APPLY_LIGHTING_FUNC applyLighting
#endif // APPLY_LIGHTING_FUNC

float calcStep(in float rayLength, in vec3 direction, in float samplingRate, in vec3 dimensions) {
    float incr = min(rayLength, rayLength / (samplingRate * length(direction * dimensions)));
    float samples = ceil(rayLength / incr);
    return rayLength / samples;
}

vec3 calcCameraDir(in vec3 entryPoint, in vec3 exitPoint, in mat4 textureToWorld) {
    return normalize(
        (textureToWorld * vec4(entryPoint, 1.0) - textureToWorld * vec4(exitPoint, 1.0)).xyz);
}

layout(location = 0) out vec4 FragData0;
layout(location = 1) out vec4 PickingData;

uniform ImageParameters outportParameters;
uniform float samplingRate = 2.0;

#pragma IVW_SHADER_SEGMENT_PLACEHOLDER_UNIFORM

void main() {
    vec4 result = vec4(0.0);   // The accumulated color along the ray;
    vec4 picking = vec4(0.0);  // The picking color of the ray
    float rayDepth = -1.0;     // The ray depth value [0, ray length], -1 means "no" depth.
                               // Uses the same space as rayPosition. Usually used to track
                               // the depth of the "first" hit in DVR.
    float depth = 1.0;         // The image depth, from far to near [0, 1].
                               // Will be overridden by rayDepth if != -1 and then
                               // written to gl_FragDepth

    vec2 texCoords = gl_FragCoord.xy * outportParameters.reciprocalDimensions;

    // The setup placeholder is expected to define:
    //  * entryPoint       the ray start point in the volume in texture coordinates
    //  * exitPoint        the ray exit point in the volume in texture coordinates
    //  * entryPointDepth  the image depth of the entry point
    //  * exitPointDepth   the image depth of the exit point
    //  * rayLength        the distance from the start to the exit point in texture space
    //  * rayDirection     the direction of the ray in texture space, normalized.

    #pragma IVW_SHADER_SEGMENT_PLACEHOLDER_SETUP

    if (entryPoint == exitPoint) {
        FragData0 = result;
        PickingData = picking;
        gl_FragDepth = depth;
        return;
    }

    // The step size in texture space
    float rayStep = calcStep(rayLength, rayDirection, samplingRate, volumeParameters.dimensions);

    vec3 cameraDir = calcCameraDir(entryPoint, exitPoint, volumeParameters.textureToWorld);

    // Current position along the ray
    float rayPosition = 0.5 * rayStep;
    // Current sample position in texture spcase
    vec3 samplePosition = entryPoint + rayPosition * rayDirection;

    #pragma IVW_SHADER_SEGMENT_PLACEHOLDER_FIRST

    for (rayPosition += rayStep; rayPosition < rayLength; rayPosition += rayStep) {
        samplePosition = entryPoint + rayPosition * rayDirection;

        #pragma IVW_SHADER_SEGMENT_PLACEHOLDER_LOOP

        if (result.a > 0.99) break;  // early ray termination
    }

    #pragma IVW_SHADER_SEGMENT_PLACEHOLDER_POST

    depth = mix(calculateDepthValue(camera, rayDepth / rayLength, entryPointDepth, exitPointDepth),
                depth, rayDepth == -1.0);

    FragData0 = result;
    PickingData = picking;
    gl_FragDepth = depth;
}
