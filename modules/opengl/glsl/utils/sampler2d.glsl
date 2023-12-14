/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#ifndef IVW_SAMPLER2D_GLSL
#define IVW_SAMPLER2D_GLSL

#include "utils/structs.glsl"

vec4 textureLookup2Dnormalized(sampler2D tex, ImageParameters textureParams, vec2 samplePos) {
    return texture(tex, samplePos);
}

vec4 textureLookup2Dscreen(sampler2D tex, ImageParameters textureParams, vec2 samplePos) {
    return texture(tex, samplePos*textureParams.reciprocalDimensions);
}

// Return a the raw texture
vec4 getTexel(sampler2D image, ImageParameters imageParams, vec2 samplePos) {
    return texture(image, samplePos);
}
// Return a value mapped from data range [min,max] to [0,1]
// The data range [min, max] here is the range specified by the dataMap_.dataRange of image
// and does not always match the range of the specified data type.
// We have to apply some scaling here to compensate for the fact that the data range of the data is
// not the same as the min/max of the data type. And at the same time take into account that OpenGL
// also does its own normalization, which if different for floating point and integer types
// see: https://www.opengl.org/wiki/Normalized_Integer
// the actual calculation of the scaling parameters is done in imageutils.cpp
vec4 getNormalizedTexel(sampler2D image, ImageParameters imageParams, vec2 samplePos) {
    return (texture(image, samplePos) + imageParams.formatOffset)
        * (1.0 - imageParams.formatScaling);
}

float getNormalizedTexelChannel(sampler2D image, ImageParameters imageParams, vec2 samplePos,int channel) {
    vec4 v = getNormalizedTexel(image,imageParams,samplePos);
    return v[channel];
}


// Return a value mapped from data range [min,max] to [-1,1]
// Same as getNormalizedTexel but for signed types. 
vec4 getSignNormalizedTexel(sampler2D image, ImageParameters imageParams, vec2 samplePos) {
    return (texture(image, samplePos) + imageParams.signedFormatOffset)
        * (1.0 - imageParams.signedFormatScaling);
}


//
// Fetch texture data using texture indices [0,N]
//

// Return a the raw texture
vec4 getTexel(sampler2D image, ImageParameters imageParams, ivec2 samplePos) {
#ifdef GLSL_VERSION_140
    return texelFetch(image, samplePos, 0);
#else
    return texture(image, samplePos);
#endif
}
// Return a value mapped from data range [min,max] to [0,1]
vec4 getNormalizedTexel(sampler2D image, ImageParameters imageParams, ivec2 samplePos) {
#ifdef GLSL_VERSION_140
    return (texelFetch(image, samplePos, 0) + imageParams.formatOffset)
        * (1.0 - imageParams.formatScaling);
#else
    return (texture(image, samplePos) + imageParams.formatOffset)
        * (1.0 - imageParams.formatScaling);
#endif
}
// Return a value mapped from data range [min,max] to [-1,1]
vec4 getSignNormalizedTexel(sampler2D image, ImageParameters imageParams, ivec2 samplePos) {
#ifdef GLSL_VERSION_140
    return (texelFetch(image, samplePos, 0) + imageParams.signedFormatOffset)
        * (1.0 - imageParams.signedFormatScaling);
#else
    return (texture(image, samplePos) + imageParams.signedFormatOffset)
        * (1.0 - imageParams.signedFormatScaling);
#endif
}

#endif  // IVW_SAMPLER2D_GLSL
