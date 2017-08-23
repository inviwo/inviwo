/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

layout(location=6) in vec3 in_PickCoords_;

uniform GeometryParameters geometry_;
uniform CameraParameters camera_;

uniform vec4 overrideColor_ = vec4(-1.0);

uniform vec2 scaling_ = vec2(1.0);
uniform vec2 offset_ = vec2(0.0);

uniform vec3 pickColor_ = vec3(-1.0);

out vec4 worldPosition_;
out vec3 normal_;
out vec3 viewNormal_;
out vec4 color_;
out vec3 texCoord_;
out vec4 pickingColor_;
 
void main() {
    color_ = in_Color;
    texCoord_ = in_TexCoord;

    worldPosition_ = geometry_.dataToWorld * in_Vertex;
    normal_ = geometry_.dataToWorldNormalMatrix * in_Normal * vec3(1.0);
    viewNormal_ = (camera_.worldToView * vec4(normal_,0)).xyz;
    gl_Position = camera_.worldToClip * worldPosition_;

    // move mesh into correct 2D position on screen and scale it accordingly
    
    gl_Position /= gl_Position.w;
    gl_Position.xy *= scaling_;
    gl_Position.xy += offset_;
    

    if (overrideColor_.a > -0.1) {
    	color_ = overrideColor_;
    }

	pickingColor_ = (pickColor_.r < 0.0) ? vec4(0.0) : vec4(pickColor_, 1.0);
}
