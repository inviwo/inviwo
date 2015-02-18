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

uniform float samplingRate_;
uniform float isoValue_;

uniform sampler2D transferFuncC1_;
uniform sampler2D transferFuncC2_;
uniform sampler2D transferFuncC3_;
uniform sampler2D transferFuncC4_;

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
    mat4x3 gradients;
    vec3 toCameraDir = normalize(camera_.position - (volumeParameters_.textureToWorld * vec4(entryPoint, 1.0)).xyz);
    while (t < tEnd) {
        samplePos = entryPoint + t * rayDirection;
        voxel = getNormalizedVoxel(volume_, volumeParameters_, samplePos);
        
		gradients = COMPUTE_ALL_GRADIENTS(voxel, volume_, volumeParameters_, samplePos);
        // World space position
        vec3 worldSpacePosition = (volumeParameters_.textureToWorld * vec4(samplePos, 1.0)).xyz;
        // macro defined in MultichannelRaycaster::initializeResources()            
		SAMPLE_CHANNELS

        // early ray termination
        if (result.a > ERT_THRESHOLD) {
            t = tEnd;
        } else {
            t += tIncr;
		}
    }

    if (tDepth != -1.0) {
        tDepth = calculateDepthValue(camera_, tDepth, texture(entryDepthTex_, texCoords).z,
                                     texture(exitDepthTex_, texCoords).z);
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