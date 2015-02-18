/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

uniform sampler2D inport_;
uniform float alpha_;
uniform vec2 dimensions_;

void main() {
    vec2 texCoordsM = vec2(gl_FragCoord.x, gl_FragCoord.y) * dimensions_;
    vec3 colorM     = texture(inport_, texCoordsM).rgb;
    vec2 texCoordsR     = vec2(gl_FragCoord.x + 1.0 , gl_FragCoord.y)       * dimensions_;
    vec2 texCoordsL     = vec2(gl_FragCoord.x - 1.0 , gl_FragCoord.y)       * dimensions_;
    vec2 texCoordsU     = vec2(gl_FragCoord.x       , gl_FragCoord.y + 1.0) * dimensions_;
    vec2 texCoordsD     = vec2(gl_FragCoord.x       , gl_FragCoord.y - 1.0) * dimensions_;
    vec2 texCoordsUL    = vec2(gl_FragCoord.x + 1.0, gl_FragCoord.y - 1.0)  * dimensions_;
    vec2 texCoordsUR    = vec2(gl_FragCoord.x + 1.0, gl_FragCoord.y + 1.0)  * dimensions_;
    vec2 texCoordsDL    = vec2(gl_FragCoord.x - 1.0, gl_FragCoord.y - 1.0)  * dimensions_;
    vec2 texCoordsDR    = vec2(gl_FragCoord.x - 1.0, gl_FragCoord.y + 1.0)  * dimensions_;
    vec3 colorR  = texture(inport_, texCoordsR).rgb;
    vec3 colorL  = texture(inport_, texCoordsL).rgb;
    vec3 colorU  = texture(inport_, texCoordsU).rgb;
    vec3 colorD  = texture(inport_, texCoordsD).rgb;
    vec3 colorUL = texture(inport_, texCoordsUL).rgb;
    vec3 colorUR = texture(inport_, texCoordsUR).rgb;
    vec3 colorDL = texture(inport_, texCoordsDL).rgb;
    vec3 colorDR = texture(inport_, texCoordsDR).rgb;
    float valR   = (colorR.r + colorR.g + colorR.b)/3.0;
    float valL   = (colorL.r + colorL.g + colorL.b)/3.0;
    float valU   = (colorU.r + colorU.g + colorU.b)/3.0;
    float valD   = (colorD.r + colorD.g + colorD.b)/3.0;
    float valUL  = (colorUL.r + colorUL.g + colorUL.b)/3.0;
    float valUR  = (colorUR.r + colorUR.g + colorUR.b)/3.0;
    float valDL  = (colorDL.r + colorDL.g + colorDL.b)/3.0;
    float valDR  = (colorDR.r + colorDR.g + colorDR.b)/3.0;
    float valX  = valUR + 2.0*valR + valDR - valUL - 2.0*valL - valUR;
    float valY  = valUL + 2.0*valU + valUR - valDL - 2.0*valD - valDR;
    float val   = length(vec2(valX, valY));
    vec4 color = vec4(vec3(val) * alpha_ + colorM * (1 - alpha_), 1.0);
    FragData0 = color;
}