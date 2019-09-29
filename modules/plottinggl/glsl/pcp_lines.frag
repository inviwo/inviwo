/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

in vec4 lPickColor;
in float lScalarMeta;
in float lFalloffAlpha;

uniform bool additiveBlend = true;
uniform bool subtractiveBelnding = false;

uniform vec4 color;
uniform vec4 selectColor;
uniform float mixColor;
uniform float mixAlpha;
uniform float mixSelection;

uniform float fallofPower = 2.0;

uniform sampler2D tf;

void main() {
    vec4 res = texture(tf, vec2(lScalarMeta, 0.5f));
    
    if (subtractiveBelnding) {
        res.rgb = 1 - res.rgb;
    }
    
    res = mix(res, color, vec4(vec3(mixColor), mixAlpha));
    res = mix(res, selectColor, vec4(mixSelection));

    if (additiveBlend) {
        res.a *= pow(lFalloffAlpha, fallofPower);
    }

    PickingData = lPickColor;
    FragData0 = res;
}
