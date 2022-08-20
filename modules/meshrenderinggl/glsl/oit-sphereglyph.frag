/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#ifdef USE_FRAGMENT_LIST
#include "oit/abufferlinkedlist.glsl"

// this is important for the occlusion query
layout(early_fragment_tests) in;

layout(pixel_center_integer) in vec4 gl_FragCoord;
#endif

uniform CameraParameters camera;
uniform LightParameters lighting;
uniform float uniformAlpha = 1.0;

uniform vec4 viewport; // holds viewport offset x, offset y, 2 / viewport width, 2 / viewport height
uniform float clipShadingFactor = 0.9;

in vec4 color;
in float radius;
in vec3 camPos;
in vec4 center;
flat in vec4 pickColor;

void main() {
    vec4 pixelPos = gl_FragCoord;
    pixelPos.xy -= viewport.xy;
    // transform fragment coordinates from window coordinates to view coordinates.
    vec4 coord = pixelPos
        * vec4(viewport.z, viewport.w, 2.0, 0.0)
        + vec4(vec3(-1.0), 1.0);

    // transform fragment coord into object space
    //coord = gl_ModelViewProjectionMatrixInverse * coord;
    coord = camera.clipToWorld * coord;
    coord /= coord.w;
    coord -= center;
    // setup viewing ray
    vec3 ray = normalize(coord.xyz - camPos);
    
    // calculate sphere-ray intersection
    // start ray at current coordinate and not at the camera
    float d1 = -dot(coord.xyz, ray);
    float d2s = dot(coord.xyz, coord.xyz) - d1*d1;
    float radicand = radius*radius - d2s;
    
    if (radicand < 0.0) {
        // no valid intersection found
        discard;
    }

    // calculate intersection point
    vec3 intersection = (d1 - sqrt(radicand))*ray + coord.xyz;
    
    vec3 normal = intersection / radius;

    // shading
    vec4 glyphColor;
    glyphColor.rgb = APPLY_LIGHTING(lighting, color.rgb, color.rgb, vec3(1.0f), intersection,
                               normal, normalize(camPos - intersection));
#if defined(UNIFORM_ALPHA)
    glyphColor.a = uniformAlpha;
#else
    glyphColor.a = color.a;
#endif // UNIFORM_ALPHA
    if (glyphColor.a < 0.01) discard;

    // depth correction for glyph
    float depth = glyphDepth(intersection + center.xyz, camera.worldToClip);

    if (clipGlypNearPlane(coord, camPos - coord.xyz, camera.viewToWorld[2].xyz, lighting,
        color.rgb * clipShadingFactor, glyphColor, depth)) {
        discard;
    }

#if defined(USE_FRAGMENT_LIST)
    // fragment list rendering
    ivec2 coords = ivec2(gl_FragCoord.xy);
    abufferRender(coords, depth, glyphColor);
    discard;
#else
    FragData0 = glyphColor;
    gl_FragDepth = depth;
    PickingData = pickColor;
#endif  // USE_FRAGMENT_LIST
}
