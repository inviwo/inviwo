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
#include "utils/shading.glsl"
#include "utils/glyphs.glsl"
#include "utils/blend.glsl"

uniform CameraParameters camera;

// holds viewport offset x, offset y, 2 / viewport width, 2 / viewport height
uniform vec4 viewport;

uniform float borderWidth = 0.0;  // [pixel]
uniform float antialising = 1.5;  // [pixel]
uniform vec4 borderColor = vec4(1.0, 0.0, 0.0, 1.0);

#if defined(ENABLE_LABELS)
struct Label {
    sampler2D tex;
    float aspect;
    vec4 color;
    float size;
};
uniform Label label;
#endif

#if defined(ENABLE_TEXTURING)
uniform sampler2D pointTexture;
uniform float textureMixing;
#endif

in PointGeom {
    vec4 center;
    vec4 color;
    flat vec4 pickColor;
    vec3 camPos;
    float radius;
    flat uint index;
}
point;

void main() {
    vec4 pixelPos = gl_FragCoord;
    pixelPos.xy -= viewport.xy;

    // gl_PointCoord [0,1] within the point
    vec2 coord = gl_PointCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    float r = length(coord);
    if (r > 1.0) discard;

    r *= point.radius;

    // shading
    vec4 glyphColor = point.color;

#if defined(ENABLE_TEXTURING)
    {
        vec2 uv = vec2(gl_PointCoord.x, 1.0 - gl_PointCoord.y);
        vec4 tex = texture(pointTexture, uv);
        glyphColor = mix(glyphColor, TEXTURING_BLEND_FUNC(tex, glyphColor), textureMixing);
    }
#endif

#if defined(ENABLE_LABELS)
    {
        const ivec2 atlasDims = ivec2(30, 30);
        vec2 labelSphereSize = 2.0 * vec2(label.size, label.size * label.aspect);
        vec2 labelSphereStart = -0.5 * labelSphereSize;
        vec2 labelSphereEnd = +0.5 * labelSphereSize;
        vec2 sphereToTexture = 1.0 / (labelSphereSize * atlasDims);
        vec2 uv = gl_PointCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0);

        if (all(lessThan(labelSphereStart, uv)) && all(lessThan(uv, labelSphereEnd))) {
            vec2 altasUv = (uv - labelSphereStart) * sphereToTexture;
            altasUv += vec2(int(point.index / atlasDims.x), point.index % atlasDims.y) / atlasDims;
            vec4 labelColor = label.color * texture(label.tex, altasUv);
            labelColor.a *= point.color.a;
            glyphColor = premultipliedAlphaBlend(labelColor, glyphColor);
        }
    }
#endif

    if (antialising > 0.0 && borderWidth > 0.0) {
        if (r > point.radius - antialising) {
            glyphColor = borderColor;
            glyphColor.a = mix(glyphColor.a, 0.0, (r - point.radius + antialising) / antialising);
        } else if (r > point.radius - borderWidth - antialising) {
            glyphColor = borderColor;
        } else if (r > point.radius - borderWidth - 2.0 * antialising) {
            glyphColor = mix(glyphColor, borderColor,
                             (r - (point.radius - borderWidth - 2.0 * antialising)) / antialising);
        }
    } else if (antialising > 0.0) {
        if (r > point.radius - antialising) {
            glyphColor.a = mix(glyphColor.a, 0.0, (r - point.radius + antialising) / antialising);
        }
    } else if (borderWidth > 0.0) {
        if (r > point.radius - borderWidth) {
            glyphColor = borderColor;
        }
    }

    FragData0 = glyphColor;
    gl_FragDepth = glyphDepth(point.center.xyz, camera.worldToClip);
    PickingData = point.pickColor;
}
