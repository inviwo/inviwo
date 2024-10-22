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
#ifndef IVW_SHADING_GLSL
#define IVW_SHADING_GLSL

#if !defined(APPLY_LIGHTING)
// fall-back to no shading in case APPLY_LIGHTING is not defined
#  define APPLY_LIGHTING(lighting, materialAmbientColor, materialDiffuseColor, \
    materialSpecularColor, position, normal, toCameraDir) \
    materialAmbientColor
#endif

#if !defined(SHADING_NORMAL)
// the SHADING_NORMAL macro is used to adapt the face normal for frontside, backside, or 
// two-sided shading/illumination.
// Values:
//  + 0: frontside only, no changes to input normal
//  + 1: backside only, invert normal direction
//  + 2: two-sided, adjust normal based on primitive orientation
//
// see orientedShadingNormal()
#  define SHADING_NORMAL 0
#endif

#include "utils/structs.glsl" //! #include "./structs.glsl"

// Returns whether the primitive of the current fragment is facing toward the camera.
// Can _only_ be used in the fragment shader.
bool isFacingForward(in vec3 normal, in vec3 worldPosition) {
#if defined(__APPLE__) || !defined(gl_FrontFacing)
    // gl_FrontFacing is not working correctly on MacOS and is not defined in compute shaders
    // Credit: https://makc3d.wordpress.com/2015/09/17/alternative-to-gl_frontfacing/
    vec3 fdx = dFdx(worldPosition);
    vec3 fdy = dFdy(worldPosition);
    return dot(normal, cross(fdx, fdy)) > 0;
#else
    return gl_FrontFacing;
#endif
}

vec3 orientedShadingNormal(in vec3 normal, in vec3 worldPosition) {
#if defined(SHADING_NORMAL) && (SHADING_NORMAL == 0)
    return normal;
#elif defined(SHADING_NORMAL) && (SHADING_NORMAL == 1)
    return -normal;
#elif defined(SHADING_NORMAL) && (SHADING_NORMAL == 2)
    if (isFacingForward(normal, worldPosition)) {
        return normal;
    } else {
        return -normal;
    }
#else
    return normal;
#endif
}

// default material uses the supplied color for the diffuse and ambient material terms
// as well as white for the specular term (as used for volume and mesh rendering).
MaterialColors defaultMaterialColors(in vec3 diffuseColor) {
    return MaterialColors(diffuseColor, diffuseColor, vec3(1.0));
}

ShadingParameters defaultShadingParameters(in MaterialColors materialColors) {
    ShadingParameters p;
    p.colors = materialColors;
    p.normal = vec3(0);
    p.worldPosition = vec3(0);
    p.lightIntensity = vec3(0);

    return p;
}

ShadingParameters defaultShadingParameters() {
    return defaultShadingParameters(defaultMaterialColors(vec3(0)));
}
ShadingParameters shading(in vec3 diffuseColor) {
    return ShadingParameters(defaultMaterialColors(diffuseColor), vec3(0), vec3(0), vec3(0));
}
ShadingParameters shading(in vec3 diffuseColor, in vec3 normal) {
    return ShadingParameters(defaultMaterialColors(diffuseColor), normal, vec3(0), vec3(0));
}
ShadingParameters shading(in vec3 diffuseColor, in vec3 normal, in vec3 worldPosition) {
    return ShadingParameters(defaultMaterialColors(diffuseColor), normal, worldPosition, vec3(0));
}

// Calculate the diffuse term based on the Lambertian reflection model
float diffuse(in vec3 normal, in vec3 toLightDir) {
    return max(dot(normal, toLightDir), 0.0);
}

// Calculate the specular term of the Blinn-Phong model
float specularBlinnPhong(in vec3 normal, in vec3 toLightDir,
                         in vec3 toCameraDir, in float specularExponent) {
    vec3 halfway = normalize(toCameraDir + toLightDir);
    // check for special case where the light source is exactly opposite
    // to the view direction, i.e. the length of the halfway vector is zero
    if (dot(halfway, halfway) < 1.0e-6) {  // check for squared length
        return 0.0;
    } else {
        return pow(max(dot(normal, halfway), 0.0), specularExponent);
    }
}

