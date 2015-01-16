/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef LIGHT_CL
#define LIGHT_CL

#include "datastructures/lightsource.cl"
#include "datastructures/bbox.cl"
#include "shading/shadingmath.cl"
#include "transformations.cl"



int getLightSourceId(const int2 nPhotonsPerLight, const int nLightSources) {
    return (get_global_id(1)*get_global_size(0)+get_global_id(0))/nPhotonsPerLight.y;
}



float2 getLightSourceUniformUV(const int2 nPhotonsPerLight, const int nLightSources) {
    int globalId = get_global_id(1)*get_global_size(0)+get_global_id(0);
    int lightSourceId =  getLightSourceId(nPhotonsPerLight, nLightSources);
 
    int lightSourceOffset = lightSourceId*nPhotonsPerLight.y;


    int lightSampleIndex = globalId-lightSourceOffset;
    int2 photonId = (int2)(lightSampleIndex%nPhotonsPerLight.x, lightSampleIndex/nPhotonsPerLight.x);

    return (float2)((convert_float2(photonId)+0.5f)/convert_float(nPhotonsPerLight.x));
}
float2 getLightSourceStratifiedUV(const int2 nPhotonsPerLight, const int nLightSources, const float2 randNum) {
    int globalId = get_global_id(1)*get_global_size(0)+get_global_id(0);

    int lightSourceId =  getLightSourceId(nPhotonsPerLight, nLightSources);

    int lightSourceOffset = lightSourceId*nPhotonsPerLight.y;


    int lightSampleIndex = globalId-lightSourceOffset;
    // Calculate integer xy coordinate on light
    int2 photonId = (int2)(lightSampleIndex%nPhotonsPerLight.x, lightSampleIndex/nPhotonsPerLight.x);

    return (float2)((convert_float2(photonId)+randNum)/convert_float(nPhotonsPerLight.x));
}

void sampleLight(LightSource lightSource, const float2 uv, float3* __restrict origin, float3* __restrict wi, float3* __restrict power, float* __restrict pdf, float3 rndNum, const BBox bbox) {
  
    if( lightSource.type == LIGHT_POINT ) {
        float3 localOrigin = (float3)(0.f);
        *origin = transformPoint(lightSource.tm, localOrigin);
        float3 sphereSample = uniformSampleSphere(uv);
        *wi = -sphereSample;

        *pdf = uniformSpherePdf();
        *power = lightSource.radiance / *pdf;
    } 
    else if (lightSource.type == LIGHT_AREA ) {
        float3 localOrigin = (float3)(lightSource.size*(-0.5f+uv), 0.f);

        *origin = transformPoint(lightSource.tm, localOrigin);
        // Area light
        float3 pointOnPlane = bbox.pMin+ (bbox.pMax-bbox.pMin)*(float3)(rndNum.x, rndNum.y, rndNum.z);
        *wi = normalize(pointOnPlane-*origin);
        *pdf = lightSource.area; 

        *power = lightSource.radiance / *pdf;
    } 
    else if (lightSource.type == LIGHT_DIRECTIONAL) {

            float3 localOrigin = (float3)(lightSource.size*(-0.5f+uv), 0.f); 
            
            *origin = transformPoint(lightSource.tm, localOrigin);
            
            // Directional light
            float3 pointOnPlane = transformPoint(lightSource.tm, (float3)(localOrigin.xy, 1.0f));
            *wi = normalize(pointOnPlane-*origin);
            *pdf = lightSource.area; 
            *power = lightSource.radiance;

    } else { // if (lightSource.type == LIGHT_CONE ) 
        float3 localOrigin = (float3)(0.0f, 0.f, 0.f);
        *origin = transformPoint(lightSource.tm, localOrigin);

        float3 localDir = uniformSampleCone(uv, lightSource.cosFOV);
        float3 pointOnPlane = transformPoint(lightSource.tm, localDir);
        *wi = normalize(pointOnPlane-*origin);
        *pdf = uniformConePdf(lightSource.cosFOV); 
        
        *power = localDir.z*localDir.z*localDir.z*localDir.z*lightSource.radiance*localDir.z/(*pdf);
    }  

 
}

void sampleLights(__global LightSource const * __restrict lightSources, const int2 nPhotonsPerLight,
                  const int nLightSources, const float2 uv, float3* __restrict origin, float3* __restrict wi, float3* __restrict power, float* __restrict pdf, float3 rndNum, const BBox bbox) {
 
    uint globalId = get_global_id(1)*get_global_size(0)+get_global_id(0);
    int lightSourceId = getLightSourceId(nPhotonsPerLight, nLightSources);

    sampleLight(lightSources[lightSourceId], uv, origin, wi, power, pdf, rndNum, bbox);

 
}
  

#endif // LIGHT_CL
