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

#define DRAW_EDGES

#include "utils/shading.glsl"

uniform LightParameters lighting;
uniform CameraParameters camera;

in fData
{
    vec4 worldPosition;
    vec4 position;
    vec3 normal;
    vec3 viewNormal;
    vec4 color;
    float area;
#ifdef ALPHA_SHAPE
    vec3 sideLengths;
#endif
#ifdef DRAW_EDGES
    vec3 edgeCoordinates;
#endif
} frag;

#ifdef USE_FRAGMENT_LIST
#include "ABufferLinkedList.hglsl"
layout(pixel_center_integer) in vec4 gl_FragCoord;
#endif

//per-face settings
struct FaceRenderSettings
{
	vec4 externalColor;
	int colorSource;

    bool separateUniformAlpha;
    float uniformAlpha;

	int normalSource;
	int shadingMode;

    bool showEdges;
    vec4 edgeColor;
    float edgeOpacity;
};
uniform FaceRenderSettings renderSettings[2];
//GLSL does not support samplers within structures
uniform sampler1D transferFunction0; //front
uniform sampler1D transferFunction1; //back

//global alpha construction
struct AlphaSettings
{
    float uniformScale;
    float angleExp;
    float normalExp;
    float baseDensity;
    float densityExp;
    float shapeExp;
};
uniform AlphaSettings alphaSettings;

// In GLSL 4.5, we have better versions for derivatives
// use them if available
#ifdef GLSL_VERSION_450
#define dFdxFinest dFdxFine
#define dFdyFinest dFdyFine
#define fwidthFinest fwidthFine
#else
// fallback to the possible less precise dFdx / dFdy
#define dFdxFinest dFdx
#define dFdyFinest dFdy
#define fwidthFinest fwidth
#endif

#define M_PI 3.1415926535897932384626433832795
vec4 performShading()
{
    FaceRenderSettings settings = renderSettings[gl_FrontFacing ? 0 : 1];
    vec3 toCameraDir = normalize(camera.position - frag.worldPosition.xyz);

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
    vec3 normal = frag.normal;
    //TODO: switch on settings.normalSource. For now, assume ==0
    normal = normalize(normal);
    if (!gl_FrontFacing) normal = -normal; //backface -> invert normal

    // alpha
    float alpha = 1;
    if (settings.separateUniformAlpha) {
        //use per-face uniform alpha
        alpha = settings.uniformAlpha;
    } else {
        //custom alpha
        float angle = abs(dot(normal, toCameraDir));
#ifdef ALPHA_UNIFORM
        alpha *= alphaSettings.uniformScale;
#endif
#ifdef ALPHA_ANGLE_BASED
        alpha *= pow(acos(angle) * 2 / M_PI, alphaSettings.angleExp);
#endif
#ifdef ALPHA_NORMAL_VARIATION
        float nv_dzi = dFdxFinest (normal.z);
        float nv_dzj = dFdyFinest (normal.z);
        float nv_curvature = min(1, nv_dzi*nv_dzi + nv_dzj*nv_dzj);
        alpha *= pow(nv_curvature, alphaSettings.normalExp * 0.5);
#endif
#ifdef ALPHA_DENSITY
        float density_alpha = alphaSettings.baseDensity / (frag.area * angle * 100);
        alpha *= pow(min(1, density_alpha), alphaSettings.densityExp);
#endif
#ifdef ALPHA_SHAPE
        float shape_alpha = 4 * frag.area / 
            (sqrt(3) * max(max(frag.sideLengths.x * frag.sideLengths.y,
                frag.sideLengths.x*frag.sideLengths.z),
                frag.sideLengths.y*frag.sideLengths.z));
        alpha *= pow(min(1, shape_alpha), alphaSettings.shapeExp);
#endif
    }
    color.a *= alpha;

    //edges
#ifdef DRAW_EDGES
        if (settings.showEdges) {
        float isEdge = any(greaterThan(frag.edgeCoordinates,vec3(1))) ? 1.0f : 0.0f;
#ifdef DRAW_EDGES_SMOOTHING
        //smoothing
        float smoothing = min(1, fwidthFinest(isEdge)) * 0.5;
        float isEdgeSmoothed = (isEdge - 0.5f) * (1-smoothing) + 0.5f;
#else
        float isEdgeSmoothed = isEdge;
#endif
        //blend in edge color
        color.rgb = mix(color.rgb, settings.edgeColor.rgb, isEdgeSmoothed*min(1,settings.edgeOpacity));
        color.a = mix(color.a, 1, isEdgeSmoothed*max(0, settings.edgeOpacity-1));
    }
#endif

    // shading
    if (settings.shadingMode==1) {
        //Phong
        color.rgb = APPLY_LIGHTING(lighting, color.rgb, color.rgb, vec3(1.0f), frag.worldPosition.xyz,
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
    //float depth = frag.position.z / frag.position.w;
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
