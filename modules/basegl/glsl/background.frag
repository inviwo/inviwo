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

uniform sampler2D inputTex_;
//uniform sampler2D inputDepth_;
uniform ImageParameters outportParameters_;

uniform ivec2 checkerBoardSize_;
uniform vec4 color1_;
uniform vec4 color2_;

vec4 checkerBoard(vec2 texCoords) {
    int a = int(gl_FragCoord.x) / checkerBoardSize_.x;
    int b = int(outportParameters_.dimensions.y - gl_FragCoord.y) / checkerBoardSize_.y;
    
    return  (((a + 1) % 2)*((b + 1) % 2) + (a % 2)*(b % 2)) * color1_ +
            (((a + 1) % 2)*(b % 2) + (a % 2)*((b + 1) % 2)) * color2_;
}

vec4 linearGradient(vec2 texCoords) {
    return texCoords.y * color1_ + (1.0 - texCoords.y) * color2_;
}

void main() {  
    vec2 texCoords = gl_FragCoord.xy * outportParameters_.reciprocalDimensions;
    vec4 srcColor = SRC_COLOR;
    vec4 backgroundColor = BACKGROUND_STYLE_FUNCTION;
    vec4 resultColor;
    resultColor.rgb = srcColor.rgb + backgroundColor.rgb * backgroundColor.a * (1.0 - srcColor.a);
    resultColor.a = srcColor.a + backgroundColor.a * (1.0 - srcColor.a);
    FragData0 = resultColor;
    //gl_FragDepth = texture(inputDepth_, texCoords).x;

}