/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

// Inspired by Inigo Quilez
// http://www.iquilezles.org/www/articles/fog/fog.htm

#ifndef IVW_FOG_GLSL
#define IVW_FOG_GLSL

vec3 computeFog(in vec3 pixelColor, in float distance, in vec3 fogColor, in float density) {
    float amount = 1.0 - exp(-distance * density);
    return mix(pixelColor, fogColor, amount);
}

vec3 computeFogScatter(in vec3 pixelColor, in float distance, in vec3 fogColor, in float scatter) {
    return pixelColor * (1.0 - exp(-distance * scatter)) + fogColor * exp(-distance * scatter);
}

vec3 computeFogInOutScatter(in vec3 pixelColor, in float distance, in vec3 fogColor,
                            in vec3 extinction, in vec3 scatter) {
    vec3 extColor = exp(-distance * extinction);
    vec3 insColor = exp(-distance * scatter);
    return pixelColor * (1.0 - extColor) + fogColor * insColor;
}

#endif
