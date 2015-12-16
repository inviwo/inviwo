/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "utils/structs.glsl"
#include "utils/shading.glsl"

uniform LightParameters light;
uniform CameraParameters camera;

in vec4 color_;
in vec3 worldPos_;

in vec3 startPos_;
in vec3 endPos_;
in vec3 gEndplanes[2];


uniform float radius;

vec3 perp(vec3 v)
{
    vec3 b = cross(v, vec3(0, 0, 1));
    if (dot(b, b) < 0.01)
        b = cross(v, vec3(0, 1, 0));
    return b;
}

bool cylinderIntersect(vec3 origin , vec3 dir, out float t){
	vec3 A = startPos_; 
	vec3 B = endPos_;
    float Epsilon = 0.0000001;
    float extent = distance(A, B);
    vec3 W = (B - A) / extent;
    vec3 U = perp(W);
    vec3 V = cross(U, W);
    U = normalize(cross(V, W));
    V = normalize(V);
    float rSqr = radius*radius;
    vec3 diff = origin - 0.5 * (A + B);
    mat3 basis = mat3(U, V, W);
    vec3 P = diff * basis;
    float dz = dot(W, dir);
    if (abs(dz) >= 1.0 - Epsilon) {
        float radialSqrDist = rSqr - P.x*P.x - P.y*P.y;
        if (radialSqrDist < 0.0)
            return false;
        t = (dz > 0.0 ? -P.z : P.z) + extent * 0.5;
        return true;
    }

    vec3 D = vec3(dot(U, dir), dot(V, dir), dz);
    float a0 = P.x*P.x + P.y*P.y - rSqr;
    float a1 = P.x*D.x + P.y*D.y;
    float a2 = D.x*D.x + D.y*D.y;
    float discr = a1*a1 - a0*a2;
    if (discr < 0.0)
        return false;

    if (discr > Epsilon) {
        float root = sqrt(discr);
        float inv = 1.0/a2;
        t = (-a1 + root)*inv;
        return true;
    }

    t = -a1/a2;
    return true;
}


void main() {
	vec3 camPos = (camera.viewToWorld * vec4(0,0,0,1)).xyz;
	vec3 dir = normalize(camPos - worldPos_);

	float d;
	if(!cylinderIntersect(worldPos_ , dir , d)){
		discard;
		return;
	}

	vec3 hitPoint = worldPos_ + d * dir;
    if (dot(hitPoint - startPos_, gEndplanes[0]) < 0.0) {
        discard;
        return;
    }


    if (dot(hitPoint - endPos_, gEndplanes[1]) > 0.0) {
        discard;
        return;
    }


	FragData0 = vec4(dir,1);



	vec3 x0 = hitPoint;
    vec3 x1 = startPos_;
    vec3 x2 = endPos_;
    float length = distance(x1, x2);
    vec3 v = (x2 - x1) / length;
    float t = dot(x0 - x1, v);
    vec3 spinePoint = x1 + t * v;
    vec3 N = normalize(hitPoint - spinePoint);

    
    vec3 color = APPLY_LIGHTING(light, color_.rgb, color_.rgb, vec3(1.0f), hitPoint, N, dir);

    vec4 ndc = camera.worldToClip * vec4(hitPoint, 1);
    gl_FragDepth = ((ndc.z / ndc.w) + 1 ) /2;
    FragData0 = vec4(color.rgb, color_.a);
}
