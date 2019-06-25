/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
 
vec3 sampleVelocity(vec3 worldPos);

/**
 * Advect point p at \p worldPos one step using Euler integration
 * Caller needs to implement `vec3 sampleVelocity(vec3 worldPos)`
 * @ see streamparticles.comp
 */ 
vec3 advectEuler(vec3 worldPos, float stepSize) {
    return worldPos + sampleVelocity(worldPos) * stepSize;
}

/**
 * Advect point p at \p worldPos one step usig 4th order Runge-Kutta integration
 * Caller needs to implement `vec3 sampleVelocity(vec3 worldPos)`
 * @ see streamparticles.comp
 */ 
vec3 advectRK4(vec3 worldPos, float stepSize) {
    const float h2 = stepSize / 2.0f;
    vec3 k1 = sampleVelocity(worldPos);
    vec3 k2 = sampleVelocity(worldPos + k1 * h2);
    vec3 k3 = sampleVelocity(worldPos + k2 * h2);
    vec3 k4 = sampleVelocity(worldPos + k3 * stepSize);
    return worldPos + (k1 + k2 + k2 + k3 + k3 + k4) * stepSize / 6.0;
}
