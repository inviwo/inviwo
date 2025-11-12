/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#if !defined(KERNEL)
#define KERNEL(s) box(s)
#endif
#if !defined(INTEGRATION_DIRECTION)
#define INTEGRATION_DIRECTION 0
#endif

uniform sampler3D volume;  // noise
uniform VolumeParameters volumeParameters;

uniform sampler3D vectorField;
uniform VolumeParameters vectorFieldParameters;

uniform float velocityScale;
uniform float alphaScale;

uniform int samples;
uniform float stepLength;
uniform mat3 invBasis = mat3(1);

in vec4 texCoord_;
in vec3 outputTexCoord_;

uniform float noiseRepeat = 5.f;

bool insideUnitCube(in vec3 pos) {
    return pos == clamp(pos, vec3(0), vec3(1));
}

float box(in int step) {
    return 1.0;
}

float gaussian(in int step) {
    float dist = step / float(samples);
    float distSq = dot(dist, dist);
    float kernelSizeSq = 1.0 / 9.0;
    return  clamp(exp(-0.693147180559945 * distSq / kernelSizeSq), 0.0, 1.0);
}

float integration(in vec3 pos, in int steps, in int integrationDirection) {
    float density = 0.0;
    for (int i = 0; i < steps; ++i) {
        vec3 v = getValueVoxel(vectorField, vectorFieldParameters, pos).xyz;
#if defined(NORMALIZATION)
        v = normalize(v);
#endif
        v *= stepLength * integrationDirection;
        pos += invBasis * v;
        // TODO: consider texture repeat?
        if (!insideUnitCube(pos)) {
            break;
        }
        density += texture(volume, mod(pos * noiseRepeat, 1)).x * KERNEL(i);
    }
    return density;
}

void main(void) {
    vec3 pos = outputTexCoord_.xyz;
    float density = texture(volume, mod(pos * noiseRepeat, 1)).x * KERNEL(0);

    float speed = length(invBasis * getValueVoxel(vectorField, vectorFieldParameters, pos).xyz);
    if (speed > 1.0e-8) {
#if INTEGRATION_DIRECTION >= 0
        density += integration(pos, samples, +1);
#endif
#if INTEGRATION_DIRECTION <= 0
        density += integration(pos, samples, -1);
#endif
    }

    FragData0 = vec4(density * alphaScale);
}
