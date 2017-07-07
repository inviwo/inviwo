/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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

uniform sampler2D inport_;
uniform ImageParameters outportParameters_;

uniform bool sharpen;
uniform mat3 kernel;

void main() {
    if(sharpen) {
        float x = outportParameters_.reciprocalDimensions.x;
        float y = outportParameters_.reciprocalDimensions.y;
        
        vec2 texCoords11 = (gl_FragCoord.xy - vec2( 0, 0)) * outportParameters_.reciprocalDimensions;
        vec2 texCoords01 = texCoords11 - vec2(-x, 0.0);
        vec2 texCoords21 = texCoords11 - vec2(x, 0.0);

        vec2 texCoords00 = texCoords11 - vec2(x, y);
        vec2 texCoords10 = texCoords11 - vec2(0.0, y);            
        vec2 texCoords20 = texCoords11 - vec2(-x, y);
        
        vec2 texCoords02 = texCoords11 - vec2(-x, y);
        vec2 texCoords12 = texCoords11 - vec2(0.0, y);
        vec2 texCoords22 = texCoords11 - vec2(x, y);

        vec4 samples[9];
        const float weights[9] = float[9](
                                        kernel[0][0], 
                                        kernel[1][0],
                                        kernel[2][0],
                                        kernel[0][1],
                                        kernel[1][1],
                                        kernel[2][1],
                                        kernel[0][2],
                                        kernel[1][2],
                                        kernel[2][2]);

        samples[0] = texture(inport_, texCoords00);
        samples[1] = texture(inport_, texCoords10);
        samples[2] = texture(inport_, texCoords20);
        samples[3] = texture(inport_, texCoords01);
        samples[4] = texture(inport_, texCoords11);
        samples[5] = texture(inport_, texCoords21);
        samples[6] = texture(inport_, texCoords02);
        samples[7] = texture(inport_, texCoords12);
        samples[8] = texture(inport_, texCoords22);

        vec4 finalVal = weights[4] * samples[4];

        for(int i = 0; i < 9; i++) {
            if(i != 4) {
                finalVal += samples[i] * weights[i];
            } 
        }

        finalVal.a = 1.0;

        FragData0 = clamp(finalVal,0,1);
    }

    else {
        FragData0 = texture(inport_, gl_FragCoord.xy * outportParameters_.reciprocalDimensions);
    }
}