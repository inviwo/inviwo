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
uniform GeometryParameters geometry_;
uniform CameraParameters camera_;

uniform sampler2D inportHeightfield_;
uniform sampler2D inportTexture_;
uniform sampler2D inportNormalMap_;

uniform int terrainShadingMode_ = 0;
uniform int normalMapping_ = 0;
uniform int lighting_ = 0;

in vec4 worldPosition_;
in vec3 normal_;
in vec4 color_;
in vec3 texCoord_;

in float height_;

void main() {
    vec4 fragColor = color_;
    
    if (terrainShadingMode_ == 1) { // color texture
        fragColor = texture(inportTexture_, texCoord_.xy).rgba;
    }
    else if (terrainShadingMode_ == 2) { // heightfield texture
        fragColor = vec4(texture(inportHeightfield_, texCoord_.xy).rrr, 1.0);
    }
/*    else {
        if (height_ < 0.1) 
            fragColor = vec4(0.2, 0.2, 0.9, 1.0);
        else if (height_ < 0.2)
            fragColor = vec4(0.4, 0.35, 0.1, 1.0);
        else if (height_ < 0.35)
            fragColor = vec4(0.2, 0.5, 0.2, 1.0);
        else if (height_ < 1.0)
            fragColor = vec4(0.5, 0.5, 0.5, 1.0);
    }
    */
    
    // normal mapping
    vec3 normal;
    if (normalMapping_ == 1) {
        normal = texture(inportNormalMap_, texCoord_.xy).rgb;
        normal = normalize(geometry_.modelToWorldNormalMatrix * normal);
    }
    else {
        normal = normalize(normal_);
    }
    
    vec3 toCameraDir_ = camera_.position - worldPosition_.xyz;
    fragColor.rgb = APPLY_LIGHTING(light_, fragColor.rgb, fragColor.rgb, fragColor.rgb, worldPosition_.xyz, normal, normalize(toCameraDir_));
    
 //   fragColor.rgb = normalize(normal);
 //fragColor.rgb = texCoord_;
    FragData0 = fragColor;
}