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

#ifndef TRANSFORMATIONS_CL
#define TRANSFORMATIONS_CL
// Column major access (used by glm)
// 00 04 08 12
// 01 05 09 13
// 02 06 10 14
// 03 07 11 15
// in opencl:
// 00 04 08  c
// 01 05 09  d
// 02 06  a  e
// 03 07  b  f


// Assume that the fourth component in x is 1
inline float3 transformPoint(const float16 m, const float3 x) {
     return (float3)(dot(m.s048, x)+m.sc, dot(m.s159, x)+m.sd, dot(m.s26a, x)+m.se);
}
inline float4 transformPoint4(const float16 m, const float4 x) {
     return (float4)( dot(m.s048c, x), dot(m.s159d, x), dot(m.s26ae, x), dot(m.s37bf, x) );
}
// Transform and project
inline float3 transformPointW(const float16 m, const float3 x) {
    return transformPoint(m, x)/(dot(m.s37b, x)+m.sf);
}
// Transform and project
inline float4 transformPoint4W(const float16 m, const float4 x) {
    float4 point = transformPoint4(m, x);
    point /= point.w;
    return point;
}
// Transform a direction (does not apply translation)
inline float3 transformVector(const float16 m, const float3 x) {
    return (float3)(dot(m.s048, x), dot(m.s159, x), dot(m.s26a, x));
}

inline float3 translatePoint(const float16 m, const float3 x) {
     return x+m.scde;
}


// Convert from [0 1] space to data space [1/(2*dim) 1-1/(2*dim)]
float3 toDataSpace3D(float3 p, float3 invDim) {
    return 0.5f*invDim + p*(1.f-invDim);
}
float2 toDataSpace2D(float2 p, float2 invDim) {
    return 0.5f*invDim + p*(1.f-invDim);
}
float toDataSpace1D(float p, float invDim) {
    return 0.5f*invDim + p*(1.f-invDim);
}


// Row-major access
//// Assume that the fourth component in x is 1
//float3 transformPoint(const float16 m, const float3 x)
//{
//     return (float3)(dot(m.s012, x)+m.s3, dot(m.s456, x)+m.s7, dot(m.s89a, x)+m.sb);
//}
//float4 transformPoint4(const float16 m, const float4 x)
//{
//     return (float4)(dot(m.s0123, x), dot(m.s4567, x), dot(m.s89ab, x), dot(m.scdef, x));
//}
//// Transform and project
//float3 transformPointW(const float16 m, const float3 x)
//{
//    return (float3)(dot(m.s012, x)+m.s3, dot(m.s456, x)+m.s7, dot(m.s89a, x)+m.sb)/(dot(m.scde, x)+m.sf);
//}
//// Transform and project
//float4 transformPoint4W(const float16 m, const float4 x)
//{
//    float4 point = (float4)(dot(m.s0123, x), dot(m.s4567, x), dot(m.s89ab, x), dot(m.scdef, x));
//    point /= point.w;
//    return point;
//}
//// Transform a direction (does not apply translation)
//float3 transformVector(const float16 m, const float3 x)
//{
//    return (float3)(dot(m.s012, x), dot(m.s456, x), dot(m.s89a, x));
//}
//
//float3 translatePoint(const float16 m, const float3 x)
//{
//     return x+m.s37b;
//}


// Encode xyz direction into theta, phi angles
// Assumes normalized direction, i.e. r = 1
float2 encodeDirection(float3 dir) {
    float phi = atan2(dir.y, dir.x);
    //if ( !isfinite(phi) ) {
        //if(dir.y < 0.f) phi = -0.5f*M_PI;
        //else            phi = 0.5f*M_PI;
    //}
    // Important: clamp dir.z to avoid NaN
    float theta = acos(clamp(dir.z,-1.f, 1.f));

    return (float2)(theta, phi);
}

// Decode theta, phi angles to xyz direction 
float3 decodeDirection(float2 angles) {
    // Evaluated three different ways of computing this.
    // The first one is by far the fastest.
    float2 cosAngles = native_cos(angles);
    float2 sinAngles = native_sin(angles);
    return (float3)(sinAngles.x*cosAngles.y,
                sinAngles.x*sinAngles.y,
                cosAngles.x);
    // Option 2 (slightly faster than option 3):
    //float2 cosAngles;
    //// Calculate both sine and cosine at once
    //float2 sinAngles = sincos(angles, &cosAngles);
    // Option 3:
    //return (float3)(native_sin(angles.x)*native_cos(angles.y),
    //                native_sin(angles.x)*native_sin(angles.y),
    //                native_cos(angles.x));
}


#endif // TRANSFORMATIONS_CL