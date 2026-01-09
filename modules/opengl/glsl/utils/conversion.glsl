/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025-2026 Inviwo Foundation
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

#ifndef IVW_CONVERSION_GLSL
#define IVW_CONVERSION_GLSL

#include "utils/structs.glsl" //! #include "./structs.glsl"

/**
 * Map the value @p value from value space to the OpenGL output range as defined by the underlying
 * data format and specified by @p map as follows:
 *   + map from value range to dataRange for float and non-normalized GL data formats
 *   + normalized dataRange [0,1] for normalized, unsigned GL data formats
 *   + or sign normalized dataRange [-1,1] for normalized, signed GL data formats
 * 
 * NOTE: negative values of sign normalized data are clamped to zero by OpenGL before
 *       renormalization when writing those in a fragment shader (confirmed on NVIDIA)
 *
 * @param value   value in value space
 * @param map     parameters used for the range mapping
 * @return @p value converted to the OpenGL output range
 */
float mapFromValueToGLOutput(in float value, in RangeConversionMap map) {
    return ((value - map.inputOffset) * map.scale) + map.outputOffset;
}
vec2 mapFromValueToGLOutput(in vec2 value, in RangeConversionMap map) {
    return ((value - map.inputOffset) * map.scale) + map.outputOffset;
}
vec3 mapFromValueToGLOutput(in vec3 value, in RangeConversionMap map) {
    return ((value - map.inputOffset) * map.scale) + map.outputOffset;
}
vec4 mapFromValueToGLOutput(in vec4 value, in RangeConversionMap map) {
    return ((value - map.inputOffset) * map.scale) + map.outputOffset;
}

/**
 * Map the normalized value @p value from [0,1] to value space
 * 
 * @param value   normalized value in [0,1]
 * @param map     parameters used for the range mapping
 * @return @p value converted to value space
 */
float mapFromNormalizedToValue(in float value, in ImageParameters params) {
    float valueScale = params.texToValue.scale / params.texToNormalized.scale;
    return (value * valueScale) + params.texToValue.outputOffset;
}
vec2 mapFromNormalizedToValue(in vec2 value, in ImageParameters params) {
    float valueScale = params.texToValue.scale / params.texToNormalized.scale;
    return (value * valueScale) + params.texToValue.outputOffset;
}
vec3 mapFromNormalizedToValue(in vec3 value, in ImageParameters params) {
    float valueScale = params.texToValue.scale / params.texToNormalized.scale;
    return (value * valueScale) + params.texToValue.outputOffset;
}
vec4 mapFromNormalizedToValue(in vec4 value, in ImageParameters params) {
    float valueScale = params.texToValue.scale / params.texToNormalized.scale;
    return (value * valueScale) + params.texToValue.outputOffset;
}
float mapFromNormalizedToValue(in float value, in VolumeParameters params) {
    float valueScale = params.texToValue.scale / params.texToNormalized.scale;
    return (value * valueScale) + params.texToValue.outputOffset;
}
vec2 mapFromNormalizedToValue(in vec2 value, in VolumeParameters params) {
    float valueScale = params.texToValue.scale / params.texToNormalized.scale;
    return (value * valueScale) + params.texToValue.outputOffset;
}
vec3 mapFromNormalizedToValue(in vec3 value, in VolumeParameters params) {
    float valueScale = params.texToValue.scale / params.texToNormalized.scale;
    return (value * valueScale) + params.texToValue.outputOffset;
}
vec4 mapFromNormalizedToValue(in vec4 value, in VolumeParameters params) {
    float valueScale = params.texToValue.scale / params.texToNormalized.scale;
    return (value * valueScale) + params.texToValue.outputOffset;
}

#endif  // IVW_CONVERSION_GLSL
