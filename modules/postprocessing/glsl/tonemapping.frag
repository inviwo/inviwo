/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef METHOD
#define METHOD 0
#endif

#include "utils/structs.glsl"

uniform sampler2D inport_;
uniform ImageParameters outportParameters_;

uniform float exposure = 1.0;
uniform float gamma = 2.2;

in vec2 texCoord;
out vec4 outColor;

// Many from here http://filmicworlds.com/blog/filmic-tonemapping-operators/

vec3 Uncharted2Tonemap(vec3 x) {
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec4 Uncharted2(vec2 tc) {
    const float W = 11.2;
    vec4 texColor = texture(inport_, tc);
    texColor.rgb *= exposure;  // Exposure Adjustment

    float ExposureBias = 2.0;
    vec3 curr = Uncharted2Tonemap(ExposureBias * texColor.rgb);

    vec3 whiteScale = 1.0 / Uncharted2Tonemap(vec3(W));
    vec3 color = curr * whiteScale;

    vec3 retColor = pow(color, vec3(1.0 / gamma));
    return clamp(vec4(retColor, texColor.a), 0, 1);
}

vec4 Reinhard(vec2 tc) {
    vec4 texColor = texture(inport_, tc);
    texColor.rgb *= exposure;  // Exposure Adjustment
    texColor.rgb = texColor.rgb / (1 + texColor.rgb);
    vec3 retColor = pow(texColor.rgb, vec3(1.0 / gamma));
    return clamp(vec4(retColor, texColor.a), 0, 1);
}

vec4 Gamma(vec2 tc) {
    vec4 texColor = texture(inport_, tc);
    texColor.rgb *= exposure;  // Exposure Adjustment
    vec3 retColor = pow(texColor.rgb, vec3(1.0 / gamma));
    return clamp(vec4(retColor, texColor.a), 0, 1);
}

//----------------------------------------------------------------------------------
void main() {
    vec2 texCoords = gl_FragCoord.xy * outportParameters_.reciprocalDimensions;
#ifdef METHOD

#if METHOD == 0
    vec4 color = texture(inport_, texCoords);
    outColor = clamp(vec4(color.rgb * exposure, color.a), 0, 1);
#elif METHOD == 1
    outColor = Gamma(texCoords);
#elif METHOD == 2
    outColor = Reinhard(texCoords);
#elif METHOD == 3
    outColor = Uncharted2(texCoords);
#endif

#else
    outColor = texture(inport_, texCoords);
#endif
}
