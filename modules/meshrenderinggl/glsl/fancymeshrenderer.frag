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

#include "utils/shading.glsl"

uniform LightParameters lighting;
uniform CameraParameters camera;

in fData {
    vec4 worldPosition;
    vec4 position;
    vec3 normal;
#ifdef SEND_COLOR
    vec4 color;
#endif
#ifdef SEND_TEX_COORD
    vec2 texCoord;
#endif
#ifdef SEND_SCALAR
    float scalar;
#endif
    float area;
#ifdef ALPHA_SHAPE
    vec3 sideLengths;
#endif
#if defined(DRAW_EDGES) || defined(DRAW_SILHOUETTE)
    vec3 edgeCoordinates;
#endif
#ifdef DRAW_SILHOUETTE
    flat bvec3 silhouettes;
#endif
}
frag;

#ifdef USE_FRAGMENT_LIST
#include "oit/abufferlinkedlist.glsl"

// this is important for the occlusion query
layout(early_fragment_tests) in;

layout(pixel_center_integer) in vec4 gl_FragCoord;
#endif

// per-face settings
struct FaceRenderSettings {
    vec4 externalColor;
    int colorSource;

    bool separateUniformAlpha;
    float uniformAlpha;
    int shadingMode;

    bool showEdges;
    vec4 edgeColor;

    int hatchingMode;
    int hatchingSteepness;
    int hatchingFreqU;
    int hatchingFreqV;
    float hatchingModulationAnisotropy;
    float hatchingModulationOffset;
    vec4 hatchingColor;
    int hatchingBlending;
};
uniform FaceRenderSettings renderSettings[2];
// GLSL does not support samplers within structures
uniform sampler2D transferFunction0;  // front
uniform sampler2D transferFunction1;  // back

// global alpha construction
struct AlphaSettings {
    float minAlpha;
    float uniformScale;
    float angleExp;
    float normalExp;
    float baseDensity;
    float densityExp;
    float shapeExp;
};
uniform AlphaSettings alphaSettings;

// other global uniform settings
uniform vec4 silhouetteColor;

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

// Deprecated, unused, included directly into the code
float smoothPattern(float s, float t, int ls, int lt, int steepness, bool hu, bool hv) {
    s *= pow(2, -ls);
    t *= pow(2, -lt);
    float c = 1;
    if (hu) c *= 1 - pow(0.5f + 0.5f * sin(s * 2 * M_PI), steepness);
    if (hv) c *= 1 - pow(0.5f + 0.5f * sin(t * 2 * M_PI), steepness);
    return c;
}

// x in [0,1]: phase
// k in [-1,1]: anisotropy
float hatchingModulation(float x, float k) {
    x = fract(x);
    float e = tan(abs(k) * 0.45 * M_PI) + 1;
    float b = 0.5 + (k >= 0 ? -1 : 1) * (x - 0.5);
    return 0.5 * (1 - cos(pow(b, e) * 2 * M_PI));
}

