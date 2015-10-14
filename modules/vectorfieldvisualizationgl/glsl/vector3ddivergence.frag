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

#include "utils/sampler3d.glsl"
#include "utils/gradients.glsl"

uniform sampler3D volume;
uniform VolumeParameters volumeParameters;

in vec4 texCoord_;

void main() {
    vec3 ox = vec3(volumeParameters.reciprocalDimensions.x, 0, 0);
    vec3 oy = vec3(0, volumeParameters.reciprocalDimensions.y, 0);
    vec3 oz = vec3(0, 0, volumeParameters.reciprocalDimensions.z);

    vec3 Fx = (getVoxel(volume, volumeParameters, texCoord_.xyz + ox).xyz -
               getVoxel(volume, volumeParameters, texCoord_.xyz - ox).xyz) /
              2.0f;
    vec3 Fy = (getVoxel(volume, volumeParameters, texCoord_.xyz + oy).xyz -
               getVoxel(volume, volumeParameters, texCoord_.xyz - oy).xyz) /
              2.0f;
    vec3 Fz = (getVoxel(volume, volumeParameters, texCoord_.xyz + oz).xyz -
               getVoxel(volume, volumeParameters, texCoord_.xyz - oz).xyz) /
              2.0f;
              
    FragData0 = vec4(Fx.x + Fy.y + Fz.z);
}