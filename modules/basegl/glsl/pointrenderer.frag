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

#define PI 3.1415926535897932384626433832795

uniform CameraParameters camera;

// holds viewport offset x, offset y, 2 / viewport width, 2 / viewport height
uniform vec4 viewport;

uniform float borderWidth;        // [pixel]
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

vec4 blendBackToFront(vec4 srcColor, vec4 dstColor) {
    return srcColor + dstColor * (1.0 - srcColor.a);
}

in PointGeom {
    vec4 center;
    vec4 color;
    flat vec4 pickColor;
    vec3 camPos;
    float radius;
    flat uint index;
}
point;

float pxToView(float px) {
    vec4 tmp = camera.clipToView * vec4(px * viewport.z, 0, gl_FragCoord.z, 1);
    tmp /= tmp.w;
    return tmp.x;
}

void main() {
    vec4 pixelPos = gl_FragCoord;
    pixelPos.xy -= viewport.xy;
    // transform fragment coordinates from window coordinates to view coordinates.
    vec4 coord = pixelPos * vec4(viewport.z, viewport.w, 2.0, 0.0) + vec4(vec3(-1.0), 1.0);
    // transform fragment coord into object space
    coord = camera.clipToWorld * coord;
    coord /= coord.w;
    coord -= point.center;

    float va = pxToView(antialising);

    float r = length(coord.xy);
    if (r > point.radius + va) discard;

    // shading
    vec4 glyphColor = point.color;

#if defined(ENABLE_TEXTURING)
    {
        vec2 uv = (coord.xy + vec2(point.radius)) / vec2(2.0 * point.radius);
        vec4 texCol = texture(pointTexture, uv);
        glyphColor = mix(glyphColor, texCol, min(texCol.a, textureMixing));
    }
#endif

#if defined(ENABLE_LABELS)
    const vec2 labelSpherePos = vec2(0.0, 0.0);
    vec2 labelSphereSize = 2.0 * point.radius * vec2(label.size, label.size * label.aspect);
    const ivec2 atlasDims = ivec2(30, 30);

    vec2 labelSphereStart = labelSpherePos - 0.5 * labelSphereSize;
    vec2 labelSphereEnd = labelSpherePos + 0.5 * labelSphereSize;
    vec2 sphereToTexture = 1.0 / (labelSphereSize * atlasDims);

    {
        vec2 uv = coord.xy;

        if (all(lessThan(labelSphereStart, uv)) && all(lessThan(uv, labelSphereEnd))) {
            vec2 altasUv = (uv - labelSphereStart) * sphereToTexture;
            altasUv += vec2(int(point.index / atlasDims.x), point.index % atlasDims.y) / atlasDims;
            vec4 labelColor = label.color * texture(label.tex, altasUv);
            labelColor.a *= point.color.a;
            glyphColor = blendBackToFront(labelColor, glyphColor);
        }
    }
#endif

    float vb = pxToView(borderWidth);

    float mixing = clamp(mix(0.0, 1.0, (r - (point.radius - vb - va)) / va), 0.0, 1.0);
    glyphColor = mix(glyphColor, borderColor, mixing);
    glyphColor.a = clamp(mix(glyphColor.a, 0.0, (r - point.radius) / va), 0.0, 1.0);

    FragData0 = glyphColor;
    gl_FragDepth = glyphDepth(point.center.xyz, camera.worldToClip);
    PickingData = point.pickColor;
}