vec4 performShading() {
    FaceRenderSettings settings = renderSettings[gl_FrontFacing ? 0 : 1];
    vec3 toCameraDir = normalize(camera.position - frag.worldPosition.xyz);

    //==================================================
    // BASE COLOR
    //==================================================
    vec4 color = vec4(1.0);
    if (settings.colorSource == 0) {
        // vertex color
#ifdef SEND_COLOR
        color = frag.color;
#endif
    } else if (settings.colorSource == 1) {
        // transfer function
#ifdef SEND_SCALAR
        if (gl_FrontFacing) {
            color = texture(transferFunction0, vec2(frag.scalar));
        } else {
            color = texture(transferFunction1, vec2(frag.scalar));
        }
#endif
    } else if (settings.colorSource == 2) {
        // external color
        color = settings.externalColor;
        color.a = 1;
    }

    //==================================================
    // NORMAL VECTOR
    //==================================================
    vec3 normal = frag.normal;
    normal = normalize(normal);
    if (!gl_FrontFacing) normal = -normal;  // backface -> invert normal WIPASPDASDPPASD

    //==================================================
    // ALPHA
    //==================================================
    float alpha = 1;
    if (settings.separateUniformAlpha) {
        // use per-face uniform alpha
        alpha = settings.uniformAlpha;
    } else {
        // custom alpha
        float angle = abs(dot(normal, toCameraDir));
#ifdef ALPHA_UNIFORM
        alpha *= max(alphaSettings.uniformScale, alphaSettings.minAlpha);
#endif
#ifdef ALPHA_ANGLE_BASED
        alpha *= pow(acos(angle) * 2 / M_PI, alphaSettings.angleExp);
        alpha += alphaSettings.minAlpha;
#endif
#ifdef ALPHA_NORMAL_VARIATION
        float nv_dzi = dFdxFinest(normal.z);
        float nv_dzj = dFdyFinest(normal.z);
        float nv_curvature = min(1, nv_dzi * nv_dzi + nv_dzj * nv_dzj + 0.0000001);
        alpha *= pow(nv_curvature, alphaSettings.normalExp * 0.5) * 10;
        alpha += alphaSettings.minAlpha;
#endif
#ifdef ALPHA_DENSITY
        float density_alpha = alphaSettings.baseDensity / (frag.area * angle * 100);
        alpha *= pow(density_alpha, alphaSettings.densityExp);
        alpha += alphaSettings.minAlpha;
#endif
#ifdef ALPHA_SHAPE
        float shape_alpha = 4 * frag.area /
                            (sqrt(3) * max(max(frag.sideLengths.x * frag.sideLengths.y,
                                               frag.sideLengths.x * frag.sideLengths.z),
                                           frag.sideLengths.y * frag.sideLengths.z));
        alpha *= pow(shape_alpha, alphaSettings.shapeExp);
        alpha += alphaSettings.minAlpha;
#endif
    }
    color.a *= min(1, alpha);

    //==================================================
    // EDGES
    //==================================================
#if defined(DRAW_EDGES) || defined(DRAW_SILHOUETTE)
    {
        // obtain silhouette flags
#ifdef DRAW_SILHOUETTE
        bvec3 silhouettes = frag.silhouettes;
#else
        bvec3 silhouettes = bvec3(false);
#endif

        // compute if we are on an edge or not
#ifdef DRAW_EDGES_SMOOTHING
        // smoothing
        float isEdgeSmoothed = 1;
        float isSilhouettesSmoothed = 1;
        vec3 dx = dFdxFinest(frag.edgeCoordinates);
        vec3 dy = dFdyFinest(frag.edgeCoordinates);
        for (int i = 0; i < 3; ++i) {
            // Distance to the line
            float d = abs(frag.edgeCoordinates[i] - 1) / length(vec2(dx[i], dy[i]));
            float fraction = frag.edgeCoordinates[i] < 1 ? (1 - (0.5 * d + 0.5)) : (0.5 * d + 0.5);
            isEdgeSmoothed *= 1 - clamp(fraction, 0, 1);
            if (silhouettes[i]) isSilhouettesSmoothed *= 1 - clamp(fraction, 0, 1);
        }
        isEdgeSmoothed = 1 - isEdgeSmoothed;
        isSilhouettesSmoothed = 1 - isSilhouettesSmoothed;
#else
        float isEdge = any(greaterThan(frag.edgeCoordinates, vec3(1))) ? 1.0f : 0.0f;
        float isEdgeSmoothed = isEdge;
        float isSilhouettesSmoothed =
            any(greaterThan(frag.edgeCoordinates, vec3(1)) && silhouettes) ? 1.0f : 0.0f;
#endif
        // blend in edge color
        if (settings.showEdges) {
            color.rgb = mix(color.rgb, settings.edgeColor.rgb,
                            isEdgeSmoothed * min(1, settings.edgeColor.a));
            color.a = mix(color.a, 1, isEdgeSmoothed * max(0, settings.edgeColor.a - 1));
        }
#ifdef DRAW_SILHOUETTE
        // blend in silhouette
        color.rgb =
            mix(color.rgb, silhouetteColor.rgb, isSilhouettesSmoothed * min(1, silhouetteColor.a));
        color.a = mix(color.a, 1, isSilhouettesSmoothed * max(0, silhouetteColor.a - 1));
#endif
    }
#endif

    //==================================================
    // HATCHING
    //==================================================
#ifdef SEND_TEX_COORD
    vec2 texCoord = frag.texCoord;
    float stripeStrength = 1;
    if (settings.hatchingMode == 1 || settings.hatchingMode == 3 || settings.hatchingMode == 4) {
        // hatch in u-direction
        float lambdaS = length(vec2(dFdxFinest(texCoord.x), dFdyFinest(texCoord.x))) + 0.000000001;
        float ls = log(lambdaS) / log(2);
        ls += settings.hatchingFreqU;
        int lsInt = int(floor(ls));
        float lsFrac = ls - lsInt;
        stripeStrength *= mix(1 - pow(0.5f + 0.5f * sin(texCoord.x * pow(2, -lsInt) * 2 * M_PI),
                                      settings.hatchingSteepness),
                              1 - pow(0.5f + 0.5f * sin(texCoord.x * pow(2, -lsInt - 1) * 2 * M_PI),
                                      settings.hatchingSteepness),
                              lsFrac);
    }
    if (settings.hatchingMode == 2 || settings.hatchingMode == 3 || settings.hatchingMode == 5) {
        // hatch in v-direction
        float lambdaT = length(vec2(dFdxFinest(texCoord.y), dFdyFinest(texCoord.y))) + 0.000000001;
        float lt = log(lambdaT) / log(2);
        lt += settings.hatchingFreqV;
        int ltInt = int(floor(lt));
        float ltFrac = lt - ltInt;
        stripeStrength *= mix(1 - pow(0.5f + 0.5f * sin(texCoord.y * pow(2, -ltInt) * 2 * M_PI),
                                      settings.hatchingSteepness),
                              1 - pow(0.5f + 0.5f * sin(texCoord.y * pow(2, -ltInt - 1) * 2 * M_PI),
                                      settings.hatchingSteepness),
                              ltFrac);
    }
    if (settings.hatchingMode == 4) {
        // modulation in u-direction
        float s = texCoord.y * pow(2, settings.hatchingFreqV);
        stripeStrength =
            1 - (1 - stripeStrength) * hatchingModulation(s + settings.hatchingModulationOffset,
                                                          settings.hatchingModulationAnisotropy);
    } else if (settings.hatchingMode == 5) {
        // modulation in v-direction
        float t = texCoord.x * pow(2, settings.hatchingFreqU);
        stripeStrength =
            1 - (1 - stripeStrength) * hatchingModulation(t + settings.hatchingModulationOffset,
                                                          settings.hatchingModulationAnisotropy);
    }
    // blend into color
    color.rgb =
        mix(color.rgb, settings.hatchingColor.rgb, (1 - stripeStrength) * settings.hatchingColor.a);
    if (settings.hatchingBlending == 1) {
        color.a =
            mix(color.a, 1, (1 - stripeStrength) * settings.hatchingColor.a);  // additive alpha
    }
#endif

    //==================================================
    // SHADING
    //==================================================
    if (settings.shadingMode == 1) {
        // Phong
        color.rgb = APPLY_LIGHTING(lighting, color.rgb, color.rgb, vec3(1.0f),
                                   frag.worldPosition.xyz, normal, toCameraDir);
    }

    // done
    return color;
}

void main() {
    vec4 fragColor = performShading();

#ifdef USE_FRAGMENT_LIST
    // fragment list rendering
    if (fragColor.a > 0.0) {
        ivec2 coords = ivec2(gl_FragCoord.xy);
        float depth = gl_FragCoord.z;
        abufferRender(coords, depth, fragColor);
    }
    discard;

#else
    // traditional rendering
    FragData0 = fragColor;
    PickingData = vec4(0.0, 0.0, 0.0, 0.0);
#endif
}
