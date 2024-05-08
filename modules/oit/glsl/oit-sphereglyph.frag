/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

// Owned by the SphereRasterizer Processor
// Should be very similar to sphereglyph.frag

#include "utils/structs.glsl" //! #include "../../opengl/glsl/utils/structs.glsl"
#include "utils/shading.glsl" //! #include "../../opengl/glsl/utils/shading.glsl"
#include "utils/glyphs.glsl"  //! #include "../../opengl/glsl/utils/glyphs.glsl"

#define PI 3.1415926535897932384626433832795

#ifdef USE_FRAGMENT_LIST
#include "oit/abufferlinkedlist.glsl"

// this is important for the occlusion query
layout(early_fragment_tests) in;

layout(pixel_center_integer) in vec4 gl_FragCoord;
#endif

uniform CameraParameters camera;
uniform LightParameters lighting;
uniform float overrideAlpha = 1.0;

// holds viewport offset x, offset y, 2 / viewport width, 2 / viewport height
uniform vec4 viewport;
uniform float clipShadingFactor = 0.9;

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
uniform sampler2D sphereTexture;
#endif

vec4 blendBackToFront(vec4 srcColor, vec4 dstColor) {
    return srcColor + dstColor * (1.0 - srcColor.a);
}

const mat3 rotations[] = mat3[](mat3(1, 0, 0, 0, 1, 0, 0, 0, 1), mat3(0, 0, 1, 0, 1, 0, -1, 0, 0),
                                mat3(-1, 0, 0, 0, 1, 0, 0, 0, -1), mat3(0, 0, -1, 0, 1, 0, 1, 0, 0),
                                mat3(0, -1, 0, 1, 0, 0, 0, 0, 1), mat3(0, 1, 0, -1, 0, 0, 0, 0, 1));

in SphereGeom {
    vec4 center;
    vec4 color;
    flat vec4 pickColor;
    vec3 camPos;
    float radius;
    flat uint index;
}
sphere;

void main() {
    vec4 pixelPos = gl_FragCoord;
    pixelPos.xy -= viewport.xy;
    // transform fragment coordinates from window coordinates to view coordinates.
    vec4 coord = pixelPos * vec4(viewport.z, viewport.w, 2.0, 0.0) + vec4(vec3(-1.0), 1.0);

    // transform fragment coord into object space
    // coord = gl_ModelViewProjectionMatrixInverse * coord;
    coord = camera.clipToWorld * coord;
    coord /= coord.w;
    coord -= sphere.center;
    // setup viewing ray
    vec3 ray = normalize(coord.xyz - sphere.camPos);

    // calculate sphere-ray intersection
    // start ray at current coordinate and not at the camera
    float d1 = -dot(coord.xyz, ray);
    float d2s = dot(coord.xyz, coord.xyz) - d1 * d1;
    float radicand = sphere.radius * sphere.radius - d2s;

    if (radicand < 0.0) {  // no valid intersection found
        discard;
    }

    // calculate intersection point
    vec3 intersection = (d1 - sqrt(radicand)) * ray + coord.xyz;
    vec3 normal = intersection / sphere.radius;

    // shading
    vec4 glyphColor = sphere.color;

#if defined(ENABLE_TEXTURING)
    {
        float theta = acos(normal.z);
        float phi =
            sign(normal.y) * acos(normal.x / sqrt(normal.x * normal.x + normal.y * normal.y));
        vec2 uv = vec2(theta / PI, (phi + PI) / 2.0 / PI);
        glyphColor = blendBackToFront(texture(sphereTexture, uv), glyphColor);
    }
#endif

#if defined(ENABLE_LABELS)
    const vec2 labelSpherePos = vec2(0.5, 0.5);
    vec2 labelSphereSize = vec2(label.size, label.size * label.aspect);
    const ivec2 atlasDims = ivec2(30, 30);

    vec2 labelSphereStart = labelSpherePos - 0.5 * labelSphereSize;
    vec2 labelSphereEnd = labelSpherePos + 0.5 * labelSphereSize;
    vec2 sphereToTexture = 1.0 / (labelSphereSize * atlasDims);

    for (int i = 0; i < 6; ++i) {
        vec3 pos = rotations[i] * normal;

        float theta = acos(pos.z);
        float phi = sign(pos.y) * acos(pos.x / sqrt(pos.x * pos.x + pos.y * pos.y));
        vec2 uv = vec2(theta / PI, (phi + PI) / 2.0 / PI);

        if (all(lessThan(labelSphereStart, uv)) && all(lessThan(uv, labelSphereEnd))) {
            vec2 altasUv = (uv - labelSphereStart) * sphereToTexture;
            altasUv +=
                vec2(int(sphere.index / atlasDims.x), sphere.index % atlasDims.y) / atlasDims;
            vec4 labelColor = label.color * texture(label.tex, altasUv);
            labelColor.a *= sphere.color.a;
            glyphColor = blendBackToFront(labelColor, glyphColor);
        }
    }
#endif

    glyphColor.rgb = APPLY_LIGHTING(lighting, glyphColor.rgb, glyphColor.rgb, vec3(1.0f),
                                    intersection, normal, normalize(sphere.camPos - intersection));

    if (glyphColor.a < 0.01) discard;

    // depth correction for glyph
    float depth = glyphDepth(intersection + sphere.center.xyz, camera.worldToClip);
    if (clipGlypNearPlane(coord, camera.viewToWorld[2].xyz, lighting,
                          sphere.color.rgb * clipShadingFactor, glyphColor, depth)) {
        discard;
    }

#if defined(USE_FRAGMENT_LIST)
    abufferRender(ivec2(gl_FragCoord.xy), depth, glyphColor);
    discard;
#else
    FragData0 = glyphColor;
    gl_FragDepth = depth;
    PickingData = sphere.pickColor;
#endif
}
