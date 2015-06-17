/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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
#include "utils/sampler2d.glsl"
#include "utils/sampler3d.glsl"

#include "utils/classification.glsl"
#include "utils/compositing.glsl"
#include "utils/depth.glsl"
#include "utils/gradients.glsl"
#include "utils/shading.glsl"

uniform VolumeParameters volumeParameters;
uniform sampler3D volume;

uniform VolumeParameters lightVolumeParameters;
uniform sampler3D lightVolume;

uniform sampler2D transferFunction;

uniform ImageParameters entryParameters;
uniform sampler2D entryColor;
uniform sampler2D entryDepth;
uniform sampler2D entryPicking;

uniform ImageParameters exitParameters;
uniform sampler2D exitColor;
uniform sampler2D exitDepth;

uniform ImageParameters outportParameters;

uniform LightParameters lighting;
uniform CameraParameters camera;
uniform VolumeIndicatorParameters positionindicator;
uniform RaycastingParameters raycaster;

uniform int channel;

#define ERT_THRESHOLD 0.99 // threshold for early ray termination

vec4 rayTraversal(vec3 entryPoint, vec3 exitPoint, vec2 texCoords) {
    vec4 result = vec4(0.0);
    vec3 rayDirection = exitPoint - entryPoint;
    float tEnd = length(rayDirection);
    float tIncr =
        min(tEnd, tEnd / (raycaster.samplingRate * length(rayDirection * volumeParameters.dimensions)));
    float samples = ceil(tEnd / tIncr);
    tIncr = tEnd / samples;
    float t = 0.5f * tIncr;
    rayDirection = normalize(rayDirection);
    float tDepth = -1.0;
    vec4 color;
    vec4 voxel;
    vec4 lightVoxel;
    vec3 samplePos;
    vec3 toCameraDir = normalize(camera.position - (volumeParameters.textureToWorld * vec4(entryPoint, 1.0)).xyz);
    while (t < tEnd) {
        samplePos = entryPoint + t * rayDirection;
        voxel = getNormalizedVoxel(volume, volumeParameters, samplePos);
        color = APPLY_CLASSIFICATION(transferFunction, voxel);
        if (color.a > 0) {
            vec3 gradient = COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volumeParameters, samplePos, channel);
            gradient = normalize(gradient);

            // World space position
            vec3 worldSpacePosition = (volumeParameters.textureToWorld * vec4(samplePos, 1.0)).xyz;
            // Note that the gradient is reversed since we define the normal of a surface as
            // the direction towards a lower intensity medium (gradient points in the inreasing direction)
            color.rgb = APPLY_LIGHTING(lighting, color.rgb, color.rgb, vec3(1.0), worldSpacePosition, -gradient, toCameraDir);

            //Light Volume Compositing
            lightVoxel = getVoxel(lightVolume, lightVolumeParameters, samplePos);
#ifdef LIGHT_COLOR_ENABLED
            color.rgb = (color.rgb * 0.6) + (lightVoxel.rgb * color.rgb * 0.8) + 
                        (0.8 * color.rgb * pow(lightVoxel.rgb, vec3(5.0)));
            color.rgb *= lightVoxel.a;
#else
            color.rgb *= lightVoxel.r;
#endif

            result = APPLY_COMPOSITING(result, color, samplePos, voxel, gradient, camera, raycaster.isoValue,
                t, tDepth, tIncr);
        }

        // early ray termination
        if (result.a > ERT_THRESHOLD) {
            t = tEnd;
        } else {
            t += tIncr;
        }
    }

    if (tDepth != -1.0) {
        tDepth = calculateDepthValue(camera, tDepth, texture(entryDepth, texCoords).x,
                                     texture(exitDepth, texCoords).x);
    } else {
        tDepth = 1.0;
    }

    gl_FragDepth = tDepth;
    return result;
}

void main() {
    vec2 texCoords = gl_FragCoord.xy * outportParameters.reciprocalDimensions;
    vec3 entryPoint = texture(entryColor, texCoords).rgb;
    vec3 exitPoint = texture(exitColor, texCoords).rgb;

    if (entryPoint == exitPoint) discard;

    vec4 color = rayTraversal(entryPoint, exitPoint, texCoords);
    FragData0 = color;
    PickingData = texture(entryPicking, texCoords);
}