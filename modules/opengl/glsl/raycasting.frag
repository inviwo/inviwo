/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include "utils/isosurface.glsl"

uniform VolumeParameters volumeParameters;
uniform sampler3D volume;

uniform sampler2D transferFunction;

uniform ImageParameters entryParameters;
uniform sampler2D entryColor;
uniform sampler2D entryDepth;
uniform sampler2D entryPicking;
uniform sampler2D entryNormal;

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
uniform IsovalueParameters isovalues;

uniform bool useNormals = false;

uniform int channel;

#define ERT_THRESHOLD 0.99  // threshold for early ray termination

#if (!defined(INCLUDE_DVR) && !defined(INCLUDE_ISOSURFACES))
#  define INCLUDE_DVR
#endif


vec4 rayTraversal(vec3 entryPoint, vec3 exitPoint, vec2 texCoords, float backgroundDepth, vec3 entryNormal) {
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
    voxel = getNormalizedVoxel(volume, volumeParameters, entryPoint + t * rayDirection);


    bool first = true;
    while (t < tEnd) {
        samplePos = entryPoint + t * rayDirection;
        vec4 previousVoxel = voxel;
        voxel = getNormalizedVoxel(volume, volumeParameters, samplePos);

        // check for isosurfaces
#if defined(ISOSURFACE_ENABLED) && defined(INCLUDE_ISOSURFACES)
        // make sure that tIncr has the correct length since drawIsoSurface will modify it
        tIncr = tEnd / samples;
        result = drawIsosurfaces(result, isovalues, voxel, previousVoxel, 
                                 volume, volumeParameters, channel, transferFunction, camera, lighting, 
                                 samplePos, rayDirection, toCameraDir, t, tIncr, tDepth);
#endif // ISOSURFACE_ENABLED

#if defined(BACKGROUND_AVAILABLE)
        result = DRAW_BACKGROUND(result, t, tIncr, backgroundColor, bgTDepth, tDepth);
#endif // BACKGROUND_AVAILABLE

#if defined(PLANES_ENABLED)
        result = DRAW_PLANES(result, samplePos, rayDirection, tIncr, positionindicator, t, tDepth);
#endif // #if defined(PLANES_ENABLED)

#if defined(INCLUDE_DVR)
        color = APPLY_CHANNEL_CLASSIFICATION(transferFunction, voxel, channel);
        if (color.a > 0) {
            
            vec3 gradient;
            if (first && useNormals) {
                gradient = -entryNormal;
            } else {
                gradient = COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume, volumeParameters, samplePos, channel);
                gradient = normalize(gradient);

                // make sure that the gradient always points away from zero
                gradient *= sign(voxel[channel] / (1.0 - volumeParameters.formatScaling) - volumeParameters.formatOffset);
            }

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
#endif // INCLUDE_DVR

        // early ray termination
        if (result.a > ERT_THRESHOLD) {
            t = tEnd;
        } else {
#if defined(ISOSURFACE_ENABLED) && defined(INCLUDE_ISOSURFACES)
            // make sure that tIncr has the correct length since drawIsoSurface will modify it
            tIncr = tEnd / samples;
#endif // ISOSURFACE_ENABLED
            t += tIncr;
        }
        first = false;
    }

    // composite background if lying beyond the last volume sample, which is located at tEnd - tIncr*0.5
    if (bgTDepth > tEnd - tIncr * 0.5) {
        result =
            DRAW_BACKGROUND(result, bgTDepth, tIncr * 0.5, backgroundColor, bgTDepth, tDepth);
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

    vec3 normal = useNormals ? texture(entryNormal, texCoords).xyz : vec3(0,0,0);
    if (entryPoint != exitPoint) {
        color = rayTraversal(entryPoint, exitPoint, texCoords, backgroundDepth, normal);
    }
    FragData0 = color;
}
