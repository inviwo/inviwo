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

#ifndef SHADING_MATH_CL
#define SHADING_MATH_CL
// Some systems does not have these defined
#ifndef M_PI
#define M_PI 3.14159265359f
#endif
#ifndef M_1_PI
#define M_1_PI 0.31830988618f
#endif

// Transform coordinate to local shading space
inline float3 worldToShading(const float3 Nu, const float3 Nv, const float3 Nn, const float3 W)
{
	return (float3)(dot(W, Nu), dot(W, Nv), dot(W, Nn));
}
// Transform coordinate from local shading space
inline float3 shadingToWorld(const float3 Nu, const float3 Nv, const float3 Nn, const float3 W)
{
    return Nu * W.x + Nv * W.y + Nn * W.z;
	//return (float3)(Nu.x * W.x + Nv.x * W.y + Nn.x * W.z,
	//				Nu.y * W.x + Nv.y * W.y + Nn.y * W.z,
	//				Nu.z * W.x + Nv.z * W.y + Nn.z * W.z);
}
// Create Ortho-Normal basis
//inline float3 createONBasis(float3 *Nu, float3 *Nv, float3 *Nn, const float3 gradient, const float3 direction)
//    *Nu = cross(gradient, direction);
//    if ( dot(*Nu, *Nu) < 1.e-3f ) {
//        Nu = cross( gradient, (float3)( 1.0f, 0.0f, 0.0f ) ); 
//        //if ( dot(Nu, Nu) < 1.e-3f ) {
//        //    Nu = normalize(cross( gradient, (float3)( 0.0f, 1.0f, 0.0f ) ));
//        //}
//        //if ( dot(Nu, Nu) < 1.e-3f ) {
//        //    Nu = normalize(cross( gradient, (float3)( 0.0f, 0.0f, 1.0f ) ));
//        //}
//    }
//    Nu = normalize(Nu);
//    float3 Nv = normalize(cross(gradient, Nu));
//
//}

void createCoordinateSystem(const float3 v1, float3 *v2, float3 *v3) {
    if (fabs(v1.x) > fabs(v1.y)) {
        float invLen = 1.f / sqrt(v1.x*v1.x + v1.z*v1.z);
        *v2 = (float3)(-v1.z * invLen, 0.f, v1.x * invLen);
    }
    else {
        float invLen = 1.f / sqrt(v1.y*v1.y + v1.z*v1.z);
        *v2 = (float3)(0.f, v1.z * invLen, -v1.y * invLen);
    }
    *v3 = cross(v1, *v2);
}
inline float absCosTheta(const float3 v) {
    return fabs(v.z);
}
inline float cosTheta(const float3 w) { return w.z; }

inline float sinTheta2(const float3 w) {
    return max(0.f, 1.f - cosTheta(w)*cosTheta(w)); 
} 

inline float sinTheta(const float3 w) {
    return native_sqrt(sinTheta2(w));
}

float cosPhi(const float3 w) {
    float sintheta = sinTheta(w);
    if (sintheta == 0.f) return 1.f;
    return clamp(w.x / sintheta, -1.f, 1.f); 
} 

inline float sinPhi(const float3 w) {
    float sintheta = sinTheta(w);
    if (sintheta == 0.f) return 0.f;
    return clamp(w.y / sintheta, -1.f, 1.f); 
}
// N Normal vector
// I Incident vector
float3 faceForward( const float3 N, const float3 I )
{
  return dot(N, I) < 0 ? N : -N;
}

// I Incident vector
// N Normal vector
// Returns the reflection direction calculated as I - 2.0 * dot(N, I) * N
float3 reflect( float3 I, float3 N) {
    return I - 2.f*dot(N, I)*N;
}


bool sameHemisphere(const float3 wo, const float3 wi) {
    return wo.z*wi.z > 0.f;
}
inline bool sameHemisphereN(const float3 wo, const float3 wi, const float3 N) {
   return dot(wo, N) >= 0.0f && dot(wi, N) >= 0.0f;
}
// Given two uniform random numbers, compute a random direction on the sphere
float3 sphericalToCartesian(float r, float theta, float phi) {
    float3 result;
    float2 sinAngle = native_sin((float2)(theta,phi));
    float2 cosAngle = native_cos((float2)(theta,phi));
    result.x = r*sinAngle.x*cosAngle.y;
    result.y = r*sinAngle.x*sinAngle.y;
    result.z = r*cosAngle.x;
    return result;
}
inline float3 sphericalDirection(float sintheta, float costheta, float phi) {
    return (float3)(sintheta * native_cos(phi), sintheta * native_sin(phi), costheta);
}
inline float3 sphericalDirectionCoordinateSystem(float sintheta, float costheta, float phi, const float3 x, const float3 y, const float3 z) {
    return sintheta * native_cos(phi) * x + sintheta * native_sin(phi) * y + costheta * z;
}
// Given two uniform random numbers, compute a random direction on the sphere
float3 uniformSampleSphere(const float2 u) {
    //float3 result;
    //result.z = 1.f-2.f*u.x;
    //float r = native_sqrt(max(0.f, 1.f-result.z*result.z));
    //float phi = 2.f*M_PI*u.y;
    //result.x = r*cos(phi);
    //result.y = r*sin(phi);
    //return result;
    float z = 1.f-2.f*u.x;
    float r = native_sqrt(max(0.f, 1.f-z*z));
    float phi = 2.f*M_PI*u.y;
    float x = r*native_cos(phi);
    float y = r*native_sin(phi);
    return (float3)(x,y,z);
}

float uniformSpherePdf() {
    //return 0.25f*M_1_PI;
    return 1.f/(4.f*M_PI);
}



float3 uniformSampleCone(const float2 rnd, float cosThetaMax) {
    float costheta = mix(cosThetaMax, 1.f, rnd.x);
    //float costheta = (1.f-rnd.x) + rnd.x*1.f;
    float sintheta = native_sqrt(1.f - costheta*costheta);
    float phi = rnd.y*2.f*M_PI;
    return (float3)(native_cos(phi)*sintheta, native_sin(phi)*sintheta, costheta);
}

float uniformConePdf(float cosThetaMax) {
    return 1.f / ( 2.f*M_PI*( 1.f - cosThetaMax ) );
}

inline float lambertianBRDF() {
 return M_1_PI;
}

// Evaluate power heuristic with Beta=2
// nSamplesFromEachPdf = (n_s, n_g)
// pdf = (pdf of s, pdf of g) = (p_s, p_g)
//
//  w = (n_s*p_s)^Beta / ( Sum_i(n_i*p_i)^Beta
//  With Beta=2
//  w = (n_s*p_s)^2 / ( (n_s*p_s)^2 + (n_g*p_g)^2) 
//
float powerHeuristic(float2 nSamplesFromEachPdf, float2 pdf) {
    float2 f = nSamplesFromEachPdf*pdf;
    float2 f2 = f*f;    
    return f2.x/(f2.x+f2.y);
}

float balanceHeuristic(float2 nSamplesFromEachPdf, float2 pdf) {
    return nSamplesFromEachPdf.x*pdf.x/(dot(nSamplesFromEachPdf, pdf));
}



#endif
