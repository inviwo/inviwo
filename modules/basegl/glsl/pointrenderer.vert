/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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
#include "utils/selectioncolor.glsl"

struct Config {
    vec3 color;
    float alpha;
    float radius;
};
uniform Config config = Config(vec3(1, 0, 0), 1.0, 0.1);

uniform sampler2D metaColor;

uniform uint marker = 0;

#if defined(ENABLE_BNL)
uniform usamplerBuffer bnl;
uniform SelectionColor bnlFilter;
uniform SelectionColor bnlSelect;
uniform SelectionColor bnlHighlight;
#endif

out PointVert {
    vec4 color;
    flat float radius;
    flat uint pickID;
    flat uint index;
    flat uint marker;
} point;

#if defined(ENABLE_PERIODICITY)
flat out uint instance;
#endif

void main(void) {
#if defined(HAS_SCALARMETA) && defined(USE_SCALARMETACOLOR)
    point.color = texture(metaColor, vec2(in_ScalarMeta, 0.5));
#elif defined(HAS_COLOR)
    point.color = in_Color;
#else
    point.color = vec4(config.color, config.alpha);
#endif

#if defined(OVERRIDE_COLOR)
    point.color.rgb = config.color;
#endif

#if defined(OVERRIDE_ALPHA)
    point.color.a = config.alpha;
#endif

#if defined(HAS_RADII) && !defined(OVERRIDE_RADIUS)
    point.radius = in_Radii;
#else
    point.radius = config.radius;
#endif

#if defined(HAS_INDEX)
    point.index = in_Index;
#else
    point.index = gl_VertexID;
#endif

#if defined(HAS_INTMETA) && !defined(OVERRIDE_MARKER)
    point.marker = in_IntMeta;
#else
    point.marker = marker;
#endif


#if defined(ENABLE_BNL)
    int bnlSize = textureSize(bnl);
    uint flags = point.index < bnlSize ? texelFetch(bnl, int(point.index)).x : uint(0);

    if (flags == 3) {
        point.color = applySelectionColor(point.color, bnlFilter);
    } else if (flags == 2) {
        point.color = applySelectionColor(point.color, bnlHighlight);
    } else if (flags == 1) {
        point.color = applySelectionColor(point.color, bnlSelect);
    }
#endif

#if defined(HAS_PICKING)
    point.pickID = in_Picking;
#else
    point.pickID = 0;
#endif
    
    gl_Position = vec4(in_Position.xyz, 1.0);
    
#if defined(ENABLE_PERIODICITY)
    instance = gl_InstanceID;
#endif
}
