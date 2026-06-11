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

#include "utils/structs.glsl"
#include "utils/conversion.glsl"

vec4 applyTF(sampler2D transferFunction, in float normalizedValue) {
    return texture(transferFunction, vec2(normalizedValue, 0.5));
}
vec4 applyTF(sampler2D transferFunction, in vec4 normalizedVoxel) {
    return applyTF(transferFunction, normalizedVoxel.r);
}
vec4 applyTF(sampler2D transferFunction, in vec4 normalizedVoxel, int channel) {
    return applyTF(transferFunction, normalizedVoxel[channel]);
}

vec4 applyTF(sampler2D transferFunction, in TFParameters tfParams, in float value) {
    // Map from value-space to TF texture coordinate [0,1]
    float tfCoord = (value - tfParams.rangeMin) / (tfParams.rangeMax - tfParams.rangeMin);
    return texture(transferFunction, vec2(clamp(tfCoord, 0.0, 1.0), 0.5));
}

vec4 applyTF(sampler2D transferFunction, in TFParameters tfParams,
             in NormalizationMap texToNormalized, in RangeConversionMap texToValue,
             in float normalizedValue) {
    // Convert from normalized [0,1] to value-space
    float value = mapFromNormalizedToValue(normalizedValue, texToNormalized, texToValue);
    return applyTF(transferFunction, tfParams, value);
}

// Volume version of applyTF
vec4 applyTF(sampler2D transferFunction, in TFParameters tfParams,
             in VolumeParameters volumeParameters, in float normalizedValue) {
    return applyTF(transferFunction, tfParams, volumeParameters.texToNormalized,
                   volumeParameters.texToValue, normalizedValue);
}

vec4 applyTF(sampler2D transferFunction, in TFParameters tfParams,
             in VolumeParameters volumeParameters, in vec4 normalizedVoxel) {
    return applyTF(transferFunction, tfParams, volumeParameters, normalizedVoxel.r);
}

vec4 applyTF(sampler2D transferFunction, in TFParameters tfParams,
             in VolumeParameters volumeParameters, in vec4 normalizedVoxel, in int channel) {
    return applyTF(transferFunction, tfParams, volumeParameters, normalizedVoxel[channel]);
}

// Image version of applyTF
vec4 applyTF(sampler2D transferFunction, in TFParameters tfParams,
             in ImageParameters imageParameters, in float normalizedValue) {
    // Convert from normalized [0,1] to value-space
    float value = mapFromNormalizedToValue(normalizedValue, imageParameters);
    return applyTF(transferFunction, tfParams, value);
}

vec4 applyTF(sampler2D transferFunction, in TFParameters tfParams,
             in ImageParameters imageParameters, in vec4 normalizedVoxel) {
    return applyTF(transferFunction, tfParams, imageParameters, normalizedVoxel.r);
}

vec4 applyTF(sampler2D transferFunction, in TFParameters tfParams,
             in ImageParameters imageParameters, in vec4 normalizedVoxel, in int channel) {
    return applyTF(transferFunction, tfParams, imageParameters, normalizedVoxel[channel]);
}

#endif  // IVW_CLASSIFICATION_GLSL
