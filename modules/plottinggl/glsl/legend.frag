/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

uniform sampler2D transferFunction;

// variable legendRotation =
// 0 -> 0 degree rotation ccw
// 1 -> 90 degree rotation ccw
// 2 -> 180 degree rotation ccw
// 3 -> 270 degree rotation ccw
uniform int legendRotation;

uniform int backgroundStyle;
uniform float checkerBoardSize;
uniform vec4 checkerColor1 = vec4(0.5, 0.5, 0.5, 1);
uniform vec4 checkerColor2 = vec4(1, 1, 1, 1);

uniform vec4 color = vec4(0, 0, 0, 1);
uniform int borderWidth = 1;
uniform ivec4 viewport = ivec4(0, 128, 128, 128);


// modified checkerboard for this specific shader
vec4 checkerBoard(vec2 pos) {
    vec2 t = floor(pos / vec2(checkerBoardSize));
    return mix(checkerColor2, checkerColor1, mod(t.x + t.y, 2.0) < 1.0 ? 1.0 : 0.0);
}

vec4 over(vec4 colorB, vec4 colorA) {
    // f(a,b) = b, b over a, regular front-to-back blending
    vec3 col = mix(colorB.rgb * colorB.a, colorA.rgb, colorA.a);
    float alpha = colorA.a + (1.0 - colorA.a) * colorB.a;
    return vec4(col, alpha);
}

void main() {
    vec2 texCoord = gl_FragCoord.xy - viewport.xy;
    vec2 outputDim = viewport.zw - vec2(2 * borderWidth);
    vec2 centeredPos = (texCoord - viewport.zw * 0.5);
    vec2 normPos = texCoord / outputDim;

    float tfSamplePos = mix(normPos.x, normPos.y, mod(legendRotation, 2));
    vec4 colorTF = texture(transferFunction, vec2(tfSamplePos, 0.0));

    // increase alpha to allow better visibility by 1 - (1 - a)^4 and then add "backgroundAlpha" to
    // set alpha to 1 if no background is wanted
    colorTF.a = mix(1.0 - pow(1.0 - colorTF.a, 4.0), 1.0, float(backgroundStyle));

    // blend in the checkerboard as background to the TF depending on its opacity
    vec4 finalColor = over(checkerBoard(centeredPos), colorTF);

    // set border flag if the fragment coord is within the border
    bool border = borderWidth > 0 && any(greaterThan(abs(centeredPos), outputDim * 0.5));
    FragData0 =  mix(finalColor, color, bvec4(border));

    // no depth input, reset depth to largest value
    gl_FragDepth = 1.0;
}
