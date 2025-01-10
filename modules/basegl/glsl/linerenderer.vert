/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

// initialize camera matrices with the identity matrix to enable default rendering
// without any transformation, i.e. all lines in clip space
uniform CameraParameters camera = CameraParameters( mat4(1), mat4(1), mat4(1), mat4(1),
                                    mat4(1), mat4(1), vec3(0), 0, 1);

uniform bool pickingEnabled = false;
uniform vec4 defaultColor = vec4(1, 1, 1, 1);

struct Config {
    vec3 color;
    float alpha;    
};
uniform Config config = Config(vec3(0.7), 1.0);

uniform sampler2D metaColor;

out LineVert {
    vec4 worldPosition;
    vec4 color;
    flat uint pickID;
    flat uint index;
} vertex;
 
void main() {
#if defined(HAS_SCALARMETA) && defined(USE_SCALARMETACOLOR)
    vertex.color = texture(metaColor, vec2(in_ScalarMeta, 0.5));
#elif defined(HAS_COLOR)
    vertex.color = in_Color;
#else
    vertex.color = defaultColor;
#endif

#if defined(OVERRIDE_COLOR)
    vertex.color.rgb = config.color;
#endif

#if defined(OVERRIDE_ALPHA)
    vertex.color.a = config.alpha;
#endif

#if defined(HAS_INDEX)
    vertex.index = in_Index;
#else
    vertex.index = gl_VertexID;
#endif

    vertex.worldPosition = geometry.dataToWorld * vec4(in_Position.xyz, 1.0);
    gl_Position = camera.worldToClip * vertex.worldPosition;
#if defined(HAS_PICKING)
    vertex.pickID = in_Picking;
#else 
    vertex.pickID = 0;
#endif
}
