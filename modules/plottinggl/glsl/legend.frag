/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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

// orientation of the legend
// 0 -> horizontal legend
// 1 -> vertical legend
uniform int legendOrientation = 0;

// background style
//  0 no background
//  1 solid color
//  2 checkerboard
//  3 checkerboard and opaque TF
//  4 opaque TF
uniform int backgroundStyle;
uniform float checkerBoardSize;
uniform vec4 checkerColor1 = vec4(0.5, 0.5, 0.5, 1.0);
uniform vec4 checkerColor2 = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec4 backgroundColor = vec4(0.0, 0.0, 0.0, 1.0);

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

vec4 solidBackground(vec4 src) {
    // pre-multiply source color and background color
    src.rgb *= src.a;
    vec4 bgColor = backgroundColor;
    bgColor.rgb *= bgColor.a;

    return src + (1.0 - src.a) * bgColor;
}

vec4 checkerboardBackground(vec4 src, vec2 pos) {
    return over(checkerBoard(pos), src);
}

vec4 noBackground(vec4 src) {
    // use regular OpenGL alpha blending
    src.rgb *= src.a;
    return src;    
}

void main() {
    vec2 texCoord = gl_FragCoord.xy - viewport.xy;
    vec2 outputDim = viewport.zw - vec2(2 * borderWidth);
    vec2 centeredPos = (texCoord - viewport.zw * 0.5);
    vec2 normPos = (texCoord - vec2(borderWidth)) / outputDim;

    float tfSamplePos = mix(normPos.x, normPos.y, legendOrientation);
    vec4 colorTF = texture(transferFunction, vec2(tfSamplePos, 0.0));

    // increase alpha for better visibility by 1 - (1 - a)^4
    colorTF.a = 1.0 - pow(1.0 - colorTF.a, 4.0);

    vec4 finalColor;
    if (backgroundStyle == 1) {
        finalColor = solidBackground(colorTF);
    } else if (backgroundStyle == 2) {
        finalColor = checkerboardBackground(colorTF, centeredPos);
    } else if (backgroundStyle == 3) {
        float verticalPos = mix(1.0 - normPos.y, normPos.x, mod(legendRotation, 2));
        if (verticalPos > 0.5) {
            finalColor = checkerboardBackground(colorTF, centeredPos);
        } else {
            finalColor = vec4(colorTF.rgb, 1.0);
        }
    } else if (backgroundStyle == 4) {
        finalColor = vec4(colorTF.rgb, 1.0);
    } else {
        finalColor = noBackground(colorTF);
    }

    // set border flag if the fragment coord is within the border
    bool border = borderWidth > 0 && any(greaterThan(abs(centeredPos), outputDim * 0.5));
    FragData0 =  mix(finalColor, color, bvec4(border));
}
