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

#ifndef PHASE_FUNCTIONS_CL
#define PHASE_FUNCTIONS_CL

#include "shading/shadingmath.cl"

// Isotropic phase functions, equal in all directions
float isotropicPhaseFunction() {
    //return 0.25f*M_1_PI;
    return 1.f/(4.f*M_PI);
}
// Henyey-Greenstein phase function 
// @param g Angle between incoming ray and scattered ray, g = 0 is forward direction and g = pi is backward direction
// @return Distribution of scattered light
float HenyeyGreensteinPhaseFunction(const float3 wo, const float3 wi, const float g) {
    float costheta = dot(wo, wi);
    return (1.f/(4.f*M_PI)) * (1.f-g*g) / pow( 1.f + g*g - 2.f*g*costheta, 1.5f);
    //return (0.25f*M_1_PI) * (1.f-g*g) / pow( 1.f + g*g - 2.f*g*costheta, 1.5f);
}
// Henyey-Greenstein phase function importance sample
float3 HenyeyGreensteinSample(const float3 wi, const float g, const float2 rnd) {
    // Dividing by zero may produce NaN, we need to check for this...
    // http://www.astro.umd.edu/~jph/HG_note.pdf
    float cosTheta;
    if ( fabs(g) < 1e-5 ) {
        cosTheta = 2.f*rnd.x-1.f;
    } else {
        float tmp = (1.f-g*g) / (1.f-g+2*g*rnd.x);
        cosTheta = ( 1.f + g*g - tmp*tmp )/(2.f*g);
    }
    float sinTheta = sqrt(max(0.f, 1.f-cosTheta*cosTheta));
    float phi = 2.f*M_PI*rnd.y;
    float3 v1, v2;
    createCoordinateSystem(wi, &v1, &v2);
    return sphericalDirectionCoordinateSystem(sinTheta, cosTheta, phi, v1, v2, wi);
}

// Schlick phase function. Approximation of Henyey-Greenstein phase function, faster to compute. 
// @param k Angle between incoming ray and scattered ray, k = 0 is forward direction and k = pi is backward direction
// @return Distribution of scattered light
float SchlickPhaseFunction(const float3 wo, const float3 wi, float k) {
    k = clamp(k, -0.99f, 0.99f);
    float tmp = (1.f-k*dot(wo, wi));
    return (1.f/(4.f*M_PI)) * (1.f-k*k) / ( tmp*tmp );
    //float costheta = dot(wo, wi);
    //return (1.f/(4.f*M_PI)) * (1.f-k*k) / ( pown((1.f+k*costheta), 2) );
}
// Schlick phase function importance sample
float3 SchlickSample(const float3 wi, float k, const float2 rnd) {
    k = clamp(k, -0.99f, 0.99f);
    float cosTheta = ((2.f*rnd.x+k-1.f) / (2.f*k*rnd.x-k+1.f));
    float sinTheta = sqrt(max(0.f, 1.f-cosTheta*cosTheta));
    float phi = 2.f*M_PI*rnd.y;
    float3 v1, v2;
    createCoordinateSystem(wi, &v1, &v2);
    return sphericalDirectionCoordinateSystem(sinTheta, cosTheta, phi, v1, v2, wi);
}

float RayleighPhaseFunction(const float3 wo, const float3 wi) {
    float costheta = dot(wo, wi);
    return (3.f/(M_PI*16.f))*(1.f+costheta*costheta);
}
// Rayleigh phase function importance sample
// From: Importance sampling the Rayleigh phase function
float3 RayleighSample(const float3 wi, const float2 rnd, float *theta) {
    float tmp = (2.f*rnd.x-1.f);
    float u = -pow(2.f*tmp+sqrt(4.f*tmp*tmp+1.f), 1.f/3.f);
    (*theta) = acos(u-1.f/u);

    float3 dir = sphericalToCartesian(1.f, *theta, 2.f*M_PI*rnd.y);
    dir += wi;
    return dir;
}




float phaseFunction(float theta) {

    return isotropicPhaseFunction();
}


#endif
