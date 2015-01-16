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

#ifndef ASHIKIMIM_BRDF_CL
#define ASHIKIMIM_BRDF_CL

#include "shading/shadingmath.cl"
#include "shading/microfacet.cl"

float AshikimBRDF(const float3 wo, const float3 wi, const float3 N, const float f0, const float exponent) {
    //float cosThetaWo = fabs(dot(wo, N));
    //float cosThetaWi = fabs(dot(wi, N));
    float cosThetaWo = absCosTheta(wo);
    float cosThetaWi = absCosTheta(wi);

    if ( cosThetaWo*cosThetaWi < 1e-4f ) 
        return (float)(0.f);

    if ( cosThetaWo == 0.f || cosThetaWi == 0.f ) 
        return 0.f;

    float3 wh = wo+wi;
    //if ( all(wh < 1e-5f) ) 
    //if ( dot(wh,wh) < 1e-3f ) 
    //    return (float)(0.f);
    // Using fast_normalize creates errors on Intel devices
    wh = normalize(wh);

    float NdotWh = absCosTheta(wh);
    // Fresnel term
    //float f0 = pown((1-etai)/(1+etat), 2);
    //float f0 = pown((1.f-n)/(1.f+n), 2);
    float F = fresnelSchlick(NdotWh, f0); 
    //float F = fresnelDielectric(etai, etat, NdotWh);
    //float NdotWh = fabs(dot(wh, N));
    float WodotWh = fabs(dot(wh, wo));
    // Distribution term
    float D = BlinnPhongDistribution(NdotWh, exponent);
    // Geometry term
    float G = CookTorranceGeometry(wo, wi, wh); //minf(1.f, minf( 2.f*NdotWh*cosThetaWo/WodotWh, 2.f*NdotWh*cosThetaWi/WodotWh ) );
    // Add a lambertian factor (1/pi)
    //return D * G * F / (4.0f * cosThetaWi * cosThetaWo);
    //return D * G * F / (4.0f * cosThetaWi * cosThetaWo);
    float denom = max(1e-4f, 4.0f * cosThetaWi * cosThetaWo);
    return native_divide(D * G * F , denom);
    
}

float3 AshikhminSample(const float3 wi, const float3 N, float exponent, const float2 rnd) {
    return BlinnSample(wi, N, exponent, rnd);
}



float AshikhminPdf(const float3 wo, const float3 wi, const float exponent) {
    return BlinnPdf(wo, wi, exponent);  
}


#endif