// Calculate the specular term of the Phong model
float specularPhong(in vec3 normal, in vec3 toLightDir,
                    in vec3 toCameraDir, in float specularExponent) {
    // Compute reflection (note that glsl uses incident direction)
    // Corresponds to 2.0*dot(toLightDir, normal)*normal - toLightDir;
    vec3 r = reflect(-toLightDir, normal);
    if (dot(toLightDir, normal) < 0.0) {
        return 0.0;
    } else {
        // scale specular exponent so that it roughly matches the one of the Blinn-Phong model
        return pow(max(dot(r, toCameraDir), 0.0), specularExponent * 0.25);
    }
}

// Functions to apply different shading modes.
// All positions and directions should be in world space!
vec3 shadeAmbient(LightParameters light_, vec3 materialAmbientColor) {
    return materialAmbientColor * light_.ambientColor;
}

vec3 shadeDiffuse(LightParameters light_, vec3 materialDiffuseColor, vec3 position, vec3 normal) {
    vec3 toLightDir = normalize(light_.position - position);
    return diffuse(normal, toLightDir) * materialDiffuseColor * light_.diffuseColor;
}

vec3 shadeSpecular(LightParameters light_, vec3 materialSpecularColor, vec3 position, vec3 normal,
                   vec3 toCameraDir) {
    vec3 toLightDir = normalize(light_.position - position);
    return specularBlinnPhong(normal, toLightDir, toCameraDir, light_.specularExponent) *
        materialSpecularColor * light_.specularColor;
}

vec3 shadeBlinnPhong(LightParameters light_, vec3 materialAmbientColor, vec3 materialDiffuseColor,
                     vec3 materialSpecularColor, vec3 position, vec3 normal, vec3 toCameraDir) {
    vec3 toLightDir = normalize(light_.position - position);
    vec3 resAmb = materialAmbientColor * light_.ambientColor;
    vec3 resDiff = diffuse(normal, toLightDir) * materialDiffuseColor * light_.diffuseColor;
    vec3 resSpec = specularBlinnPhong(normal, toLightDir, toCameraDir, light_.specularExponent) *
        materialSpecularColor * light_.specularColor;

    return resAmb + resDiff + resSpec;
}

vec3 shadePhong(LightParameters light_, vec3 materialAmbientColor, vec3 materialDiffuseColor,
                vec3 materialSpecularColor, vec3 position, vec3 normal, vec3 toCameraDir) {
    vec3 toLightDir = normalize(light_.position - position);
    vec3 resAmb = materialAmbientColor * light_.ambientColor;
    vec3 resDiff = diffuse(normal, toLightDir) * materialDiffuseColor * light_.diffuseColor;
    vec3 resSpec = specularPhong(normal, toLightDir, toCameraDir, light_.specularExponent) *
        materialSpecularColor * light_.specularColor;
    return resAmb + resDiff + resSpec;
}

vec3 shadeBlinnPhongTwoSided(LightParameters light_, vec3 materialAmbientColor, vec3 materialDiffuseColor,
                     vec3 materialSpecularColor, vec3 position, vec3 normal, vec3 toCameraDir) {
    return shadeBlinnPhong(light_, materialAmbientColor, materialDiffuseColor, materialSpecularColor,
                           position, normal, toCameraDir);
}

vec3 shadePhongTwoSided(LightParameters light_, vec3 materialAmbientColor, vec3 materialDiffuseColor,
                vec3 materialSpecularColor, vec3 position, vec3 normal, vec3 toCameraDir) {
    return shadePhong(light_, materialAmbientColor, materialDiffuseColor, materialSpecularColor,
                      position, normal, toCameraDir);
}

vec3 applyLighting(in LightParameters lightsource, in ShadingParameters shading, in vec3 viewDir) {
    return APPLY_LIGHTING(lightsource, shading.colors.ambient, shading.colors.diffuse,
                          shading.colors.specular, shading.worldPosition, shading.normal, viewDir);
}

#endif
