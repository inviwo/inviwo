/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_PACKEDLIGHTSOURCE_H
#define IVW_PACKEDLIGHTSOURCE_H

#include <modules/opencl/openclmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/light/baselightsource.h>

namespace inviwo {

/**
 * \class PackedLightSource
 *
 * Data type that can be transfered to OpenCL device
 * Must be same as modules/opencl/cl/datastructures/lightsource.cl
 * Note that largest variables should be placed first
 * in order to ensure struct size
 */

typedef struct {
    mat4 tm;        // Transformation matrix from local to world coordinates
    vec4 radiance;  // cl_float3 == cl_float4
    vec2 size;      // width, height
    int type;       // LightSourceType, use integer to handle size of struct easier
    float area;     // area of light source
    float cosFOV;   // cos( (field of view)/2 ), used by cone light

    int padding[7];  // OpenCL requires sizes that are power of two (32, 64, 128 and so on)
} PackedLightSource;

// Transform a BaseLightSource to PackedLightSource
IVW_MODULE_OPENCL_API PackedLightSource
baseLightToPackedLight(const LightSource* lightsource, float radianceScale);

/**
* \brief Transform a BaseLightSource to PackedLightSource and apply the transformation matrix to the light
* source transformation matrix.
* Also transforms the width and height of the light source to
* match the supplied transformation extent, which means
* that a light size of (1,1) can be specified and it will be as large as the volume
* if transformLightMat == worldToTexture.
*
* @param const LightSource * lightsource Input light source
* @param float radianceScale Scales light power
* @param const mat4 & transformLightMat Transformation to be applied to lightSource.modelToWorld.
* @return PackedLightSource
*/
IVW_MODULE_OPENCL_API PackedLightSource baseLightToPackedLight(const LightSource* lightsource,
    float radianceScale,
    const mat4& transformLightMat);

} // namespace

#endif // IVW_PACKEDLIGHTSOURCE_H

