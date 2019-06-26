/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#ifndef IVW_RAYCASTGEOMETRY_GLSL
#define IVW_RAYCASTGEOMETRY_GLSL

vec4 drawPlanes(in vec4 oldres, in vec3 pos, in vec3 dir, in float inc, PlaneParameters plane, in float t, inout float tDepth) {
    vec4 result = oldres;
    
    float e = dot(pos-plane.position, plane.normal);
    float step = abs(dot(plane.normal, inc*dir));
    if ( e <= 0.0 && e > -step ) {
        if (tDepth == -1.0 && oldres.a > 0.0) tDepth = t;
        result.rgb = result.rgb + (1.0 - result.a) * plane.color.a * plane.color.rgb;
        result.a = result.a + (1.0 - result.a) * plane.color.a;
    }
    return result;
}

vec4 drawPlanes(in vec4 oldres, in vec3 pos, in vec3 dir, in float inc, PlaneParameters plane1,
                 PlaneParameters plane2, in float t, inout float tDepth) {
    vec4 result = oldres;
    result = drawPlanes(result, pos, dir, inc, plane1, t, tDepth);
    result = drawPlanes(result, pos, dir, inc, plane2, t, tDepth);
    return result;
}

vec4 drawPlanes(in vec4 oldres, in vec3 pos, in vec3 dir, in float inc, PlaneParameters plane1,
                 PlaneParameters plane2, PlaneParameters plane3,in float t, inout float tDepth) {
    vec4 result = oldres;
    result = drawPlanes(result, pos, dir, inc, plane1, t, tDepth);
    result = drawPlanes(result, pos, dir, inc, plane2, t, tDepth);
    result = drawPlanes(result, pos, dir, inc, plane3, t, tDepth);
    return result;
}

vec4 drawBackground(in vec4 oldres,in float depth, in float inc, in vec4 color,in float bgDepth,inout float tDepth){
    vec4 result = oldres;

    if((depth-inc) <= bgDepth && (depth) >= bgDepth){
        if (tDepth == -1.0 && color.a > 0.0) tDepth = depth;
        result.rgb = result.rgb + (1.0 - result.a) * color.a * color.rgb;
        result.a = result.a + (1.0 - result.a) * color.a;
    }
    
    return result;    
}

#endif  // IVW_RAYCASTGEOMETRY_GLSL
