/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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
uniform CameraParameters camera;

uniform vec4 overrideColor;

smooth in vec4 position_;

#ifdef USE_FRAGMENT_LIST
#include "ABufferLinkedList.hglsl"
layout(pixel_center_integer) in vec4 gl_FragCoord;
#endif

struct FaceRenderSettings
{
	sampler1D transferFunction;
	vec4 externalColor;
	int colorSource;

	int alphaMode;
	float alphaScale;

	int normalSource;

	int shadingMode;
};
uniform FaceRenderSettings frontSettings;
uniform FaceRenderSettings backSettings;

in vec4 worldPosition_;
in vec3 normal_;
in vec3 viewNormal_;
in vec4 color_;

void main() {
    vec4 fragColor = vec4(1.0);
    vec3 toCameraDir_ = camera.position - worldPosition_.xyz;
#ifdef OVERRIDE_COLOR_BUFFER
    vec4 color = overrideColor;
    color.a = 1;
#else
    vec4 color = color_;
#endif
    color.a *= frontSettings.alphaScale;

    // shading
    fragColor.rgb = APPLY_LIGHTING(lighting, color.rgb, color.rgb, vec3(1.0f), worldPosition_.xyz,
                                   normalize(normal_), normalize(toCameraDir_));
    fragColor.a = color.a;

#ifdef USE_FRAGMENT_LIST

    //fragment list rendering
    ivec2 coords=ivec2(gl_FragCoord.xy);
    float depth = position_.z / position_.w;
    abufferRender(coords, depth, fragColor);
    discard;

#else
    //traditional rendering

#ifdef COLOR_LAYER
    FragData0 = fragColor;
#endif

#endif

}
