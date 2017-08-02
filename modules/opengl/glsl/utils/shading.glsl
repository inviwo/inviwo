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

// Helper functions to calculate the shading
vec3 shadeDiffuseCalculation(LightParameters light_, vec3 materialDiffuseColor, vec3 normal,
                             vec3 position) {
							 vec3 col = vec3(0);
	for(int i = 0; i < light_.numLights; i++){
		vec3 toLightDir = normalize(light_.lights[i].position - position);

		vec4 att = light_.lights[i].attenuation;
		float attenuation = 1.0;
		if(att.w == 1.0) { 
			float dist = length(light_.lights[i].position - position);
			attenuation = 1.0 / (att.x + att.y * dist + att.z * (dist * dist));  
		}
		col += attenuation * materialDiffuseColor * light_.lights[i].diffuseColor * max(dot(normal, toLightDir), 0.0);
	}
	return col;
}

vec3 shadeSpecularBlinnPhongCalculation(LightParameters light_, vec3 materialSpecularColor,
                                        vec3 normal, vec3 position, vec3 toCameraDir) {
	vec3 col = vec3(0);
	for(int i = 0; i < light_.numLights; i++){
		vec3 toLightDir = normalize(light_.lights[i].position - position);
		vec4 att = light_.lights[i].attenuation;
		float attenuation = 1.0;
		if(att.w == 1.0) { 
			float dist = length(light_.lights[i].position - position);
			attenuation = 1.0 / (att.x + att.y * dist + att.z * (dist * dist));  
		}
		vec3 halfway = toCameraDir + toLightDir;

		// check for special case where the light source is exactly opposite
		// to the view direction, i.e. the length of the halfway vector is zero
		if (dot(halfway, halfway) < 1.0e-6) {  // check for squared length
			return vec3(0.0);
		} else {
			halfway = normalize(halfway);
			col+= attenuation * materialSpecularColor * light_.lights[i].specularColor *
				   pow(max(dot(normal, halfway), 0.0), light_.specularExponent);
		}
	}
	return col;
}

vec3 shadeSpecularPhongCalculation(LightParameters light_, vec3 materialSpecularColor, vec3 normal,
                                   vec3 position, vec3 toCameraDir) {
	vec3 col = vec3(0);
	for(int i = 0; i < light_.numLights; i++){
		vec3 toLightDir = normalize(light_.lights[i].position - position);
		vec4 att = light_.lights[i].attenuation;
		float attenuation = 1.0;
		if(att.w == 1.0) { 
			float dist = length(light_.lights[i].position - position);
			attenuation = 1.0 / (att.x + att.y * dist + att.z * (dist * dist));  
		}
		// Compute reflection (not that glsl uses incident direction)
		// Equivalent to: 2.0*dot(toLightDir, normal)*normal - toLightDir;
		vec3 r = reflect(-toLightDir, normal);
   
		if (dot(toLightDir, normal) < 0.0) {
			return vec3(0.0);
		} else {
			col += attenuation * materialSpecularColor * light_.lights[i].specularColor *
				pow(max(dot(r, toCameraDir), 0.0), light_.specularExponent * 0.25);
		}
	}
	return col;
}

// Functions to apply different shading modes.
// All positions and directions should be in world space!
vec3 shadeAmbient(LightParameters light_, vec3 materialAmbientColor, vec3 position) {
	vec3 ambient = vec3(0);
	for(int i = 0; i < light_.numLights; i++){
		vec4 att = light_.lights[i].attenuation;
		float attenuation = 1.0;
		if(att.w == 1.0) { 
			float dist = length(light_.lights[i].position - position);
			attenuation = 1.0 / (att.x + att.y * dist + att.z * (dist * dist));  
		}
		ambient += attenuation * light_.lights[i].ambientColor;
	}
    return materialAmbientColor * ambient;
}

vec3 shadeDiffuse(LightParameters light_, vec3 materialDiffuseColor, vec3 position, vec3 normal) {
    return shadeDiffuseCalculation(light_, materialDiffuseColor, normal, position);
}

vec3 shadeSpecular(LightParameters light_, vec3 materialSpecularColor, vec3 position, vec3 normal,
                   vec3 toCameraDir) {
    return shadeSpecularPhongCalculation(light_, materialSpecularColor, normal, position, toCameraDir);
}

vec3 shadeBlinnPhong(LightParameters light_, vec3 materialAmbientColor, vec3 materialDiffuseColor,
                     vec3 materialSpecularColor, vec3 position, vec3 normal, vec3 toCameraDir) {
    vec3 resAmb = shadeAmbient(light_, materialAmbientColor, position);
    vec3 resDiff = shadeDiffuseCalculation(light_, materialDiffuseColor, normal, position);
    vec3 resSpec = shadeSpecularBlinnPhongCalculation(light_, materialSpecularColor, normal, position, toCameraDir);
    return resAmb + resDiff + resSpec;
}

vec3 shadePhong(LightParameters light_, vec3 materialAmbientColor, vec3 materialDiffuseColor,
                vec3 materialSpecularColor, vec3 position, vec3 normal, vec3 toCameraDir) {
    vec3 resAmb = shadeAmbient(light_, materialAmbientColor, position);
    vec3 resDiff = shadeDiffuseCalculation(light_, materialDiffuseColor, normal, position);
    vec3 resSpec = shadeSpecularPhongCalculation(light_, materialSpecularColor, normal, position, toCameraDir);
    return resAmb + resDiff + resSpec;
}

vec3 shadeOrenNayarDiffuse(LightParameters light_, vec3 materialDiffuseColor, vec3 position, vec3 normal, vec3 toCameraDir) { // http://ruh.li/GraphicsOrenNayar.html
	vec3 col = vec3(0);
	float NdotV = dot(normal, toCameraDir);
	float angleVN = acos(NdotV);

	float roughnessSquared = light_.roughness * light_.roughness;

	float A = 1.0 - 0.5 * (roughnessSquared / (roughnessSquared + 0.57));
	float B = 0.45 * (roughnessSquared / (roughnessSquared + 0.09));

	for(int i = 0; i < light_.numLights; i++){
		vec3 lightDirection = normalize(light_.lights[i].position - position);
		vec4 att = light_.lights[i].attenuation;
		float attenuation = 1.0;
		if(att.w == 1.0) { 
			float dist = length(light_.lights[i].position - position);
			attenuation = 1.0 / (att.x + att.y * dist + att.z * (dist * dist));  
		}

		float NdotL = dot(normal, lightDirection);

		float angleLN = acos(NdotL);

		float alpha = max(angleVN, angleLN);
		float beta = min(angleVN, angleLN);
		float gamma = dot(toCameraDir - normal * dot(toCameraDir, normal), lightDirection - normal * dot(lightDirection, normal));

		float C = sin(alpha) * tan(beta);

		// put it all together
		float result = max(0.0, NdotL) * (A + B * max(0.0, gamma) * C);
		col += attenuation * light_.lights[i].diffuseColor * materialDiffuseColor * result;
	}
	return col;
}

vec3 shadeOrenNayar(LightParameters light_, vec3 materialAmbientColor, vec3 materialDiffuseColor, vec3 position, vec3 normal, vec3 toCameraDir) {
	vec3 diffuse = shadeOrenNayarDiffuse(light_, materialDiffuseColor, position, normal, toCameraDir);
	vec3 ambient = shadeAmbient(light_, materialAmbientColor, position);
	return ambient + diffuse;
}

#endif
