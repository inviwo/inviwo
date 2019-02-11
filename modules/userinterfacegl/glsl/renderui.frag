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

uniform sampler2DArray arrayTexSampler;
// texture indices for normal, pressed, checked, 
// halo normal, halo pressed, halo checked
// border normal, border pressed, border checked
uniform int arrayTexMap[9] = int[]( 0, 0, 0, 0, 0, 0, 0, 0, 0 );

uniform vec4 uiColor = vec4(0.0, 0.0, 0.0, 1.0);
uniform vec4 uiBorderColor = vec4(0.0, 0.0, 0.0, 1.0);
uniform vec4 haloColor = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec3 pickingColor = vec3(0.0);

uniform ivec2 uiState = ivec2(0, -1); // active / hovered

in vec3 texCoord;

void main() {
    vec4 dstColor = vec4(0.0);

    // draw halo only if hovered
    if (uiState.y > 0) {
        vec4 halo = texture(arrayTexSampler, vec3(texCoord.xy, arrayTexMap[uiState.x + 3]));
        // front-to-back blending
        dstColor = vec4(haloColor.rgb, haloColor.a * halo.a);
    }

    // sample UI color texture, first channel contains border, second channel filled areas, fourth channel alpha
    // border dominates if non-zero

    // border color
    vec4 fill = uiColor * texture(arrayTexSampler, vec3(texCoord.xy, arrayTexMap[uiState.x]));
    vec4 border = uiBorderColor * texture(arrayTexSampler, vec3(texCoord.xy, arrayTexMap[uiState.x + 6]));

    // blend border color with fill color
    vec4 color = mix(fill, border, border.a);
    color = fill * (1.0 - border.a) + border * border.a;

    // mix color with optional halo
    dstColor = mix(dstColor, color, color.a);

    FragData0 = dstColor;

//FragData0 = vec4(texCoord, 1.0);

    // prevent picking for transparent regions and if picking color alpha is negative
    // by setting the alpha to 0.
    PickingData = vec4(pickingColor.rgb, step(0.0, (color.a - 0.001)));
}
