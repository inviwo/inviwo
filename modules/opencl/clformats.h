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
IVW_MODULE_OPENCL_API cl::ImageFormat dataFormatToCLImageFormat(DataFormatEnums::Id format);

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

    enum Normalization {
        NONE,
        NORMALIZED,
        SIGN_NORMALIZED
    };

    struct CLFormat {
        CLFormat()
            : format(cl::ImageFormat(CL_R, CL_UNORM_INT8))
            , normalization(NONE)
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
        CLFormatArray_[DataFormatEnums::FLOAT16]     = CLFormat(CL_R, CL_HALF_FLOAT, NONE);
        CLFormatArray_[DataFormatEnums::FLOAT32]     = CLFormat(CL_R, CL_FLOAT, NONE);
        CLFormatArray_[DataFormatEnums::INT8]        = CLFormat(CL_R, CL_SNORM_INT8, SIGN_NORMALIZED);
        CLFormatArray_[DataFormatEnums::INT16]       = CLFormat(CL_R, CL_SNORM_INT16, SIGN_NORMALIZED);
        CLFormatArray_[DataFormatEnums::INT32]       = CLFormat(CL_R, CL_SIGNED_INT32, NONE);
        CLFormatArray_[DataFormatEnums::UINT8]       = CLFormat(CL_R, CL_UNORM_INT8, NORMALIZED);
        CLFormatArray_[DataFormatEnums::UINT16]      = CLFormat(CL_R, CL_UNORM_INT16, NORMALIZED);
        CLFormatArray_[DataFormatEnums::UINT32]      = CLFormat(CL_R, CL_UNSIGNED_INT32, NONE);
        //2 channels
        CLFormatArray_[DataFormatEnums::Vec2FLOAT16] = CLFormat(CL_RG, CL_HALF_FLOAT, NONE);
        CLFormatArray_[DataFormatEnums::Vec2FLOAT32] = CLFormat(CL_RG, CL_FLOAT, NONE);
        CLFormatArray_[DataFormatEnums::Vec2INT8]    = CLFormat(CL_RG, CL_SNORM_INT8, SIGN_NORMALIZED);
        CLFormatArray_[DataFormatEnums::Vec2INT16]   = CLFormat(CL_RG, CL_SNORM_INT16, SIGN_NORMALIZED);
        CLFormatArray_[DataFormatEnums::Vec2INT32]   = CLFormat(CL_RG, CL_SIGNED_INT32, NONE);
        CLFormatArray_[DataFormatEnums::Vec2UINT8]   = CLFormat(CL_RG, CL_UNORM_INT8, NORMALIZED);
        CLFormatArray_[DataFormatEnums::Vec2UINT16]  = CLFormat(CL_RG, CL_UNORM_INT16, NORMALIZED);
        CLFormatArray_[DataFormatEnums::Vec2UINT32]  = CLFormat(CL_RG, CL_UNSIGNED_INT32, NONE);
        //3 channels
        // Not supported

        //4 channels
        CLFormatArray_[DataFormatEnums::Vec4FLOAT16] = CLFormat(CL_RGBA, CL_HALF_FLOAT, NONE);
        CLFormatArray_[DataFormatEnums::Vec4FLOAT32] = CLFormat(CL_RGBA, CL_FLOAT, NONE);
        CLFormatArray_[DataFormatEnums::Vec4INT8]    = CLFormat(CL_RGBA, CL_SNORM_INT8, SIGN_NORMALIZED);
        CLFormatArray_[DataFormatEnums::Vec4INT16]   = CLFormat(CL_RGBA, CL_SNORM_INT16, SIGN_NORMALIZED);
        CLFormatArray_[DataFormatEnums::Vec4INT32]   = CLFormat(CL_RGBA, CL_SIGNED_INT32, NONE);
        CLFormatArray_[DataFormatEnums::Vec4UINT8]   = CLFormat(CL_RGBA, CL_UNORM_INT8, NORMALIZED);
        CLFormatArray_[DataFormatEnums::Vec4UINT16]  = CLFormat(CL_RGBA, CL_UNORM_INT16, NORMALIZED);
        CLFormatArray_[DataFormatEnums::Vec4UINT32]  = CLFormat(CL_RGBA, CL_UNSIGNED_INT32, NONE);
    };

    CLFormat getCLFormat(DataFormatEnums::Id id) const;

private:
    CLFormat CLFormatArray_[DataFormatEnums::NUMBER_OF_FORMATS];
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

