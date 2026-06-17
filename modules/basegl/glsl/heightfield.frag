/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

uniform LightParameters lighting;
uniform GeometryParameters geometry;
uniform CameraParameters camera;

uniform sampler2D heightfield;
uniform ImageParameters heightfieldParameters;
uniform sampler2D colorTexture;
uniform sampler2D normalmap;

uniform int terrainShadingMode = 0;
uniform int normalMapping = 0;

in Vertex {
    vec4 worldPos;
    vec3 normal;
    vec4 color;
    vec3 texCoord;
} fragment;


void main() {
    vec4 fragColor = fragment.color;

    if (terrainShadingMode == 1) {  // color texture
        fragColor = texture(colorTexture, fragment.texCoord.xy);
    } else if (terrainShadingMode == 2) {  // heightfield texture
        fragColor = vec4(texture(heightfield, fragment.texCoord.xy).rrr, 1.0);
    }

    // normal mapping
    vec3 normal;
    if (normalMapping == 1) {
        normal = texture(normalmap, fragment.texCoord.xy).rgb * 2.0 - 1.0;
        normal = normalize(geometry.modelToWorldNormalMatrix * normal);
    } else {
        normal = normalize(fragment.normal);
    }

    normal = orientedShadingNormal(normal, fragment.worldPos.xyz);

    vec3 toCameraDir = normalize(camera.position - fragment.worldPos.xyz);

    ShadingParameters shadingParams = shading(fragColor.rgb, normal, fragment.worldPos.xyz);
    fragColor.rgb = applyLighting(lighting, shadingParams, toCameraDir);

    FragData0 = fragColor;
}