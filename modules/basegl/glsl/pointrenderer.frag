/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

uniform float pointSize; // [pixel]
uniform float borderWidth; // [pixel]
uniform float antialising = 1.5; // [pixel]

uniform vec4 borderColor = vec4(1.0, 0.0, 0.0, 1.0);

in vec4 worldPosition_;
in vec3 normal_;
in vec4 color_;

void main() {
    // calculate normal from texture coordinates
    vec3 normal;
    normal.xy = gl_PointCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    float r = sqrt(dot(normal.xy, normal.xy));
    if (r > 1.0) {
       discard;   // kill pixels outside circle
    }
    normal.z = sqrt(1.0 - r);

    float glyphRadius = pointSize * 0.5;
    
    r *= pointSize * 0.5 + borderWidth;

    // pseudo antialiasing with the help of the alpha channel
    // i.e. smooth transition between center and border, and smooth alpha fall-off at the outer rim
    float outerglyphRadius = glyphRadius + borderWidth - antialising; // used for adjusting the alpha value of the outer rim

    float borderValue = clamp(mix(0.0, 1.0, (r - glyphRadius + 1) / 2), 0.0, 1.0);
    float borderAlpha = clamp(mix(1.0, 0.0, (r - outerglyphRadius) / (glyphRadius + borderWidth - outerglyphRadius)), 0.0, 1.0);

    vec4 color = mix(color_, borderColor, borderValue);
    FragData0 = vec4(color.rgb, color.a * borderAlpha);
}
