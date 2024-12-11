/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

// Owned by the MeshRenderProcessorGL Processor

#include "utils/shading.glsl"

#if !defined(TEXCOORD_LAYER) && !defined(NORMALS_LAYER) \
    && !defined(VIEW_NORMALS_LAYER) && !defined(COLOR_LAYER)
#  define COLOR_LAYER
#endif

uniform LightParameters lighting;
uniform CameraParameters camera;
uniform sampler2D srcTexture;

in vec4 worldPosition_;
in vec3 normal_;
in vec3 viewNormal_;
in vec3 texCoord_;
in vec4 color_;
flat in vec4 pickColor_;

void main() {
    // Prevent invisible fragments from blocking other objects (e.g., depth/picking)
    if(color_.a == 0) { discard; }

    vec4 fragColor = texture(srcTexture, texCoord_.xy);
    vec3 toCameraDir_ = camera.position - worldPosition_.xyz;

    fragColor = vec4(1.0, 0.0, 0.0, 1.0); 
    if (texCoord_.x < 0.5 && texCoord_.y < 0.5) {
        fragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else if(texCoord_.x < 0.5 && texCoord_.y > 0.5) {
        fragColor = vec4(0.0, 0.0, 1.0, 1.0);
    }
    fragColor = texture(srcTexture, texCoord_.xy);
    fragColor.rgb = APPLY_LIGHTING(lighting, fragColor.rgb, fragColor.rgb, fragColor.rgb,
                                worldPosition_.xyz, normalize(normal_), normalize(toCameraDir_));

#ifdef COLOR_LAYER
    FragData0 = fragColor;
#endif
#ifdef TEXCOORD_LAYER
    tex_coord_out = vec4(texCoord_,1.0f);
#endif
#ifdef NORMALS_LAYER
    normals_out = vec4((normalize(normal_)+1.0)*0.5,1.0f);
#endif
#ifdef VIEW_NORMALS_LAYER
    view_normals_out = vec4((normalize(viewNormal_)+1.0)*0.5,1.0f);
#endif

    PickingData = pickColor_;
}
