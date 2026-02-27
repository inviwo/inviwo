/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

uniform GeometryParameters geometry;

uniform float widthScaling = 1.0;
uniform vec4 defaultColor = vec4(0.7, 0.7, 0.7, 1);
uniform float defaultWidth = 0.1;
uniform sampler2D metaColor;

out LineVert {
    vec4 color;
    vec3 binormal;
    float width;
    flat uint pickID;
} line;

 
void main() {
#if defined(HAS_SCALARMETA) && defined(USE_SCALARMETACOLOR) && !defined(FORCE_COLOR)
    line.color = texture(metaColor, vec2(in_ScalarMeta, 0.5));
#elif defined(HAS_COLOR) && !defined(FORCE_COLOR)
    line.color = in_Color;
#else
    line.color = defaultColor;
#endif

    vec3 binormal = vec3(1, 0, 0);
#if defined(HAS_NORMAL)
    binormal = in_Normal;
#endif

#if defined(FORCE_WIDTH)
    line.width = defaultWidth * widthScaling;
#else
    line.width = length(binormal) * widthScaling;
#endif

    line.binormal = geometry.dataToWorldNormalMatrix * normalize(binormal);

#if defined(HAS_PICKING)
    line.pickID = in_Picking;
#else 
    line.pickID = 0;
#endif

    gl_Position = geometry.dataToWorld * vec4(in_Position, 1.0);
}  
