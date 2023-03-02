/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

// Owned by the SphereRenderer Processor

#include "utils/structs.glsl"
#include "utils/selectioncolor.glsl"


struct Config {
    vec3 color;
    float alpha;
    float radius;
};
uniform Config config = Config(vec3(1, 0, 0), 1.0, 0.1);

uniform sampler2D metaColor;

#if defined(ENABLE_BNL)
uniform usamplerBuffer bnl;
uniform SelectionColor bnlFilter;
uniform SelectionColor bnlSelect;
uniform SelectionColor bnlHighlight;
#endif

out SphereVert {
    vec4 color;
    flat float radius;
    flat uint pickID;
    flat uint index;
}
sphere;

#if defined(ENABLE_PERIODICITY)
flat out uint instance;
#endif

void main(void) {
#if defined(HAS_SCALARMETA) && defined(USE_SCALARMETACOLOR)
    sphere.color = texture(metaColor, vec2(in_ScalarMeta, 0.5));
#elif defined(HAS_COLOR)
    sphere.color = in_Color;
#else
    sphere.color = vec4(config.color, config.alpha);
#endif

#if defined(OVERRIDE_COLOR)
    sphere.color.rbg = config.color;
#endif

#if defined(OVERRIDE_ALPHA)
    sphere.color.a = config.alpha;
#endif

#if defined(HAS_RADII) && !defined(OVERRIDE_RADIUS)
    sphere.radius = in_Radii;
#else
    sphere.radius = config.radius;
#endif

#if defined(HAS_INDEX)
    sphere.index = in_Index;
#else
    sphere.index = gl_VertexID;
#endif

#if defined(ENABLE_BNL)
    int bnlSize = textureSize(bnl);
    uint flags = sphere.index < bnlSize ? texelFetch(bnl, int(sphere.index)).x : uint(0);

    if (flags == 3) {
        sphere.color = applySelectionColor(sphere.color, bnlFilter);
    } else if (flags == 2) {
        sphere.color = applySelectionColor(sphere.color, bnlHighlight);
    } else if (flags == 1) {
        sphere.color = applySelectionColor(sphere.color, bnlSelect);
    }
#endif

#if defined(HAS_PICKING)
    sphere.pickID = in_Picking;
#else
    sphere.pickID = 0;
#endif
    
    gl_Position = vec4(in_Position.xyz, 1.0);
    
#if defined(ENABLE_PERIODICITY)
    instance = gl_InstanceID;
#endif
}
