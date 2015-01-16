/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef SAMPLERS_CL
#define SAMPLERS_CL

__constant sampler_t smpUNormNoClampNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
__constant sampler_t smpUNormClampNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
__constant sampler_t smpUNormClampEdgeNearest = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
__constant sampler_t smpNormClampLinear = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
__constant sampler_t smpNormClampEdgeLinear = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
__constant sampler_t smpNormClampEdgeNearest = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;


typedef struct VolumeParameters_t {
    float16 modelToWorld;
    float16 worldToModel;
    float16 worldToTexture;
    float16 textureToWorld;
    float16 textureToIndex;                 // Transform from [0 1] to [-0.5 dim-0.5]
    float16 indexToTexture;                 // Transform from [-0.5 dim-0.5] to [0 1]
    float16 textureSpaceGradientSpacing;    // Maximum possible distance to go without ending up outside of a voxel (half of minimum voxel spacing for volumes with orthogonal basis)
    float worldSpaceGradientSampleSpacing;  // Spacing between gradient samples in world space 
    float formatScaling;                    // Scaling of data values.
    float formatOffset;                     // Offset of data values.
    float signedFormatScaling;              // Scaling of signed data values.
    float signedFormatOffset;               // Offset of signed data values.
    char padding__[44];                     // Padding to align to 512 bytes
} VolumeParameters;

/*
 * Get voxel data with linear interpolation, coordinate in [0 1]^3
 */
float getVoxel(read_only image3d_t volume, float4 pos) {
#ifdef SCALE_VOLUME_DATA
    // Scale 12-bit data
    return read_imagef(volume, smpNormClampEdgeLinear, pos).x * VOLUME_FORMAT_SCALING;  
#else 
    return read_imagef(volume, smpNormClampEdgeLinear, pos).x;  
#endif
}
/*
 * Get voxel data in unnormalized coordinates in [0 get_image_dim(volume)-1]
 */
float getVoxelUnorm(read_only image3d_t volume, int4 pos) {
#ifdef SCALE_VOLUME_DATA
    // Scale 12-bit data
    return read_imagef(volume, smpUNormNoClampNearest, pos).x * VOLUME_FORMAT_SCALING;  
#else 
    return read_imagef(volume, smpUNormNoClampNearest, pos).x;  
#endif
}
/*
 * Return a value mapped from data range [min,max] to [0,1] using linear interpolation
 * @param pos Coordinate in [0 1]^3
 */
float4 getNormalizedVoxel(read_only image3d_t volume, __constant VolumeParameters* volumeParams, float4 pos) {
    return (read_imagef(volume, smpNormClampEdgeLinear, pos) + volumeParams->formatOffset) * volumeParams->formatScaling;  
}


/*
 * Return a value mapped from data range [min,max] to [0,1] using nearest neighbor interpolation
 * @param pos Coordinate in [0 get_image_dim(volume)-1]
 */
float4 getNormalizedVoxelUnorm(read_only image3d_t volume, __constant VolumeParameters* volumeParams, int4 pos) {
    return (read_imagef(volume, smpUNormNoClampNearest, pos) + volumeParams->formatOffset) * volumeParams->formatScaling;  
}

/*
 * Return a value mapped from data range [min,max] to [-1,1] using linear interpolation
 * @param pos Coordinate in [0 1]^3
 */
float4 getSignNormalizedVoxel(read_only image3d_t volume, __constant VolumeParameters* volumeParams, float4 pos) {
    return (read_imagef(volume, smpNormClampEdgeLinear, pos) + volumeParams->signedFormatOffset) * volumeParams->signedFormatScaling;  
}

/*
 * Return a value mapped from data range [min,max] to [-1,1] using nearest neighbor interpolation
 * @param pos Coordinate in [0 get_image_dim(volume)-1]
 */
float4 getSignNormalizedVoxelUnorm(read_only image3d_t volume, __constant VolumeParameters* volumeParams, int4 pos) {
    return (read_imagef(volume, smpUNormNoClampNearest, pos) + volumeParams->signedFormatOffset) * volumeParams->signedFormatScaling;  
}

#endif // SAMPLERS_CL
