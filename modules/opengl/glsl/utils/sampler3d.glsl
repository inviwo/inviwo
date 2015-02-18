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

//
// Fetch texture data using texture coordinates [0,1]
//

// Return a the raw texture
vec4 getVoxel(sampler3D volume, VolumeParameters volumeParams, vec3 samplePos) {
    return (texture(volume, samplePos));
}
// Return a value mapped from data range [min,max] to [0,1]
vec4 getNormalizedVoxel(sampler3D volume, VolumeParameters volumeParams, vec3 samplePos) {
    return (texture(volume, samplePos) + volumeParams.formatOffset)
        * (1.0 - volumeParams.formatScaling);
}
// Return a value mapped from data range [min,max] to [-1,1]
vec4 getSignNormalizedVoxel(sampler3D volume, VolumeParameters volumeParams, vec3 samplePos) {
    return (texture(volume, samplePos) + volumeParams.signedFormatOffset)
        * (1.0 - volumeParams.signedFormatScaling);
}


//
// Fetch texture data using texture indices [0,N]
//

// Return a the raw texture
vec4 getVoxel(sampler3D volume, VolumeParameters volumeParams, ivec3 samplePos) {
#ifdef GLSL_VERSION_140
    return (texelFetch(volume, samplePos, 0) + volumeParams.formatOffset)
               * (1.0-volumeParams.formatScaling);
#else
    return (texture(volume, samplePos) + volumeParams.formatOffset)
               * (1.0-volumeParams.formatScaling);
#endif
}
// Return a value mapped from data range [min,max] to [0,1]
vec4 getNormalizedVoxel(sampler3D volume, VolumeParameters volumeParams, ivec3 samplePos) {
#ifdef GLSL_VERSION_140
    return (texelFetch(volume, samplePos, 0) + volumeParams.formatOffset)
        * (1.0 - volumeParams.formatScaling);
#else
    return (texture(volume, samplePos) + volumeParams.formatOffset)
        * (1.0 - volumeParams.formatScaling);
#endif
}
// Return a value mapped from data range [min,max] to [-1,1]
vec4 getSignNormalizedVoxel(sampler3D volume, VolumeParameters volumeParams, ivec3 samplePos) {
#ifdef GLSL_VERSION_140
    return (texelFetch(volume, samplePos, 0) + volumeParams.signedFormatOffset)
        * (1.0 - volumeParams.signedFormatScaling);
#else
    return (texture(volume, samplePos) + volumeParams.signedFormatOffset)
        * (1.0 - volumeParams.signedFormatScaling);
#endif
}