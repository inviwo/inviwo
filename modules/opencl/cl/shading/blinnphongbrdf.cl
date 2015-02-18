/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef BLINN_PHONG_BRDF_CL
#define BLINN_PHONG_BRDF_CL

#include "shading/shadingmath.cl"
#include "shading/microfacet.cl"

float BlinnBRDF(const float3 wo, const float3 wi, const float f0, const float exponent) {
    Microfacet distribution;
    float3 wh = normalize(wo+wi);
    float cosThetaH = absCosTheta(wh);
    distribution.D = BlinnPhongDistribution(cosThetaH, exponent);
    distribution.G = CookTorranceGeometry(wo, wi, wh);
    distribution.F = fresnelSchlick(cosThetaH, f0); 
    return microfacetBRDF(wo, wi, wh, &distribution);
}

float BlinnPdf(const float3 wo, const float3 wi, const float exponent) {
    if ( !sameHemisphere(wo, wi) ) return 0.f; 

    float3 H = normalize(wo+wi);
    float dotWoH = dot(wo, H);
    //float costheta = fabs(dot(H, N));
    float cosThetaH = absCosTheta(H);
    float pdf = ((exponent + 1.f) * native_powr(cosThetaH, exponent)) / (2.f * M_PI * 4.f * dotWoH);
    //return fabs(pdf);
    return dotWoH > 0.f ? pdf : 0.f; 
}  

float3 BlinnSample(const float3 wo, const float3 N, float exponent, const float2 rnd) {
    float costheta = native_powr(rnd.x, 1.f/(exponent+1.f));
    float sintheta = native_sqrt(max(0.f, 1.f-costheta*costheta));
    float phi = rnd.y * 2.f * M_PI;
    float3 wh = sphericalDirection(sintheta, costheta, phi);
    if (!sameHemisphereN(wo, wh, N)) wh = -wh;
    
    return -wo + 2.f*dot(wo,wh)*wh;

}



#endif
