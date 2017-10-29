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

smooth in vec4 position_;

#ifdef USE_FRAGMENT_LIST
#include "ABufferLinkedList.hglsl"
layout(pixel_center_integer) in vec4 gl_FragCoord;
#endif

//GLSL does not support samplers within structures
uniform sampler1D transferFunction0; //front
uniform sampler1D transferFunction1; //back
struct FaceRenderSettings
{
	vec4 externalColor;
	int colorSource;

	int alphaMode;
	float alphaScale;

	int normalSource;

	int shadingMode;
};
uniform FaceRenderSettings renderSettings[2];

in vec4 worldPosition_;
in vec3 normal_;
in vec3 viewNormal_;
in vec4 color_;

#define M_PI 3.1415926535897932384626433832795
vec4 performShading()
{
    FaceRenderSettings settings = renderSettings[gl_FrontFacing ? 0 : 1];
    vec3 toCameraDir = normalize(camera.position - worldPosition_.xyz);

    // base color
    vec4 color = vec4(1.0);
    if (settings.colorSource == 0) {
        // vertex color
        // TODO
    } else if (settings.colorSource == 1) {
        // transfer function
        // TODO
    } else if (settings.colorSource == 2) {
        // external color
        color = settings.externalColor;
        color.a = 1;
    }

    // normal vector
    vec3 normal = normal_;
    //TODO: switch on settings.normalSource. For now, assume ==0
    normal = normalize(normal);
    if (!gl_FrontFacing) normal = -normal; //backface -> invert normal

    // alpha
    float alpha = 1;
    if (settings.alphaMode==1) {
        //angle-based
        alpha = acos(abs(dot(normal, toCameraDir))) * 2 / M_PI;
    }
    color.a *= alpha * settings.alphaScale;

    // shading
    if (settings.shadingMode==1) {
        //Phong
        color.rgb = APPLY_LIGHTING(lighting, color.rgb, color.rgb, vec3(1.0f), worldPosition_.xyz,
                                   normal, toCameraDir);
    } else if (settings.shadingMode==2) {
        //PBR
    }

    //done
    return color;
}

void main() {
    vec4 fragColor = performShading();

#ifdef USE_FRAGMENT_LIST

    //fragment list rendering
    ivec2 coords=ivec2(gl_FragCoord.xy);
    //float depth = position_.z / position_.w;
    float depth = gl_FragCoord.z;
    abufferRender(coords, depth, fragColor);
    discard;

#else
    //traditional rendering

#ifdef COLOR_LAYER
    FragData0 = fragColor;
#endif

#endif

}
