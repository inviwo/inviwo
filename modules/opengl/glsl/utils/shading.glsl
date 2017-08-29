/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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


float computeLightAttenuation(Light light, vec3 position){
#ifdef LIGHT_ATTENUATION
    float dist = length(light.position - position);
    return 1.0 / (light.attenuation.x + light.attenuation.y * dist +
                                 light.attenuation.z * (dist * dist));
#else
    return 1.0;
#endif
}



// Helper functions to calculate the shading
vec3 shadeDiffuseCalculation(Light light, vec3 materialDiffuseColor,
                             vec3 normal, vec3 position, float attenuation) {
    vec3 toLightDir = normalize(light.position - position);

    return attenuation * materialDiffuseColor * light.diffuseColor *
               max(dot(normal, toLightDir), 0.0);
}

vec3 shadeSpecularBlinnPhongCalculation(Light light,
                                        vec3 materialSpecularColor, vec3 normal, vec3 position,
                                        vec3 toCameraDir, float attenuation, float specularExponent) {
    vec3 toLightDir = normalize(light.position - position);
    vec3 halfway = toCameraDir + toLightDir;

    // check for special case where the light source is exactly opposite
    // to the view direction, i.e. the length of the halfway vector is zero
    if (dot(halfway, halfway) < 1.0e-6) {  // check for squared length
        return vec3(0.0);
    } else {
        halfway = normalize(halfway);
        return attenuation * materialSpecularColor * light.specularColor *
            pow(max(dot(normal, halfway), 0.0), specularExponent);
    }
}

vec3 shadeSpecularPhongCalculation(Light light, vec3 materialSpecularColor,
                                   vec3 normal, vec3 position, vec3 toCameraDir, 
                                   float attenuation, float specularExponent) {
    vec3 toLightDir = normalize(light.position - position);
    // Compute reflection (not that glsl uses incident direction)
    // Equivalent to: 2.0*dot(toLightDir, normal)*normal - toLightDir;
    vec3 r = reflect(-toLightDir, normal);

    if (dot(toLightDir, normal) < 0.0) {
        return vec3(0.0);
    } else {
        return attenuation * materialSpecularColor * light.specularColor *
               pow(max(dot(r, toCameraDir), 0.0), specularExponent * 0.25);
    }
}

// Functions to apply different shading modes.
// All positions and directions should be in world space!
vec3 shadeAmbient(LightParameters lightParameters_, vec3 materialAmbientColor, vec3 position) {
    vec3 ambient = vec3(0);
    for (int i = 0; i < NUMBER_OF_LIGHTS; i++) {
        ambient += lightParameters_.lights[i].ambientColor;
    }
    return materialAmbientColor * ambient;
}

vec3 shadeDiffuse(LightParameters lightParameters_, vec3 materialDiffuseColor, vec3 position,
                  vec3 normal) {
    vec3 color = vec3(0);
    for(int i = 0; i < NUMBER_OF_LIGHTS; i++) {
        Light light = lightParameters_.lights[i];
        float attenuation = computeLightAttenuation(light, position);
        color += shadeDiffuseCalculation(light, materialDiffuseColor, normal, position, attenuation);
    }
    return color;
}

vec3 shadeSpecular(LightParameters lightParameters_, vec3 materialSpecularColor, vec3 position,
                   vec3 normal, vec3 toCameraDir) {
    vec3 color = vec3(0);
    for(int i = 0; i < NUMBER_OF_LIGHTS; i++) {
        Light light = lightParameters_.lights[i];
        float attenuation = computeLightAttenuation(light, position);
        color += shadeSpecularPhongCalculation(light, materialSpecularColor, normal, position,
                                         toCameraDir, attenuation, lightParameters_.specularExponent);
    }
    return color;
}

