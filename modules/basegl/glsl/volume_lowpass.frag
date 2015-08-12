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
#include "utils/sampler3d.glsl"

uniform sampler3D volume;
uniform VolumeParameters volumeParameters;
uniform int kernelSize;

uniform float inv2Sigma;

in vec4 texCoord_;

void main() {
    int k2 = kernelSize / 2;
    vec4 value = vec4(0,0,0,0);
    float tot_weight = 0;
    for(int z = -k2;z < k2;z++)for(int y = -k2;y < k2;y++)for(int x = -k2;x < k2;x++){
		float w = 1.0;
		#ifdef GAUSSIAN
		vec3 p = vec3(x,y,z)/vec3(k2);
		float l = dot(p,p);
		w = exp(-l*inv2Sigma);
		#endif
        value += w*getVoxel(volume, volumeParameters, texCoord_.xyz + (vec3(x, y, z) * volumeParameters.reciprocalDimensions));
		tot_weight += w;

    }
    vec4 voxel = value / tot_weight;
   
    FragData0 = voxel;
}