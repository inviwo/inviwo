/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef GLSL_VERSION_150
#extension GL_EXT_gpu_shader4: enable
#extension GL_EXT_geometry_shader4: enable
#endif

#include "utils/structs.glsl"
#include "utils/sampler3d.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform VolumeParameters lightVolumeParameters_;

uniform mat4 permMatInv_;
uniform mat4 permMatInvSec_;

in int instanceID_[3];
in vec2 texCoord2D_[3];

out vec4 texCoord_;

out vec4 permutedPosInv_;
out vec4 permutedPosInvSec_;

void main() {
    int i;
    texCoord_.z = (instanceID_[0] * lightVolumeParameters_.reciprocalDimensions.z) 
					+ (0.5 * lightVolumeParameters_.reciprocalDimensions.z);
    texCoord_.w = 1.f;
    gl_Layer = instanceID_[0];

    for (i = 0; i<gl_in.length(); ++i) {
#ifndef GLSL_VERSION_150
        gl_Position = gl_PositionIn[i];
#else
        gl_Position = gl_in[i].gl_Position;
#endif
        texCoord_.xy = texCoord2D_[i];
        permutedPosInv_ = permMatInv_ * texCoord_;
        permutedPosInvSec_ = permMatInvSec_ * texCoord_;
        EmitVertex();
    }

    EndPrimitive();
}