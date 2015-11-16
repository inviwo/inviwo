/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

uniform sampler3D volume_;
uniform VolumeParameters volumeParameters_;

uniform sampler3D lightVolume_;
uniform VolumeParameters lightVolumeParameters_;

uniform sampler2D transferFunc_;

#ifdef POINT_LIGHT
uniform vec3 lightPos_;
uniform mat4 permutedLightMatrix_;
#else
uniform vec4 permutedLightDirection_;
#endif

in vec4 texCoord_;
in vec4 permutedTexCoord_;

//Perform light propagation
vec4 propagateLight(in vec3 coord, in vec3 coordPerm) {
    //Retrieve voxel color
    vec4 voxel = getNormalizedVoxel(volume_, volumeParameters_, coordPerm);
    vec4 color = applyTF(transferFunc_, voxel);
    //Calculate previous permuted coordinate
#ifdef POINT_LIGHT
    vec4 permLightDir = permutedLightMatrix_ * vec4(normalize(coordPerm - lightPos_), 1.0);
    vec3 previousPermutedCoord = vec3(coord.xy - permLightDir.xy * lightVolumeParameters_.reciprocalDimensions.z/max(1e-4,abs(permLightDir.z)),
                                      coord.z - lightVolumeParameters_.reciprocalDimensions.z);
#else
    vec3 previousPermutedCoord = vec3(coord.xy - permutedLightDirection_.xy * lightVolumeParameters_.reciprocalDimensions.z/max(1e-4,abs(permutedLightDirection_.z)),
                                      coord.z - lightVolumeParameters_.reciprocalDimensions.z);
#endif
    
    float dt = distance(coord, previousPermutedCoord);
    color.a = 1.0 - pow(1.0 - color.a, dt * REF_SAMPLING_INTERVAL);
    //Retrieve previous light value
    vec4 lightVoxel = getNormalizedVoxel(lightVolume_, lightVolumeParameters_, previousPermutedCoord);
    //Return newly calculated propagate light value
#ifdef SUPPORT_LIGHT_COLOR
    vec4 newCol = vec4((1.0 - color.a)*lightVoxel.a);
    newCol.rgb = (1.0 - color.a)*lightVoxel.rgb;
#else
    vec4 newCol = vec4((1.0 - color.a)*lightVoxel.r);
#endif
    return newCol;
}

void main() {
    FragData0 = propagateLight(texCoord_.xyz, permutedTexCoord_.xyz);
}