/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

layout(location=9) in vec3 in_PickCoords_;

uniform GeometryParameters geometry;
uniform CameraParameters camera;

uniform vec4 overrideColor;

uniform vec2 scaling_ = vec2(1.0f);
uniform vec2 offset_ = vec2(0.0, 0.0);

uniform vec4 meshColors_[5];
uniform vec3 pickColors[10];

out vec4 worldPosition_;
out vec3 normal_;
out vec3 viewNormal_;
out vec4 color_;
out vec3 texCoord_;
flat out vec4 pickColor_;
out float pickingCoord_;
 
void main() {
    color_ = in_Color;
    texCoord_ = in_TexCoord;
    worldPosition_ = geometry.dataToWorld * in_Vertex;
    normal_ = geometry.dataToWorldNormalMatrix * in_Normal * vec3(1.0);
    viewNormal_ = (camera.worldToView * vec4(normal_,0)).xyz;
    gl_Position = camera.worldToClip * worldPosition_;

    // move mesh into correct 2D position on screen and scale it accordingly
    gl_Position /= gl_Position.w;
    gl_Position.xy *= scaling_;
    gl_Position.xy += offset_;

    pickingCoord_ = in_PickCoords_.x;

    int pickID = int(pickingCoord_ * 10.0 + 0.5);

#if defined(CUSTOM_COLOR)
#  if (CUSTOM_COLOR == 0)
    color_ = overrideColor;
#  else
    // apply RGB axis coloring
    color_ = meshColors_[pickID / 2];
#  endif // #if (CUSTOM_COLOR == 0)
#endif

    pickColor_ = vec4(pickColors[pickID], 1.0);
}
