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

#include "utils/structs.glsl"

uniform ImageParameters outportParameters;

uniform vec2 origin = vec2(0.0, 0.0);   // screen coords (pixel)
uniform vec2 extent = vec2(32.0, 32.0); // width and height in screen coords (pixel)

uniform vec2 marginScale = vec2(1.0);

uniform mat3 uiTextureMatrix = mat3(1);

out vec4 color;
out vec3 texCoord;

void main() {
    color = in_Color;

    // transform texture coordinates to account for UI widget orientation
    // invert y coord to flip input images
    vec3 t = uiTextureMatrix * vec3(in_TexCoord.x, 1.0 - in_TexCoord.y, 1.0);
    texCoord = vec3(t.xy, 0.0);

    vec2 vertexPos = in_Vertex.xy;

    // apply border image calculations, i.e. the corners have a fixed width determined by marginScale
    // The idea is to move the inner vertex coords at 0.25 and 0.75 to their respective positions
    // to maintain constant corner sizes.
    //
    //    if (vertexPos.x < 0.5) {
    //        // adjust left margin without affecting left edge
    //        vertexPos.x *= marginScale.x;
    //    }
    //    else {
    //        // adjust right margin without affecting right edge
    //        vertexPos.x = 1.0 - (1.0 - vertexPos.x) * marginScale.x;
    //    }
    // compact calculations identical to the code in the above comment
    vec2 leftAdjustment = vertexPos * marginScale;
    vec2 rightAdjustment = 1.0 - (1.0 - vertexPos) * marginScale;
    vertexPos = mix(leftAdjustment, rightAdjustment, step(0.5, vertexPos));

    // determine vertex position
    vec2 pos = vertexPos * extent + origin;

    // transform incoming vertex coords from screen coords to normalized dev coords
    pos = pos * outportParameters.reciprocalDimensions * 2.0 - 1.0;
    gl_Position = vec4(pos, -1.0, 1.0);
}
