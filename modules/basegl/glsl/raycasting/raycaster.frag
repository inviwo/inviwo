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


#include "utils/structs.glsl"
#include "utils/sampler3d.glsl"
#include "utils/depth.glsl"

@INCLUDE

vec3 calcRaydirection(in vec3 entryPoint, in vec3 exitPoint) {
    return normalize(exitPoint - entryPoint);
}

float calcEnd(in vec3 entryPoint, in vec3 exitPoint) { return length(exitPoint - entryPoint); }

float calcIncr(in float end, in vec3 direction, in float samplingRate, in vec3 dimensions) {
    float incr = min(end, end / (samplingRate * length(direction * dimensions)));
    float samples = ceil(end / incr);
    return end / samples;
}

vec3 calcCameraDir(in vec3 entryPoint, in vec3 exitPoint, in mat4 textureToWorld) {
    return normalize(
        (textureToWorld * vec4(entryPoint, 1.0) - textureToWorld * vec4(exitPoint, 1.0)).xyz);
}


uniform VolumeParameters volumeParameters;
uniform sampler3D volume;

uniform ImageParameters entryParameters;
uniform sampler2D entryColor;
uniform sampler2D entryDepth;

uniform ImageParameters exitParameters;
uniform sampler2D exitColor;
uniform sampler2D exitDepth;

uniform ImageParameters outportParameters;

uniform CameraParameters camera;
uniform float samplingRate = 2.0;

@UNIFORM

uniform int channel = 0;


vec4 rayTraversal(in vec3 entryPoint, in vec3 exitPoint, in float entryPointDepth,
                  in float exitPointDepth, in vec4 backgroundColor, in float backgroundDepth) {

    float tEnd = calcEnd(entryPoint, exitPoint);
    vec3 rayDirection = calcRaydirection(entryPoint, exitPoint);
    float tIncr = calcIncr(tEnd, rayDirection, samplingRate, volumeParameters.dimensions);
    vec3 toCameraDir = calcCameraDir(entryPoint, exitPoint, volumeParameters.textureToWorld);

    vec4 result = vec4(0.0);
    float tDepth = -1.0;
    vec4 voxel = vec4(0.0);
    vec3 gradient = vec3(0.0);

    @PRE

    voxel = getNormalizedVoxel(volume, volumeParameters, entryPoint + 0.5 * tIncr * rayDirection);

    for (float t = 0.5 * tIncr; t < tEnd; t += tIncr) {
        vec3 samplePos = entryPoint + t * rayDirection;
        vec4 previousVoxel = voxel;
        vec3 previousGradient = gradient;
        voxel = getNormalizedVoxel(volume, volumeParameters, samplePos);
    
        @LOOP

        // early ray termination
        if (result.a > 0.99) break;
    }

    @POST

    if (tDepth != -1.0) {
        tDepth = calculateDepthValue(camera, tDepth / tEnd, entryPointDepth, exitPointDepth);
    } else {
        tDepth = 1.0;
    }

    gl_FragDepth = min(backgroundDepth, tDepth);

    return result;
}

void main() {
    vec2 texCoords = gl_FragCoord.xy * outportParameters.reciprocalDimensions;
    vec3 entryPoint = texture(entryColor, texCoords).rgb;
    vec3 exitPoint = texture(exitColor, texCoords).rgb;

    float entryPointDepth = texture(entryDepth, texCoords).x;
    float exitPointDepth = texture(exitDepth, texCoords).x;

    vec4 color = vec4(0);
    float backgroundDepth = 1;
    PickingData = vec4(0);

    @MAIN

    if (entryPoint != exitPoint) {
        color = rayTraversal(entryPoint, exitPoint, entryPointDepth, exitPointDepth, color,
                             backgroundDepth);
    }
    FragData0 = color;
}
