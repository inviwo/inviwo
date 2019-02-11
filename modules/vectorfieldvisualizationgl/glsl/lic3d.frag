/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

uniform sampler3D volume;  // noise
uniform VolumeParameters volumeParameters;

uniform sampler3D vectorField;
uniform VolumeParameters vectorFieldParameters;

uniform sampler2D tf;
uniform float velocityScale;
uniform float alphaScale;

uniform int samples;
uniform float stepLength;
uniform mat3 invBasis;
uniform bool normalizeVectors;
uniform bool intensityMapping;

in vec4 texCoord_;

#define noise volume
#define noiseParameters volumeParameters

uniform float noiseRepeat = 5.f;

void main(void) {
    float v = texture(noise, mod(texCoord_.xyz * noiseRepeat, 1)).r;
    float voxelVelo = length(texture(vectorField, texCoord_.xyz).xyz);

    if (voxelVelo < 0.000001) {
        discard;
        return;
    }

    int c = 1;
    vec3 posF;

    posF = texCoord_.xyz;
    for (int i = 0; i < samples / 2; i++) {
        vec3 V0 = texture(vectorField, posF).rgb;
        if (normalizeVectors) {
            V0 = normalize(V0);
        }

        posF += invBasis * (V0 * stepLength);

        if (any(lessThan(posF, vec3(0)))) break;
        if (any(greaterThan(posF, vec3(1)))) break;

        v += texture(noise, mod(posF * noiseRepeat, 1)).r;
        c += 1;
    }
    posF = texCoord_.xyz;
    for (int i = 0; i < samples / 2; i++) {
        vec3 V0 = texture(vectorField, posF).rgb;
        if (normalizeVectors) {
            V0 = normalize(V0);
        }

        posF -= invBasis * (V0 * stepLength);

        if (any(lessThan(posF, vec3(0)))) break;
        if (any(greaterThan(posF, vec3(1)))) break;

        v += texture(noise, mod(posF * noiseRepeat, 1)).r;
        c += 1;
    }

    v /= c;

    if (intensityMapping) {
        v = pow(v, (4.0 / pow((v + 1.0), 4)));
    }

    if (v < 0.6 && false) {
        discard;
        return;
    }

    vec4 color = applyTF(tf, voxelVelo / velocityScale);
    color.a *= v * alphaScale;

    FragData0 = color;
}
