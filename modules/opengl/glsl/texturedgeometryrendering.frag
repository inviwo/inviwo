/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

uniform LightParameters light_;
uniform CameraParameters camera_;

in vec4 worldPosition_;
in vec3 texCoord_;
uniform sampler2D colorTex_;
// Use normals from buffer by default.
// Optinally aquire them from a texture
#if USE_NORMAL_TEXTURE
uniform GeometryParameters geometry_;
uniform sampler2D normalTex_;
#else
in vec3 normal_;
#endif

void main() {
    vec4 fragColor;
    vec4 color = texture(colorTex_, texCoord_.xy);
    fragColor.w = color.w;
#if USE_NORMAL_TEXTURE
    // Normals in texture are assumed to lie in [0 1]^3.
    // First transform them to [-1 1]^3 and then
    // from data to world space
    vec3 normal_ = geometry_.dataToWorldNormalMatrix * ( 2.0 * texture(normalTex_, texCoord.xy).xyz - 1.0 );
#endif
    vec3 toCameraDir_ = camera_.position - worldPosition_.xyz;
    fragColor.rgb = APPLY_LIGHTING(light_, color.rgb, color.rgb, vec3(1.0), worldPosition_.xyz,
                                   normalize(normal_), normalize(toCameraDir_));

    FragData0 = fragColor;
}