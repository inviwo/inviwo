/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "utils/structs.glsl"

// Helper functions to calculate the shading
vec3 shadeDiffuseCalculation(LightParameters light_, vec3 materialDiffuseColor, vec3 normal,
                             vec3 toLightDir) {
    return materialDiffuseColor * light_.diffuseColor * max(dot(normal, toLightDir), 0.0);
}

vec3 shadeSpecularBlinnPhongCalculation(LightParameters light_, vec3 materialSpecularColor,
                                        vec3 normal, vec3 toLightDir, vec3 toCameraDir) {
    vec3 halfway = toCameraDir + toLightDir;

    // check for special case where the light source is exactly opposite
    // to the view direction, i.e. the length of the halfway vector is zero
    if (dot(halfway, halfway) < 1.0e-6) {  // check for squared length
        return vec3(0.0);
    } else {
        halfway = normalize(halfway);
        return materialSpecularColor * light_.specularColor *
               pow(max(dot(normal, halfway), 0.0), light_.specularExponent);
    }
}

vec3 shadeSpecularPhongCalculation(LightParameters light_, vec3 materialSpecularColor, vec3 normal,
                                   vec3 toLightDir, vec3 toCameraDir) {
    // Compute reflection (not that glsl uses incident direction)
    // Equivalent to: 2.0*dot(toLightDir, normal)*normal - toLightDir;
    vec3 r = reflect(-toLightDir, normal);
   
    return materialSpecularColor * light_.specularColor *
           pow(max(dot(r, toCameraDir), 0.0), light_.specularExponent * 0.25);
}

// Functions to apply different shading modes.
// All positions and directions should be in world space!
vec3 shadeAmbient(LightParameters light_, vec3 materialAmbientColor) {
    return materialAmbientColor * light_.ambientColor;
}

vec3 shadeDiffuse(LightParameters light_, vec3 materialDiffuseColor, vec3 position, vec3 normal) {
    return shadeDiffuseCalculation(light_, materialDiffuseColor, normal,
                                   normalize(light_.position - position));
}

vec3 shadeSpecular(LightParameters light_, vec3 materialSpecularColor, vec3 position, vec3 normal,
                   vec3 toCameraDir) {
    return shadeSpecularPhongCalculation(light_, materialSpecularColor, normal,
                                         normalize(light_.position - position), toCameraDir);
}

vec3 shadeBlinnPhong(LightParameters light_, vec3 materialAmbientColor, vec3 materialDiffuseColor,
                     vec3 materialSpecularColor, vec3 position, vec3 normal, vec3 toCameraDir) {
    vec3 toLightDir = normalize(light_.position - position);
    vec3 resAmb = shadeAmbient(light_, materialAmbientColor);
    vec3 resDiff = shadeDiffuseCalculation(light_, materialDiffuseColor, normal, toLightDir);
    vec3 resSpec = shadeSpecularBlinnPhongCalculation(light_, materialSpecularColor, normal,
                                                      toLightDir, toCameraDir);
    return resAmb + resDiff + resSpec;
}

vec3 shadePhong(LightParameters light_, vec3 materialAmbientColor, vec3 materialDiffuseColor,
                vec3 materialSpecularColor, vec3 position, vec3 normal, vec3 toCameraDir) {
    vec3 toLightDir = normalize(light_.position - position);
    vec3 resAmb = shadeAmbient(light_, materialAmbientColor);
    vec3 resDiff = shadeDiffuseCalculation(light_, materialDiffuseColor, normal, toLightDir);
    vec3 resSpec = shadeSpecularPhongCalculation(light_, materialSpecularColor, normal, toLightDir,
                                                 toCameraDir);
    return resAmb + resDiff + resSpec;
}

#endif
