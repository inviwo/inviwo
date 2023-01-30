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

uniform GeometryParameters geometry;

uniform vec4 defaultColor = vec4(1, 0, 0, 1);
uniform float defaultRadius = 0.1f;

uniform sampler2D metaColor;

#if defined(ENABLE_BNL)
uniform usamplerBuffer bnl;
uniform SelectionColor showFiltered;
uniform SelectionColor showSelected;
uniform SelectionColor showHighlighted;
uniform float selectionScaleFactor = 1.0;
#endif

out vec4 worldPosition;
out vec4 sphereColor;
flat out float sphereRadius;
flat out uint pickID;

void main(void) {
#if defined(HAS_SCALARMETA) && defined(USE_SCALARMETACOLOR) && !defined(FORCE_COLOR)
    sphereColor = texture(metaColor, vec2(in_ScalarMeta, 0.5));
#elif defined(HAS_COLOR) && !defined(FORCE_COLOR)
    sphereColor = in_Color;
#else
    sphereColor = defaultColor;
#endif

#if defined(HAS_RADII) && !defined(FORCE_RADIUS)
    sphereRadius = in_Radii;
#else
    sphereRadius = defaultRadius;
#endif

#if defined(ENABLE_BNL)
    int bnlSize = textureSize(bnl);
#if defined(HAS_INDEX)
    uint flags = in_Index < bnlSize ? texelFetch(bnl, int(in_Index)).x : uint(0);
#else
    uint flags = gl_VertexID < bnlSize ? texelFetch(bnl, gl_VertexID).x : uint(0);
#endif

    if (flags == 3) {
        sphereColor = applySelectionColor(sphereColor, showFiltered);
    } else if (flags == 2) {
        sphereColor = applySelectionColor(sphereColor, showHighlighted);
    } else if (flags == 1) {
        sphereColor = applySelectionColor(sphereColor, showSelected);
        sphereRadius *= selectionScaleFactor;
    }
#endif

#if defined(HAS_PICKING)
    pickID = in_Picking;
#else
    pickID = 0;
#endif

    worldPosition = geometry.dataToWorld * vec4(in_Position.xyz, 1.0);
    gl_Position = worldPosition;
}
