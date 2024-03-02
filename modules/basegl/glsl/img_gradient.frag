/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

uniform sampler2D inport;
uniform ImageParameters inportParameters;
uniform ImageParameters outportParameters;

uniform vec2 worldSpaceGradientSpacing = vec2(1.0);
uniform mat2 textureSpaceGradientSpacing = mat2(1.0);
uniform int channel = 0;
uniform bool renormalization = true;

float partialDiff(in vec2 texcoord, in vec2 gradientTextureSpacing, in float gradientWorldSpacing, in int component) {
    float fds = getNormalizedTexel(inport, inportParameters, texcoord + gradientTextureSpacing)[component]
        - getNormalizedTexel(inport, inportParameters, texcoord - gradientTextureSpacing)[component];
    return fds / gradientWorldSpacing * 0.5;
}

void main() {
    vec2 texCoords = gl_FragCoord.xy * outportParameters.reciprocalDimensions;

    float fdx = partialDiff(texCoords.xy, textureSpaceGradientSpacing[0], worldSpaceGradientSpacing[0], channel);
    float fdy = partialDiff(texCoords.xy, textureSpaceGradientSpacing[1], worldSpaceGradientSpacing[1], channel);

    FragData0 = vec4(fdx, fdy, 0, 0);
}