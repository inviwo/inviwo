/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

 #include "utils/compositing.glsl"

 vec4 drawDVR(in vec4 result, in sampler2D tf, in vec3 samplePos, in vec4 voxel, in int channel, 
    in vec3 gradient, in mat4 textureToWorld, in LightParameters lighting, in vec3 toCameraDir, 
    in float t, in float tIncr, inout float tDepth) {
    vec4 color = APPLY_CLASSIFICATION(tf, voxel, channel);
    if (color.a > 0) {
        #if defined(SHADING_ENABLED)
        vec3 worldSpacePosition = (textureToWorld * vec4(samplePos, 1.0)).xyz;
        color.rgb = APPLY_LIGHTING(lighting, color.rgb, color.rgb, vec3(1.0), worldSpacePosition,
                                   -gradient, toCameraDir);
        #endif
        result = compositeDVR(result, color, t, tDepth, tIncr);
    }
    return result;
 }