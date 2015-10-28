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

#ifndef IVW_CLFORMATS_H
#define IVW_CLFORMATS_H

#include <modules/opencl/openclmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/opencl/inviwoopencl.h>

namespace inviwo {


    
/**
 * Create a cl::ImageFormat based on DataFormatId.
 * Outputs an error message if a corresponding format does not exist and then returns the default ImageFormat.
 *
 * @see DataFormat
 * @param format Id of a DataFormat
 * @return Default ImageFormat created by the constructor
 * if a corresponding format does not exist, otherwise an ImageFormat with corresponding channel and data type.
 */
IVW_MODULE_OPENCL_API cl::ImageFormat dataFormatToCLImageFormat(DataFormatId format);

/**
 * \class CLFormats
 *
 * \brief Matches inviwo data formats to OpenCL image formats. 
 *
 * 1, 2 and 4 channel data are supported by OpenCL.
 * One channel data will be possible to retrieve using the x component.
 * Two channel data will be possible to retrieve using the xy component.
 * Four channel data will be possible to retrieve using the xyzw component.
 */
class IVW_MODULE_OPENCL_API CLFormats{

public:

    enum class Normalization {
        None,
        Normalized,
        SignNormalized,
    };

    struct CLFormat {
        CLFormat()
            : format(cl::ImageFormat(CL_R, CL_UNORM_INT8))
            , normalization(Normalization::None)
            , scaling(1.f)
            , valid(false) {}
        CLFormat(const cl::ImageFormat& f, Normalization n, float sc = 1.f)
            : format(f)
            , normalization(n)
            , scaling(sc)
            , valid(true) {}
        CLFormat(const int channelOrder, const int channelType, Normalization n, float sc = 1.f)
            : format(cl::ImageFormat(channelOrder, channelType))
            , normalization(n)
            , scaling(sc)
            , valid(true) {}
        cl::ImageFormat format;
        Normalization normalization;
        double scaling;
        bool valid;
    };

    CLFormats() {
        //1 channel
        CLFormatArray_[static_cast<size_t>(DataFormatId::Float16)] = CLFormat(CL_R, CL_HALF_FLOAT, Normalization::None);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Float32)] = CLFormat(CL_R, CL_FLOAT, Normalization::None);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Int8)] = CLFormat(CL_R, CL_SNORM_INT8, Normalization::SignNormalized);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Int16)] = CLFormat(CL_R, CL_SNORM_INT16, Normalization::SignNormalized);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Int32)] = CLFormat(CL_R, CL_SIGNED_INT32, Normalization::None);
        CLFormatArray_[static_cast<size_t>(DataFormatId::UInt8)] = CLFormat(CL_R, CL_UNORM_INT8, Normalization::Normalized);
        CLFormatArray_[static_cast<size_t>(DataFormatId::UInt16)] = CLFormat(CL_R, CL_UNORM_INT16, Normalization::Normalized);
        CLFormatArray_[static_cast<size_t>(DataFormatId::UInt32)] = CLFormat(CL_R, CL_UNSIGNED_INT32, Normalization::None);
        //2 channels
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec2Float16)] = CLFormat(CL_RG, CL_HALF_FLOAT, Normalization::None);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec2Float32)] = CLFormat(CL_RG, CL_FLOAT, Normalization::None);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec2Int8)] = CLFormat(CL_RG, CL_SNORM_INT8, Normalization::SignNormalized);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec2Int16)] = CLFormat(CL_RG, CL_SNORM_INT16, Normalization::SignNormalized);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec2Int32)] = CLFormat(CL_RG, CL_SIGNED_INT32, Normalization::None);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec2UInt8)] = CLFormat(CL_RG, CL_UNORM_INT8, Normalization::Normalized);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec2UInt16)] = CLFormat(CL_RG, CL_UNORM_INT16, Normalization::Normalized);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec2UInt32)] = CLFormat(CL_RG, CL_UNSIGNED_INT32, Normalization::None);
        //3 channels
        // Not supported

        //4 channels
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec4Float16)] = CLFormat(CL_RGBA, CL_HALF_FLOAT, Normalization::None);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec4Float32)] = CLFormat(CL_RGBA, CL_FLOAT, Normalization::None);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec4Int8)] = CLFormat(CL_RGBA, CL_SNORM_INT8, Normalization::SignNormalized);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec4Int16)] = CLFormat(CL_RGBA, CL_SNORM_INT16, Normalization::SignNormalized);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec4Int32)] = CLFormat(CL_RGBA, CL_SIGNED_INT32, Normalization::None);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec4UInt8)] = CLFormat(CL_RGBA, CL_UNORM_INT8, Normalization::Normalized);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec4UInt16)] = CLFormat(CL_RGBA, CL_UNORM_INT16, Normalization::Normalized);
        CLFormatArray_[static_cast<size_t>(DataFormatId::Vec4UInt32)] = CLFormat(CL_RGBA, CL_UNSIGNED_INT32, Normalization::None);
    };

    CLFormat getCLFormat(DataFormatId id) const;

private:
    CLFormat CLFormatArray_[static_cast<size_t>(DataFormatId::NumberOfFormats)];
};

static const CLFormats CLFormats_ = CLFormats();
#include <warn/push>
#include <warn/ignore/unused-function>
static const CLFormats* getCLFormats() {
    return &CLFormats_;
}
#include <warn/pop>    

} // namespace

#endif // IVW_CLFORMATS_H

