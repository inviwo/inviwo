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
#include "utils/raycastgeometry.glsl"


uniform VolumeParameters volumeParameters;
uniform sampler3D volume;

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

uniform sampler2D transferFunction1;
uniform sampler2D transferFunction2;
uniform sampler2D transferFunction3;
uniform sampler2D transferFunction4;

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
    mat4 color;
    vec4 voxel;
    vec3 samplePos;
    mat4x3 gradients;
    vec3 toCameraDir = normalize(camera.position - (volumeParameters.textureToWorld * vec4(entryPoint, 1.0)).xyz);
    while (t < tEnd) {
        samplePos = entryPoint + t * rayDirection;
        voxel = getNormalizedVoxel(volume, volumeParameters, samplePos);
        
        // macro defined in MultichannelRaycaster::initializeResources()
        // sets colors;
        SAMPLE_CHANNELS;

        if (color[0].a > 0 || color[1].a > 0 || color[2].a > 0 || color[3].a > 0) {
            // World space position
            vec3 worldSpacePosition = (volumeParameters.textureToWorld * vec4(samplePos, 1.0)).xyz;
            gradients = COMPUTE_ALL_GRADIENTS(voxel, volume, volumeParameters, samplePos);
            result = DRAW_PLANES(result, samplePos, rayDirection, tIncr, positionindicator);
            for (int i = 0; i < NUMBER_OF_CHANNELS; ++i) {
                color[i].rgb = APPLY_LIGHTING(lighting, color[i].rgb, color[i].rgb, vec3(1.0),
                    worldSpacePosition, normalize(-gradients[i]), toCameraDir);
                result = APPLY_COMPOSITING(result, color[i], samplePos, voxel,
                    gradients[i], camera, raycaster.isoValue, t, tDepth, tIncr);
            }
        } else {
            result = DRAW_PLANES(result, samplePos, rayDirection, tIncr, positionindicator);
        }

        // early ray termination
        if (result.a > ERT_THRESHOLD) {
            t = tEnd;
        } else {
            t += tIncr;
        }
    }

    if (tDepth != -1.0) {
        tDepth = calculateDepthValue(camera, tDepth/tEnd, texture(entryDepth, texCoords).x,
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