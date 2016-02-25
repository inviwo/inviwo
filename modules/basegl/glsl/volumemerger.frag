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

uniform sampler3D volume;
uniform VolumeParameters volumeParameters;
uniform sampler3D vol2;
uniform VolumeParameters vol2Parameters;
uniform sampler3D vol3;
uniform VolumeParameters vol3Parameters;
uniform sampler3D vol4;
uniform VolumeParameters vol4Parameters;

in vec4 texCoord_;

 

void main() {
    vec4 outv = vec4(0.0,0.0,0.0,0.0);
    outv[0] = getVoxel(volume,volumeParameters, texCoord_.xyz).r;
    int i = 1;
#ifdef HAS_VOL2
    outv[i++] = getVoxel(vol2,vol2Parameters, texCoord_.xyz).r;
#endif
#ifdef HAS_VOL3
    outv[i++] = getVoxel(vol3,vol3Parameters, texCoord_.xyz).r;
#endif
#ifdef HAS_VOL4
    outv[i++] = getVoxel(vol4,vol4Parameters, texCoord_.xyz).r;
#endif
    FragData0 = outv;
}