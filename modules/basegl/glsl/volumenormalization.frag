/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

uniform VolumeParameters volumeParameters;
uniform sampler3D volume;

in vec4 texCoord_;

void main() {

    vec4 original = texture(volume, texCoord_.xyz);
    float l, m, n, o;

#ifdef NORMALIZE_CHANNEL_0
    l = getNormalizedVoxelChannel(volume, volumeParameters, texCoord_.xyz, 0);
#else
    l = original[0];
#endif

#ifdef NORMALIZE_CHANNEL_1
    m = getNormalizedVoxelChannel(volume, volumeParameters, texCoord_.xyz, 1);
#else
    m  = original[1];
#endif

#ifdef NORMALIZE_CHANNEL_2
    n = getNormalizedVoxelChannel(volume, volumeParameters, texCoord_.xyz, 2);
#else
    n = original[2];
#endif

#ifdef NORMALIZE_CHANNEL_3
    o = getNormalizedVoxelChannel(volume, volumeParameters, texCoord_.xyz, 3);
#else
    o = original[3];
#endif

    FragData0 = vec4(l, m, n, o);
}
