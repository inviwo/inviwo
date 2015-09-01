/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef IVW_IMAGE3D_WRITE_CL
#define IVW_IMAGE3D_WRITE_CL

// cl_khr_3d_image_writes is defined but not supported
// on some NVIDIA drivers (355.82 for example)
// This means that we cannot rely on cl_khr_3d_image_writes
// being correctly defined and must use our own define.
#ifdef SUPPORTS_VOLUME_WRITE //cl_khr_3d_image_writes 
    #pragma OPENCL_EXTENSION cl_khr_3d_image_writes : enable
    #define image_3d_write_uint8_t __write_only image_3d_t
    #define image_3d_write_uint16_t __write_only image_3d_t
    #define image_3d_write_uint32_t __write_only image_3d_t
    #define image_3d_write_float16_t __write_only image_3d_t
    #define image_3d_write_float32_t __write_only image_3d_t
#else
    #define image_3d_write_uint8_t __global uchar*
    #define image_3d_write_uint16_t __global ushort*
    #define image_3d_write_uint32_t __global uint*
    #define image_3d_write_float16_t __global half*
    #define image_3d_write_float32_t __global float*
#endif


void writeImageUInt8f(image_3d_write_uint8_t image, int4 coord, int4 dimension, float val) {
#ifdef SUPPORTS_VOLUME_WRITE
    write_imagef(image, coord, (uint4)(val));
#else
    // The preferred method for conversions from floating-point values to normalized integer
    image[coord.x + coord.y*dimension.x + coord.z*dimension.x*dimension.y] = convert_uchar_sat_rte(val*255.f);
#endif
}

void write_imageUInt8(image_3d_write_uint8_t image, int4 coord, int4 dimension, uchar val) {
#ifdef SUPPORTS_VOLUME_WRITE
    write_imageui(image, coord, (uint4)(val));
#else
    image[coord.x + coord.y*dimension.x + coord.z*dimension.x*dimension.y] = val;
#endif
}


#endif // IVW_IMAGE3D_WRITE_CL
