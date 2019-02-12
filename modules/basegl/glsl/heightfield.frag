/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include "utils/shading.glsl"

uniform LightParameters lighting;
uniform GeometryParameters geometry;
uniform CameraParameters camera;

uniform sampler2D inportHeightfield;
uniform sampler2D inportTexture;
uniform sampler2D inportNormalMap;

uniform int terrainShadingMode = 0;
uniform int normalMapping = 0;

in vec4 worldPosition_;
in vec3 normal_;
in vec4 color_;
in vec3 texCoord_;


void main() {
    vec4 fragColor = color_;

    if (terrainShadingMode == 1) {  // color texture
        fragColor = texture(inportTexture, texCoord_.xy).rgba;
    } else if (terrainShadingMode == 2) {  // heightfield texture
        fragColor = vec4(texture(inportHeightfield, texCoord_.xy).rrr, 1.0);
    }

    // normal mapping
    vec3 normal;
    if (normalMapping == 1) {
        normal = texture(inportNormalMap, texCoord_.xy).rgb;
        normal = normalize(geometry.modelToWorldNormalMatrix * normal);
    } else {
        normal = normalize(normal_);
    }

    vec3 toCameraDir_ = camera.position - worldPosition_.xyz;
    fragColor.rgb = APPLY_LIGHTING(lighting, fragColor.rgb, fragColor.rgb, fragColor.rgb,
                                   worldPosition_.xyz, normal, normalize(toCameraDir_));

    FragData0 = fragColor;
}