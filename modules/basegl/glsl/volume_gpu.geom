/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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
#include "utils/sampler3d.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform VolumeParameters volumeParameters;

in int instanceID_[3];
in vec2 texCoord2D_[3];

// texCoord_ starts at 1/dims and goes to 1 - 1/dims
out vec4 texCoord_;

// dataposition_ ranges from zero to one, including zero and one.
out vec4 dataposition_;
out vec4 worldPos_;

void main() {
    float datapositionz = instanceID_[0] / (volumeParameters.dimensions.z - 1.0);
    float texCoordz = (instanceID_[0] * volumeParameters.reciprocalDimensions.z) +
                      (0.5 * volumeParameters.reciprocalDimensions.z);

    gl_Layer = instanceID_[0];

    for (int i = 0; i < gl_in.length(); ++i) {
        gl_Position = gl_in[i].gl_Position;
        texCoord_ = vec4(texCoord2D_[i], texCoordz, 1.0f);
        dataposition_ = vec4((texCoord2D_[i] - volumeParameters.reciprocalDimensions.xy * 0.5f) /
                                 (1.0f - volumeParameters.reciprocalDimensions.xy),
                             datapositionz, 1.0f);
        worldPos_ = volumeParameters.textureToWorld * texCoord_;
        EmitVertex();
    }

    EndPrimitive();
}
