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

#ifndef COOK_TORRANCE_BRDF_CL
#define COOK_TORRANCE_BRDF_CL

#include "shading/shadingmath.cl"
#include "shading/microfacet.cl"

float CookTorranceBRDF(const float3 wo, const float3 wi, const float f0, const float m) {
    if(!sameHemisphere(wo, wi)) {
        return 0.f;
    }
    Microfacet distribution;
    float3 wh = normalize(wo+wi);
    float NdotWh = absCosTheta(wh);
    distribution.D = BeckmannDistribution(NdotWh, m);
    //distribution.D = BlinnPhongDistribution(NdotWh, m);
    distribution.G = CookTorranceGeometry(wo, wi, wh);
    distribution.F = fresnelSchlick(NdotWh, f0); 
    return microfacetBRDF(wo, wi, wh, &distribution);
}
float3 CookTorranceSample(const float3 wo, float m, const float2 rnd) {
    return BeckmannSample(wo, m, rnd);    
}
float CookTorrancePdf(const float3 wo, const float3 wi, const float3 N, float m) {
    if (sameHemisphereN(wo, wi, N)) {
        return BeckmannPdf(wo, wi, m);
    } else {
        return 0.f;
    }
}

#endif
