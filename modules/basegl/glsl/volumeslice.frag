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
#include "utils/sampler3d.glsl"
#include "utils/classification.glsl"

uniform sampler3D volume;
uniform VolumeParameters volumeParameters;

uniform sampler2D transferFunction;

uniform mat4 sliceRotation; // Rotates around slice axis (offset to center point)
uniform float slice;

uniform vec4 fillColor;

uniform float alphaOffset = 0.0;

in vec3 texCoord_;

void main() {
    // Rotate around center and translate back to origin
    vec3 samplePos = (sliceRotation * vec4(texCoord_.x, texCoord_.y, slice, 1.0)).xyz;
#ifdef COLOR_FILL_ENABLED
    if ((samplePos.x < 0 || samplePos.x >= 1) || (samplePos.y < 0 || samplePos.y >= 1) || (samplePos.z < 0 || samplePos.z >= 1)) {
       FragData0 = fillColor;
       return;
    }
#endif
  
    vec4 voxel = getNormalizedVoxel(volume, volumeParameters, samplePos);

#ifdef TF_MAPPING_ENABLED
    voxel = applyTF(transferFunction, voxel);
    voxel.a += alphaOffset;
#endif

    FragData0 = voxel;
}