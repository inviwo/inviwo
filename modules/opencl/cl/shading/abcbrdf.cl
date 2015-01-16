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

#ifndef ABC_BRDF_CL
#define ABC_BRDF_CL

#include "shading/shadingmath.cl"
#include "shading/microfacet.cl"

// BRDF: ABC-model, http://vcl.itn.liu.se/publications/2011/LKYU11/main.pdf
// NOTE: This code has not been tested properly

float abcModel(const float B, const float C, const float f2) {
    return 1.f/pow( (1.f-B*f2), C);
}  

// A term is the specular component which is dealt with outside of this function
float ABCMicrofacetBRDF(const float3 wo, const float3 wi, const float f0, const float B, const float C) {
    //float G = 1.f;
    // Project vectors to tangent plane of normal
    //float3 Vp = // V - dot(V, N)*N;
    //float3 Lp = L - dot(L, N)*N;
    //float S = Lp-Vp;
    //float d2 = dot(S, S);//S.x*S.x+S.y*S.y+S.z*S.z;
    ////float cosThetaH = sqrt(1.f-d2/4.f);
    //float cosThetaH = absCosTheta(wh);
    //float Q = fresnel(cosThetaH, 0.f);

    //float3 H = wo+wi; // V + L;
    //float3 Dp = H-dot(N, H)*N;
    //return kd/M_PI+G*Q*abcModel( A,B,C,dot(Dp, Dp) ); 

    Microfacet distribution;
    float3 wh = normalize(wo+wi);
    float NdotWh = absCosTheta(wh);
    distribution.D = abcModel(B, C, 1.f-NdotWh); 
    distribution.G = CookTorranceGeometry(wo, wi, wh);
    distribution.F = 1.f;
    return microfacetBRDF(wo, wi, wh, &distribution);
}

float3 ABCMicrofacetSample(const float3 wo, const float B, const float C, const float2 rnd) {
    float theta;
    if ( C == 1.f ) {
        theta = acos ( 1.f+B-exp(rnd.x*native_log(1.f+B)) );
    } else {
        theta = acos ( ( 1.f+B-pow((1.f + rnd.x*pow((1.f+B), 1.f-C)-1.f), -1.f/(C-1.f)) ) / B );
    }
    float costheta = cos (theta);
    float sintheta = native_sqrt(max(0.f, 1.f - costheta*costheta));
    float phi = rnd.y * 2.f * M_PI;

    float3 H = sphericalDirection(sintheta, costheta, phi);

    if (!sameHemisphere(wo, H))
        H.z *= -1.f;

    // Compute incident direction by reflecting about $\wh$
    return -wo + 2.f * dot(wo, H) * H;
}



#endif
