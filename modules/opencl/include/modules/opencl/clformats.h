/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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
 * Outputs an error message if a corresponding format does not exist and then returns the default
 * ImageFormat.
 *
 * @see DataFormat
 * @param format Id of a DataFormat
 * @return Default ImageFormat created by the constructor if a corresponding format does not exist,
 * otherwise an ImageFormat with corresponding channel and data type.
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
class IVW_MODULE_OPENCL_API CLFormats {
public:
    enum class Normalization {
        None,
        Normalized,
        SignNormalized,
    };

    struct CLFormat {
        CLFormat();
        CLFormat(const cl::ImageFormat& f, Normalization n, float sc = 1.f);
        CLFormat(const int channelOrder, const int channelType, Normalization n, float sc = 1.f);
        cl::ImageFormat format;
        Normalization normalization;
        double scaling;
        bool valid;
    };

    CLFormats();
    const CLFormat& getCLFormat(DataFormatId id) const;

    static const CLFormat& get(DataFormatId id);

private:
    CLFormat CLFormatArray_[static_cast<size_t>(DataFormatId::NumberOfFormats)];
};

IVW_MODULE_OPENCL_API bool operator==(const CLFormats::CLFormat& a, const CLFormats::CLFormat& b);
IVW_MODULE_OPENCL_API bool operator!=(const CLFormats::CLFormat& a, const CLFormats::CLFormat& b);

}  // namespace inviwo

#endif  // IVW_CLFORMATS_H
