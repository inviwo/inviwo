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

uniform sampler3D volume_;

uniform VolumeParameters volumeParameters_;

uniform int kernelSize_;

in vec4 texCoord_;


void main() {
    int k2 = kernelSize_ / 2;
    vec4 value = vec4(0, 0, 0, 0);

    value -= 6.0*getNormalizedVoxel(volume_,
                         volumeParameters_,
                         texCoord_.xyz + (vec3(0, 0, 0) * volumeParameters_.reciprocalDimensions));

    for (int x = -1; x <= 1; x+=2) {
        value += getNormalizedVoxel(volume_,
                          volumeParameters_, 
                          texCoord_.xyz + (vec3(x, 0, 0) * volumeParameters_.reciprocalDimensions));
        value += getNormalizedVoxel(volume_,
                          volumeParameters_,
                          texCoord_.xyz + (vec3(0, x, 0) * volumeParameters_.reciprocalDimensions));
        value += getNormalizedVoxel(volume_,
                          volumeParameters_,
                          texCoord_.xyz + (vec3(0, 0, x) * volumeParameters_.reciprocalDimensions));
    }
    
    vec4 voxel = value;

    FragData0 = voxel;
}