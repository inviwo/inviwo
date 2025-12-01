/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#if !defined(KERNEL)
#  define KERNEL(s) box(s)
#  if !defined(KERNEL_NORMALIZATION)
#  define KERNEL_NORMALIZATION
#  endif
#endif
#if !defined(INTEGRATION_DIRECTION)
#  define INTEGRATION_DIRECTION 0
#endif

uniform sampler2D inport;
uniform ImageParameters inportParameters;

uniform sampler2D noiseTexture;

uniform int samples;
uniform float stepLength;
uniform mat2 invBasis = mat2(1);

uniform bool useRK4;

uniform bool postProcessing = false;
uniform bool intensityMapping = false;
uniform float brightness = 0.0;  // in [-1, 1]
uniform float contrast = 0.0;    // in [-1, 1]
uniform float gamma = 1.0;

in vec3 texCoord_;

vec2 euler(in vec2 pos) {
    vec2 v = getValueTexel(inport, inportParameters, pos).xy;
#if defined(NORMALIZATION)
    v = normalize(v);
#endif
    return v;
}

vec2 rungekutta4(in vec2 p0, in vec2 v0, float stepsize) {
    vec2 p1 = p0 + v0 * stepsize * 0.5;
    vec2 v1 = euler(p1);

    vec2 p2 = p0 + v1 * stepsize * 0.5;
    vec2 v2 = euler(p2);

    vec2 p3 = p0 + v2 * stepsize;
    vec2 v3 = euler(p3);

    return (v0 + 2.0 * (v1 + v2) + v3) / 6.0;
}

bool insideUnitSquare(in vec2 pos) {
    return pos == clamp(pos, vec2(0), vec2(1));
}

float box(in int step) {
    return 1.0;
}

float gaussian(in int step) {
    float weight = 2.0 / float(samples);
#if INTEGRATION_DIRECTION == 0
    weight *= 0.5;
#endif

    float dist = step / float(samples);
    float distSq = dot(dist, dist);
    float kernelSizeSq = 1.0 / 9.0;
    return clamp(exp(-0.693147180559945 * distSq / kernelSizeSq) * weight, 0.0, 1.0);
}

float integration(in vec2 pos, in int steps, in float stepSize, inout int stepCount) {
    float density = 0.0;
    int integrationSteps = 1;
    for (int i = 0; i < steps; ++i) {
        vec2 v = euler(pos);
#if defined(USE_RUNGEKUTTA)
        v = rungekutta4(pos, v, stepSize);
#endif
        v *= stepSize;
        pos += invBasis * v;
        // TODO: consider texture repeat?
        if (!insideUnitSquare(pos)) {
            break;
        }
        density += texture(noiseTexture, pos).x * KERNEL(i);
        ++stepCount;
    }
    return density;
}

void main() {
    vec2 pos = texCoord_.xy;
    float density = texture(noiseTexture, pos).x * KERNEL(0);

    float speed = length(invBasis * getValueTexel(inport, inportParameters, pos).xy);
    if (speed > 1.0e-8) {
        int stepCount = 1;
#if INTEGRATION_DIRECTION >= 0
        density += integration(pos, samples, stepLength, stepCount);
#endif
#if INTEGRATION_DIRECTION <= 0
        density += integration(pos, samples, -stepLength, stepCount);
#endif

#if defined(KERNEL_NORMALIZATION)
        density /= stepCount;
#endif
    }

    if (postProcessing) {
        density = mix(density, pow(density, 5.0 / pow(density + 1.0, 4)), intensityMapping);
        // brightness-contrast
        density = clamp((density - 0.5) * (contrast + 1.0) + 0.5 + brightness, 0, 1);
        // gamma correction
        density = pow(density, gamma);
    }

    FragData0 = vec4(density);
}
