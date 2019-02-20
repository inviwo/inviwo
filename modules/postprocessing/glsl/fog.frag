/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

//! #version 330
//! layout(location = 0) out vec4 FragData0;
//! #include "../../opengl/glsl/utils/structs.glsl"
//! #include "../../opengl/glsl/utils/depth.glsl"

#include "utils/structs.glsl"
#include "utils/depth.glsl"
#include "fog.glsl"

uniform vec3 fogColor;
uniform float fogDensity = 2.0;
uniform vec2 range = vec2(0, 1);

uniform CameraParameters camera;
uniform sampler2D inportColor;
uniform sampler2D inportDepth;

void main() {
    vec4 color = texelFetch(inportColor, ivec2(gl_FragCoord.xy), 0);
    float depth = texelFetch(inportDepth, ivec2(gl_FragCoord.xy), 0).r;

    float d = convertDepthScreenToView(camera, depth);
    // offset depth so that fog starts at given range
    float minFogDepth = (camera.farPlane - camera.nearPlane) * range.x;
    float fogDepth = (camera.farPlane - camera.nearPlane) * range.y - minFogDepth;
    d -= camera.nearPlane + minFogDepth;

    color.rgb = computeFog(color.rgb, clamp(d, 0, fogDepth), fogColor, fogDensity);

    //color.rgb *= color.a;
    FragData0 = color;
}
