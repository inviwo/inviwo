/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMECL_BASE_H
#define IVW_VOLUMECL_BASE_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <inviwo/core/datastructures/volume/volume.h>

#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/openclmoduledefine.h>

namespace inviwo {


// This class enables inviwo to use cl::Image(s) in a generic way (i.e. not caring if it is an Image2D or Image2DGL/ImageGL).
class IVW_MODULE_OPENCL_API VolumeCLBase {

public:
    struct VolumeParameters {
        mat4 modelToWorld;
        mat4 worldToModel;
        mat4 worldToTexture;
        mat4 textureToWorld;
        mat4 textureToIndex;                    // Transform from [0 1] to [-0.5 dim-0.5]
        mat4 indexToTexture;                    // Transform from [-0.5 dim-0.5] to [0 1]
        mat4 textureSpaceGradientSpacing;       // Maximum possible distance to go without ending up outside of a voxel (half of minimum voxel spacing for volumes with orthogonal basis)
        float worldSpaceGradientSampleSpacing;  // Spacing between gradient samples in world space 
        float formatScaling;                    // Scaling of data values.
        float formatOffset;                     // Offset of data values.
        float signedFormatScaling;              // Scaling of signed data values.
        float signedFormatOffset;               // Offset of signed data values.
        char padding__[44];                     // Padding to align to 512 bytes
    };
    VolumeCLBase();
    VolumeCLBase(const VolumeCLBase& other);
    virtual ~VolumeCLBase();

    virtual cl::Image& getEditable() = 0;
    virtual const cl::Image& get() const = 0;

    /** 
     * \brief Calculates scaling for 12-bit data dependent on internal OpenCL format.
     * Scaling will be applied using: dataValue * scaling
     * @return vec2 Offset in first component and scaling in second.
     */
    virtual vec2 getVolumeDataOffsetAndScaling(const Volume* volume) const;
    const Buffer& getVolumeStruct(const Volume* volume) const;
protected:
    Buffer volumeStruct_; // Contains VolumeParameters
};

} // namespace

namespace cl {

// Kernel argument specializations for VolumeCLBase type
// (enables calling cl::Queue::setArg with VolumeCLBase)
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const inviwo::VolumeCLBase& value);

} // namespace cl



#endif // IVW_VOLUMECL_BASE_H
