/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

uniform vec3 pickingColor;
uniform ImageParameters outportParameters;

#ifdef BACKGROUND_AVAILABLE
uniform ImageParameters bgParameters;
uniform sampler2D bgColor;
uniform sampler2D bgPicking;
uniform sampler2D bgDepth;
#endif

uniform sampler2D inport;

void main() {
    const vec2 textureSize = textureSize(inport, 0);

    vec2 texCoords = gl_FragCoord.xy;
    texCoords.y = outportParameters.dimensions.y - texCoords.y;
    texCoords = texCoords / textureSize;

    vec4 color = texture(inport, texCoords);
    #ifdef SwizzleColor
    color = color.bgra;
    #endif



    #ifdef BACKGROUND_AVAILABLE
    const vec2 bgTexCoords = gl_FragCoord.xy * outportParameters.reciprocalDimensions;
    vec4 bg = texture(bgColor, bgTexCoords);

    if (color.a > 0) {
        color.rgb *= color.a;
        color += (1.0 - color.a) * bg;

        FragData0 = color;
        PickingData = vec4(pickingColor, 1.0);
    } else {
        PickingData = texture(bgPicking, bgTexCoords);
        FragData0 = bg;
    }
    gl_FragDepth = texture(bgDepth, bgTexCoords).r;


    #else
    

    if (color.a > 0) {
        FragData0 = color;
        PickingData = vec4(pickingColor, 1.0);
    } else {
        FragData0 = vec4(0.0, 0.0, 0.0, 0.0);
        PickingData = vec4(0.0, 0.0, 0.0, 0.0);
    }
    gl_FragDepth = 0.0;
    #endif
}
