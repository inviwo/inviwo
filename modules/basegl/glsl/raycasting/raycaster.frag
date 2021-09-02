/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

//? #version 460

#include "utils/structs.glsl"   //! #include "../../../opengl/glsl/utils/structs.glsl"
#include "utils/sampler3d.glsl" //! #include "../../../opengl/glsl/utils/sampler3d.glsl"
#include "utils/depth.glsl"     //! #include "../../../opengl/glsl/utils/depth.glsl"

#pragma IVW_INCLUDE

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

uniform ImageParameters entryParameters;
uniform sampler2D entryColor;
uniform sampler2D entryDepth;
uniform ImageParameters exitParameters;
uniform sampler2D exitColor;
uniform sampler2D exitDepth;
uniform ImageParameters outportParameters;
uniform float samplingRate = 2.0;

uniform int channel = 0;

//? uniform VolumeParameters volumeParameters;
//? uniform CameraParameters camera;

#pragma IVW_UNIFORM


void main() {
    vec2 texCoords = gl_FragCoord.xy * outportParameters.reciprocalDimensions;

    vec3 entryPoint = texture(entryColor, texCoords).rgb;
    vec3 exitPoint = texture(exitColor, texCoords).rgb;
    float entryPointDepth = texture(entryDepth, texCoords).x;
    float exitPointDepth = texture(exitDepth, texCoords).x;

    // The length of the ray in texture space
    float rayLength = length(exitPoint - entryPoint);

    // The normalized direction of the ray
    vec3 rayDirection = normalize(exitPoint - entryPoint);

    // The step size in texture space
    float rayStep = calcStep(rayLength, rayDirection, samplingRate, 
                             volumeParameters.dimensions);

    vec4 result = vec4(0.0);  // The accumulated color along the ray;
    vec4 picking = vec4(0.0); // The picking color of the ray
    float rayDepth = -1.0;    // The ray depth value (0 to ray length, -1 mean "no" depth
    float depth = 1.0;        // The image depth 

    #pragma IVW_SETUP

    if (entryPoint == exitPoint) {
        FragData0 = result;
        PickingData = picking;
        gl_FragDepth = depth;
        return;
    }

    vec3 cameraDir = calcCameraDir(entryPoint, exitPoint, 
                                   volumeParameters.textureToWorld);

    // Current position along the ray
    float rayPosition = 0.5 * rayStep;
    // Current sample position in texture spcase
    vec3 samplePosition = entryPoint + rayPosition * rayDirection;

    #pragma IVW_FIRST

    for (rayPosition += rayStep; rayPosition < rayLength; rayPosition += rayStep) {
        samplePosition = entryPoint + rayPosition * rayDirection;

        #pragma IVW_LOOP

        if (result.a > 0.99) break; // early ray termination
    }


    #pragma IVW_POST

    depth = mix(calculateDepthValue(camera, rayDepth / rayLength, 
                                    entryPointDepth, exitPointDepth), 
                depth, 
                rayDepth == -1.0);
    
    FragData0 = result;
    PickingData = picking;
    gl_FragDepth = depth;
}
