/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

uniform sampler2D transferFunction;

uniform ImageParameters entryParameters;
uniform sampler2D entryColor;
uniform sampler2D entryDepth;
uniform sampler2D entryPicking;

uniform ImageParameters exitParameters;
uniform sampler2D exitColor;
uniform sampler2D exitDepth;

uniform ImageParameters bgParameters;
uniform sampler2D bgColor;
uniform sampler2D bgPicking;
uniform sampler2D bgDepth;

uniform ImageParameters outportParameters;

uniform LightParameters lighting;
uniform CameraParameters camera;
uniform VolumeIndicatorParameters positionindicator;
uniform RaycastingParameters raycaster;

uniform int channel;

uniform int isoValueCount = 0;
uniform float isoValues[10];
uniform vec4 isoValueColors[10];

#define ERT_THRESHOLD 0.99  // threshold for early ray termination



vec4 rayTraversal(vec3 entryPoint, vec3 exitPoint, vec2 texCoords, float backgroundDepth) {
    vec4 result = vec4(0.0);
    vec3 rayDirection = exitPoint - entryPoint;
    float tEnd = length(rayDirection);
    float tIncr = min(
        tEnd, tEnd / (raycaster.samplingRate * length(rayDirection * volumeParameters.dimensions)));
    float samples = ceil(tEnd / tIncr);
    tIncr = tEnd / samples;
    float t = 0.5f * tIncr;
    rayDirection = normalize(rayDirection);
    float tDepth = -1.0;
    vec4 color;
    vec4 voxel;
    vec3 samplePos;
    vec3 toCameraDir = normalize((volumeParameters.textureToWorld * vec4(entryPoint, 1.0) -
                                  volumeParameters.textureToWorld * vec4(exitPoint, 1.0))
                                     .xyz);

    vec4 backgroundColor = vec4(0);
    float bgTDepth = -1;
#ifdef BACKGROUND_AVAILABLE
    backgroundColor = texture(bgColor, texCoords);
    // convert to raycasting depth
    bgTDepth = tEnd * calculateTValueFromDepthValue(
        camera, backgroundDepth, texture(entryDepth, texCoords).x, texture(exitDepth, texCoords).x);        

    if (bgTDepth < 0) {
        result = backgroundColor;
    }
#endif // BACKGROUND_AVAILABLE

    // used for isosurface computation
    float prevSample = getNormalizedVoxel(volume, volumeParameters, entryPoint + t * rayDirection)[channel];

    while (t < tEnd) {
        samplePos = entryPoint + t * rayDirection;
        voxel = getNormalizedVoxel(volume, volumeParameters, samplePos);

        // check for isosurfaces
        float currentSample = voxel[channel];
        float sampleDelta = (currentSample - prevSample);
        if (sampleDelta > 0) {
            for (int i = 0; i < isoValueCount; ++i) {
                // found isosurface if differences between current/prev sample and isovalue have different signs
                if ((isoValues[i] - currentSample) * (isoValues[i] - prevSample) < 0) {
                    // apply linear interpolation
                    float a = (currentSample - isoValues[i]) / sampleDelta;
                    vec3 isopos = samplePos - tIncr * a * rayDirection;
                    vec3 gradient = COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volumeParameters, isopos, channel);
                    gradient = normalize(gradient);


                    // two-sided
                    if (dot(gradient, rayDirection) < 0) {
                        gradient = -gradient;
                    }

                    vec4 surfaceColor = isoValueColors[i];
                    vec3 isoposWorld = (volumeParameters.textureToWorld * vec4(isopos, 1.0)).xyz;
                    surfaceColor.rgb = APPLY_LIGHTING(lighting, surfaceColor.rgb, surfaceColor.rgb, vec3(1.0),
                                       isoposWorld, -gradient, toCameraDir);

                    //surfaceColor.rgb = vec3(dot(gradient, normalize(lighting.position - isoposWorld)));

                    result = APPLY_COMPOSITING(result, surfaceColor, isopos, voxel, gradient, camera,
                                       raycaster.isoValue, t - tIncr * a, tDepth, tIncr);                    
                }
            }
        } else {
            // previous sample is larger, iterate in reverse over isovalues
            for (int i = isoValueCount - 1; i >= 0; --i) {
                // found isosurface if differences between current/prev sample and isovalue have different signs
                if ((isoValues[i] - currentSample) * (isoValues[i] - prevSample) < 0) {
                    // apply linear interpolation
                    float a = (currentSample - isoValues[i]) / sampleDelta;
                    vec3 isopos = samplePos - tIncr * a * rayDirection;
                    vec3 gradient = COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volumeParameters, isopos, channel);
                    gradient = normalize(gradient);


                    // two-sided
                    if (dot(gradient, rayDirection) < 0) {
                        gradient = -gradient;
                    }

                    vec4 surfaceColor = isoValueColors[i];
                    vec3 isoposWorld = (volumeParameters.textureToWorld * vec4(isopos, 1.0)).xyz;
                    surfaceColor.rgb = APPLY_LIGHTING(lighting, surfaceColor.rgb, surfaceColor.rgb, vec3(1.0),
                                       isoposWorld, -gradient, toCameraDir);

                    //surfaceColor.rgb = vec3(dot(gradient, normalize(lighting.position - isoposWorld)));

                    result = APPLY_COMPOSITING(result, surfaceColor, isopos, voxel, gradient, camera,
                                       raycaster.isoValue, t - tIncr * a, tDepth, tIncr);                    
                }
            }
        }

        result = DRAW_BACKGROUND(result, t, tIncr, backgroundColor, bgTDepth, tDepth);
        result = DRAW_PLANES(result, samplePos, rayDirection, tIncr, positionindicator, t, tDepth);

        color = APPLY_CHANNEL_CLASSIFICATION(transferFunction, voxel, channel);
        if (color.a > 0) {
            vec3 gradient =
                COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volumeParameters, samplePos, channel);
            gradient = normalize(gradient);

            // World space position
            vec3 worldSpacePosition = (volumeParameters.textureToWorld * vec4(samplePos, 1.0)).xyz;
            // Note that the gradient is reversed since we define the normal of a surface as
            // the direction towards a lower intensity medium (gradient points in the increasing
            // direction)
            color.rgb = APPLY_LIGHTING(lighting, color.rgb, color.rgb, vec3(1.0),
                                       worldSpacePosition, -gradient, toCameraDir);

            result = APPLY_COMPOSITING(result, color, samplePos, voxel, gradient, camera,
                                       raycaster.isoValue, t, tDepth, tIncr);
        }
        // early ray termination
        if (result.a > ERT_THRESHOLD) {
            t = tEnd;
        } else {
            t += tIncr;
        }
    }

    if (bgTDepth > tEnd) {
        result =
            DRAW_BACKGROUND(result, bgTDepth, tIncr, backgroundColor, bgTDepth, tDepth);
    }

    if (tDepth != -1.0) {
        tDepth = calculateDepthValue(camera, tDepth / tEnd, texture(entryDepth, texCoords).x,
                                     texture(exitDepth, texCoords).x);

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

    vec4 color = vec4(0);

    float backgroundDepth = 1;
#ifdef BACKGROUND_AVAILABLE
    color = texture(bgColor, texCoords);
    gl_FragDepth = backgroundDepth = texture(bgDepth, texCoords).x;
    PickingData = texture(bgPicking, texCoords);
#else // BACKGROUND_AVAILABLE
    PickingData = vec4(0);
    if (entryPoint == exitPoint) {
        discard;
    }
#endif // BACKGROUND_AVAILABLE
    if (entryPoint != exitPoint) {
        color = rayTraversal(entryPoint, exitPoint, texCoords, backgroundDepth);
    }
    FragData0 = color;
}