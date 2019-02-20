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

in vec4 gColor;
in vec2 gPos;
in float gR;
flat in vec4 pickColor_;

uniform int circle = 1;
uniform float borderWidth = 1;
uniform vec4 borderColor;

uniform float antialiasing = 1.5; // [pixel]

void main(void) {
    float r = 0;
    if (circle == 1) {
        r = length(gPos);
    } else {
        r = max(abs(gPos.x), abs(gPos.y));
    }
    if (r > gR + antialiasing) {
        discard;
    }

    float innerGlyphRadius = gR - borderWidth;

    // pseudo antialiasing with the help of the alpha channel
    // i.e. smooth transition between center and border, and smooth alpha fall-off at the outer rim
    vec4 color = gColor;
    if (borderWidth > 0.0) {
        float borderValue = clamp(mix(0.0, 1.0, (r - innerGlyphRadius) / 1.0), 0.0, 1.0);
        color = mix(color, borderColor, borderValue);
    }
    float borderAlpha = clamp(mix(1.0, 0.0, (r - gR) / antialiasing), 0.0, 1.0);

    color.rgb *= color.a;
    FragData0 = color * borderAlpha;
    //FragData0 = pickColor_;
    PickingData = pickColor_;
}
