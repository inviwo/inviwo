/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

uniform sampler2D texColor0_;
uniform sampler2D texDepth0_;
uniform sampler2D texPicking0_;

uniform sampler2D texColor1_;
uniform sampler2D texDepth1_;
uniform sampler2D texPicking1_;
uniform vec2 screenDimRCP_;

void main() {
    vec2 texCoords = gl_FragCoord.xy * screenDimRCP_;
    vec4 color0 = texture(texColor0_, texCoords);
    vec4 picking0 = texture(texPicking0_, texCoords);
    float depth0 = texture(texDepth0_, texCoords).r;
    vec4 color1 = texture(texColor1_, texCoords);
    vec4 picking1 = texture(texPicking1_, texCoords);
    float depth1 = texture(texDepth1_, texCoords).r;
    vec4 colorOut;
    vec4 pickingOut;
    float depthOut;

    if (depth1 <= depth0) {
        colorOut.rgb = color1.rgb * color1.a + color0.rgb * (1.0 - color1.a);
        colorOut.a = color1.a + color0.a * (1.0 - color1.a);
        pickingOut = (picking1.a > 0 ? picking1 : (color1.a < 0.95 ? picking0 : vec4(0.0)));
        depthOut = depth1;
    } else {
        colorOut.rgb = color0.rgb * color0.a + color1.rgb * (1.0 - color0.a);
        colorOut.a = color0.a + color1.a * (1.0 - color0.a);
        pickingOut = (picking0.a > 0 ? picking0 : (color0.a < 0.95 ? picking1 : vec4(0.0)));
        depthOut = depth0;
    }

    FragData0 = colorOut;
    PickingData = pickingOut;
    gl_FragDepth = depthOut;
}