vec3 shadeBlinnPhong(LightParameters lightParameters_, vec3 materialAmbientColor,
                     vec3 materialDiffuseColor, vec3 materialSpecularColor, vec3 position,
                     vec3 normal, vec3 toCameraDir) {
    vec3 resAmb = shadeAmbient(lightParameters_, materialAmbientColor, position);

    vec3 resDiff = vec3(0);
    vec3 resSpec = vec3(0);
    for(int i = 0; i < NUMBER_OF_LIGHTS; i++) {
        Light light = lightParameters_.lights[i];
        float attenuation = computeLightAttenuation(light, position);
        resDiff += shadeDiffuseCalculation(light, materialDiffuseColor, normal, position, attenuation);
        resSpec += shadeSpecularBlinnPhongCalculation(light, materialSpecularColor, normal, position, 
                                        toCameraDir, attenuation, lightParameters_.specularExponent);
    }
    return resAmb + resDiff + resSpec;
}

vec3 shadePhong(LightParameters lightParameters_, vec3 materialAmbientColor,
                vec3 materialDiffuseColor, vec3 materialSpecularColor, vec3 position, vec3 normal,
                vec3 toCameraDir) {
    vec3 resAmb = shadeAmbient(lightParameters_, materialAmbientColor, position);

    vec3 resDiff = vec3(0);
    vec3 resSpec = vec3(0);
    for(int i = 0; i < NUMBER_OF_LIGHTS; i++) {
        Light light = lightParameters_.lights[i];
        float attenuation = computeLightAttenuation(light, position);
        resDiff += shadeDiffuseCalculation(light, materialDiffuseColor, normal, position, attenuation);
        resSpec += shadeSpecularPhongCalculation(light, materialSpecularColor, normal, position, 
                                        toCameraDir, attenuation, lightParameters_.specularExponent);
    }
    return resAmb + resDiff + resSpec;
}

// http://ruh.li/GraphicsOrenNayar.html
vec3 shadeOrenNayarDiffuseCalculation(Light light, vec3 materialDiffuseColor,
                           vec3 position, vec3 normal, vec3 toCameraDir, 
                           float attenuation, float A, float B, float NdotV, float angleVN) {  
        vec3 lightDirection = normalize(light.position - position);

        float NdotL = dot(normal, lightDirection);

        float angleLN = acos(NdotL);

        float alpha = max(angleVN, angleLN);
        float beta = min(angleVN, angleLN);
        float gamma = dot(toCameraDir - normal * dot(toCameraDir, normal),
                          lightDirection - normal * dot(lightDirection, normal));

        float C = sin(alpha) * tan(beta);

        // put it all together
        float result = max(0.0, NdotL) * (A + B * max(0.0, gamma) * C);
        return attenuation * light.diffuseColor * materialDiffuseColor * result;
}

vec3 shadeOrenNayarDiffuse(LightParameters lightParameters_, vec3 materialDiffuseColor, 
                                        vec3 position, vec3 normal, vec3 toCameraDir) {
    float roughnessSquared = lightParameters_.roughness * lightParameters_.roughness;

    float A = 1.0 - 0.5 * (roughnessSquared / (roughnessSquared + 0.57));
    float B = 0.45 * (roughnessSquared / (roughnessSquared + 0.09));

    float NdotV = dot(normal, toCameraDir);
    float angleVN = acos(NdotV);

    vec3 color = vec3(0);

    for(int i = 0; i < NUMBER_OF_LIGHTS; i++) {
        Light light = lightParameters_.lights[i];
        float attenuation = computeLightAttenuation(light, position);
        color += shadeOrenNayarDiffuseCalculation(light, materialDiffuseColor, position, 
                                normal, toCameraDir, attenuation, A, B, NdotV, angleVN);
    }
    return color;
}

vec3 shadeOrenNayar(LightParameters lightParameters_, vec3 materialAmbientColor,
                    vec3 materialDiffuseColor, vec3 position, vec3 normal, vec3 toCameraDir) {
    vec3 diffuse = shadeOrenNayarDiffuse(lightParameters_, materialDiffuseColor, position, normal,
                                         toCameraDir);
    vec3 ambient = shadeAmbient(lightParameters_, materialAmbientColor, position);
    return ambient + diffuse;
}

#endif
