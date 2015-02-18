/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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


uniform VolumeParameters volumeParameters_;
uniform sampler3D volume_;

uniform sampler2D transferFunc_;

uniform ImageParameters entryParameters_;
uniform sampler2D entryColorTex_;
uniform sampler2D entryDepthTex_;

uniform ImageParameters exitParameters_;
uniform sampler2D exitColorTex_;
uniform sampler2D exitDepthTex_;

uniform ImageParameters outportParameters_;

uniform LightParameters light_;
uniform CameraParameters camera_;
uniform VolumeIndicatorParameters positionIndicator_;
uniform int channel_;

uniform float samplingRate_;
uniform float isoValue_;

#define ERT_THRESHOLD 0.99 // threshold for early ray termination



vec4 rayTraversal(vec3 entryPoint, vec3 exitPoint, vec2 texCoords) {
    vec4 result = vec4(0.0);
    vec3 rayDirection = exitPoint - entryPoint;
    float tEnd = length(rayDirection);
    float tIncr =
        min(tEnd, tEnd / (samplingRate_ * length(rayDirection * volumeParameters_.dimensions)));
    float samples = ceil(tEnd / tIncr);
    tIncr = tEnd / samples;
    float t = 0.5f * tIncr;
    rayDirection = normalize(rayDirection);
    float tDepth = -1.0;
    vec4 color;
    vec4 voxel;
    vec3 samplePos;
    vec3 toCameraDir = normalize(camera_.position - (volumeParameters_.textureToWorld * vec4(entryPoint, 1.0)).xyz);
    while (t < tEnd) {
        samplePos = entryPoint + t * rayDirection;
        voxel = getNormalizedVoxel(volume_, volumeParameters_, samplePos);
        
        vec3 gradient = COMPUTE_GRADIENT_FOR_CHANNEL(voxel, volume_, volumeParameters_, samplePos, channel_);
        gradient = normalize(gradient);
        color = APPLY_CLASSIFICATION(transferFunc_, voxel); 

        // World space position
        vec3 worldSpacePosition = (volumeParameters_.textureToWorld * vec4(samplePos, 1.0)).xyz;
        // Note that the gradient is reversed since we define the normal of a surface as
        // the direction towards a lower intensity medium (gradient points in the inreasing direction)
        color.rgb = APPLY_LIGHTING(light_, color.rgb, color.rgb, vec3(1.0), worldSpacePosition, -gradient, toCameraDir);

        result = DRAW_PLANES(result, samplePos, rayDirection, tIncr, positionIndicator_);
        result = APPLY_COMPOSITING(result, color, samplePos, voxel, gradient, camera_, isoValue_, 
                                   t, tDepth, tIncr);

        // early ray termination
        if (result.a > ERT_THRESHOLD) {
            t = tEnd;
        } else {
            t += tIncr;
		}
    }

    if (tDepth != -1.0) {
        tDepth = calculateDepthValue(camera_, tDepth, texture(entryDepthTex_, texCoords).x,
                                     texture(exitDepthTex_, texCoords).x);
    } else {
        tDepth = 1.0;
	}

    gl_FragDepth = tDepth;
    return result;
}




void main() {
    vec2 texCoords = gl_FragCoord.xy * outportParameters_.reciprocalDimensions;
    vec3 entryPoint = texture(entryColorTex_, texCoords).rgb;
    vec3 exitPoint = texture(exitColorTex_, texCoords).rgb;

    if (entryPoint == exitPoint) discard;

    vec4 color = rayTraversal(entryPoint, exitPoint, texCoords);
    FragData0 = color;
}