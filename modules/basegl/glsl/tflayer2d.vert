/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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
uniform CameraParameters camera;

uniform mat4 trafo = mat4(1);

out TFVertex {
    smooth vec2 center;
    smooth vec4 color;
    flat float radius;
    flat int primitive;
    flat uint pickID;
} out_vert;

void main() {
    // vec4 pos = mat4(2.0, 0.0, 0.0, 0.0,
    //                 0.0, 2.0, 0.0, 0.0,
    //                 0.0, 0.0, 1.0, 0.0,
    //                 -1.0, -1.0, 0.0, 1.0) * vec4(in_Position, 1.0);
    vec4 pos = geometry.dataToWorld * vec4(in_Position, 1.0);
    out_vert.center = pos.xy;
    out_vert.radius = in_Radii;
#if defined(HAS_SCALARMETA)
    out_vert.color = in_ScalarMeta;
#else
    out_vert.color = in_Color;    
#endif // HAS_SCALARMETA

#if defined(HAS_INDEX)
    out_vert.primitive = in_Index;
#else
    out_vert.primitive = 1;
#endif // HAS_INDEX

#if defined(HAS_PICKING)
    out_vert.pickID = in_Picking;
#else
    out_vert.pickID = 0;
#endif

    gl_Position = pos;
}
