/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

uniform sampler2D color_;

uniform vec2 position_;
uniform ivec2 dimensions_;
uniform ivec2 legendSize_;

// variable rotationTF_ =
// 0 -> 0 degree rotation ccw
// 1 -> 90 degree rotation ccw
// 2 -> 180 degree rotation ccw
// 3 -> 270 degree rotation ccw
uniform int rotationTF_;

uniform int backgroundAlpha_;
uniform int checkerBoardSize_;
uniform vec4 checkerColor1 = vec4(0.5, 0.5, 0.5, 1);
uniform vec4 checkerColor2 = vec4(1, 1, 1, 1);

uniform vec4 borderColor_;

#define M_PI 3.1415926535897932384626433832795

// modified checkerboard for this specific shader
vec4 checkerBoard() {
    vec2 t = floor(ivec2(gl_FragCoord.x - position_.x * dimensions_.x,
                         dimensions_.y - (gl_FragCoord.y - position_.y * dimensions_.y)) /
                   vec2(checkerBoardSize_));
    return mix(checkerColor2, checkerColor1, mod(t.x + t.y, 2.0) < 1.0 ? 1.0 : 0.0);
}

vec2 mirrorVertically(vec2 v) {
    float mirrorVertical = mod(rotationTF_, 2);
    return v.xy * (1 - mirrorVertical) + v.yx * mirrorVertical;
}

// map the gl_FragCoord to the transfer function coordinates
vec2 calcSamplePos() {
    // mirror variables vertically
    vec2 alteredDimensions = mirrorVertically(dimensions_);
    vec2 alteredFragCoords = mirrorVertically(gl_FragCoord.xy);
    vec2 alteredLegendSize = mirrorVertically(legendSize_);
    vec2 alteredPosition = mirrorVertically(position_);

    vec2 pixelPosA = alteredPosition * alteredDimensions - (alteredLegendSize / 2.0);
    vec2 screenPosA = pixelPosA / alteredDimensions;

    vec2 pixelPosB = alteredPosition * alteredDimensions + (alteredLegendSize / 2.0);
    vec2 screenPosB = pixelPosB / alteredDimensions;

    // map the fragment coordinates to the legends local coordinates
    return ((alteredFragCoords / alteredDimensions) - screenPosA) / (screenPosB - screenPosA);
}

vec4 over(vec4 colorB, vec4 colorA) {
    // f(a,b) = b, b over a, regular front-to-back blending
    vec3 col = mix(colorB.rgb * colorB.a, colorA.rgb, colorA.a);
    float alpha = colorA.a + (1.0 - colorA.a) * colorB.a;
    return vec4(col, alpha);
}

void main() {

    vec2 samplePos = calcSamplePos();

    vec4 colorTF = texture(color_, samplePos);

    // if the sampled position is outside the original square, it is part of the border
    // NOTE: This method is a little bit unprecise, and the border width can differ with +-1 pixel
    // or so
    if (calcSamplePos().x < 0 || calcSamplePos().x > 1 || calcSamplePos().y < 0 ||
        calcSamplePos().y > 1)
        colorTF = borderColor_;

    // increase alpha to allow better visibility by 1 - (1 - a)^4 and then add "backgroundAlpha_" to
    // set alpha to 1 if no background is wanted
    colorTF.a = min(1.0f - pow(1.0f - colorTF.a, 4.0f) + backgroundAlpha_, 1.0f);

    // blend in the checkerboard as background to the TF depending on its opacity
    vec4 finalColor = over(checkerBoard(), colorTF);

    FragData0 = finalColor;

    // no depth input, reset depth to largest value
    gl_FragDepth = 1.0;
}
