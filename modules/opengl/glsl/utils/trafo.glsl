/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#ifndef IVW_TRAFO_GLSL
#define IVW_TRAFO_GLSL

mat4 rotate(vec3 axis, float angle) {
  vec3 a = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;

  return mat4(
    oc*a.x*a.x + c,      oc*a.y*a.x + a.z*s,  oc*a.z*a.x - a.y*s,  0.0,
    oc*a.x*a.y - a.z*s,  oc*a.y*a.y + c,      oc*a.z*a.y + a.x*s,  0.0,
    oc*a.x*a.z + a.y*s,  oc*a.y*a.z - a.x*s,  oc*a.z*a.z + c,      0.0,
    0.0,                 0.0,                 0.0,                 1.0
  );
}

mat4 rotate(vec3 from, vec3 to) {
    vec3 v1 = normalize(from);
    vec3 w1 = normalize(to);
    float cosTheta = dot(v1, w1);

    if (cosTheta > 1.0 - 1e-6) { // vectors are the same 
        return mat4(1.0);
    }

    if (cosTheta < -1.0 + 1e-6) { // vectors are opposite
        vec3 orthogonal = abs(v1.x) < 0.1 ? vec3(1, 0, 0) : vec3(0, 1, 0);
        return rotate(cross(v1, orthogonal), 3.14159265);
    }

    // General case
    vec3 axis = cross(v1, w1);
    float angle = acos(clamp(cosTheta, -1.0, 1.0));
    return rotate(axis, angle);
}

mat4 scale(vec3 scale) {
  return mat4(
    scale.x, 0.0, 0.0, 0.0,
    0.0, scale.y, 0.0, 0.0,
    0.0, 0.0, scale.z, 0.0,
    0.0, 0.0, 0.0,     1.0
  );
}

mat4 translate(vec3 dist) {
  return mat4(
    1.0, 0.0, 0.0, dist.x,
    0.0, 1.0, 0.0, dist.y,
    0.0, 0.0, 1.0, dist.z,
    0.0, 0.0, 0.0, 1.0
  );
}

#endif // IVW_TRAFO_GLSL
