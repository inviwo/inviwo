/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#ifndef IVW_CLASSIFICATION_GLSL
#define IVW_CLASSIFICATION_GLSL

vec4 applyTF(sampler2D transferFunction, vec4 voxel) {
    return texture(transferFunction, vec2(voxel.r, 0.5));
}

vec4 applyTF(sampler2D transferFunction, vec4 voxel, int channel) {
    return texture(transferFunction, vec2(voxel[channel], 0.5));
}

vec4 applyTF(sampler2D transferFunction, float intensity) {
    return texture(transferFunction, vec2(intensity, 0.5));
}

// Absolute TF mode: remap normalized voxel value through the volume's data range into TF texture
// space. The TF texture covers [tfParams.rangeMin, tfParams.rangeMax] in data-space coordinates.
vec4 applyTF(sampler2D transferFunction, TFParameters tfParams, NormalizationMap texToNormalized,
             vec4 voxel) {
    // Convert from normalized [0,1] back to data-space value
    float absValue = voxel.r / texToNormalized.scale + texToNormalized.offset;
    // Map from data-space to TF texture coordinate [0,1]
    float tfCoord = (absValue - tfParams.rangeMin) / (tfParams.rangeMax - tfParams.rangeMin);
    return texture(transferFunction, vec2(clamp(tfCoord, 0.0, 1.0), 0.5));
}

vec4 applyTF(sampler2D transferFunction, TFParameters tfParams, NormalizationMap texToNormalized,
             vec4 voxel, int channel) {
    float absValue = voxel[channel] / texToNormalized.scale + texToNormalized.offset;
    float tfCoord = (absValue - tfParams.rangeMin) / (tfParams.rangeMax - tfParams.rangeMin);
    return texture(transferFunction, vec2(clamp(tfCoord, 0.0, 1.0), 0.5));
}

vec4 applyTF(sampler2D transferFunction, TFParameters tfParams, NormalizationMap texToNormalized,
             float intensity) {
    float absValue = intensity / texToNormalized.scale + texToNormalized.offset;
    float tfCoord = (absValue - tfParams.rangeMin) / (tfParams.rangeMax - tfParams.rangeMin);
    return texture(transferFunction, vec2(clamp(tfCoord, 0.0, 1.0), 0.5));
}

#endif  // IVW_CLASSIFICATION_GLSL
