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

#include "clformats.h"
#include "openclformatexception.h"

namespace inviwo {

cl::ImageFormat dataFormatToCLImageFormat(inviwo::DataFormatEnums::Id format)
{
    // Difference between SNORM/UNORM and SIGNED/UNSIGNED INT
    // SNORM/UNORM:
    // Each channel component is a normalized integer value
    // SIGNED/UNSIGNED:
    // Each channel component is an unnormalized integer value
    // From the specification (note that CL_Rx, CL_RGx and CL_RGBx are optional):
    // If the image channel order is CL_A, CL_INTENSITY, CL_Rx, CL_RA, CL_RGx,
    // CL_RGBx, CL_ARGB, CL_BGRA, or CL_RGBA, the border color is (0.0f, 0.0f,
    // 0.0f, 0.0f).
    // If the image channel order is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE, the border
    // color is (0.0f, 0.0f, 0.0f, 1.0f).
    // CL_INTENSITY maps single value I to (I, I, I, I)
    cl::ImageFormat clFormat;

    switch (format) {
        case DataFormatEnums::NOT_SPECIALIZED:
            throw OpenCLFormatException("Unsupported data format: NOT_SPECIALIZED", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::FLOAT16:
            clFormat = cl::ImageFormat(CL_R, CL_HALF_FLOAT);
            break;

        case DataFormatEnums::FLOAT32:
            clFormat = cl::ImageFormat(CL_R, CL_FLOAT);
            break;

        case DataFormatEnums::FLOAT64:
            throw OpenCLFormatException("Unsupported data format: FLOAT64", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::INT8:
            clFormat = cl::ImageFormat(CL_R, CL_SNORM_INT8);
            break;

        case DataFormatEnums::INT16:
            clFormat = cl::ImageFormat(CL_R, CL_SNORM_INT16);
            break;

        case DataFormatEnums::INT32:
            clFormat = cl::ImageFormat(CL_R, CL_SIGNED_INT32);
            break;

        case DataFormatEnums::INT64:
            throw OpenCLFormatException("Unsupported data format: INT64", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::UINT8:
            clFormat = cl::ImageFormat(CL_R, CL_UNORM_INT8);
            break;

        case DataFormatEnums::UINT16:
            clFormat = cl::ImageFormat(CL_R, CL_UNORM_INT16);
            break;

        case DataFormatEnums::UINT32:
            clFormat = cl::ImageFormat(CL_R, CL_UNSIGNED_INT32);
            break;

        case DataFormatEnums::UINT64:
            throw OpenCLFormatException("Unsupported data format: UINT64", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec2FLOAT16:
            clFormat = cl::ImageFormat(CL_RG, CL_HALF_FLOAT);
            break;

        case DataFormatEnums::Vec2FLOAT32:
            clFormat = cl::ImageFormat(CL_RG, CL_FLOAT);
            break;

        case DataFormatEnums::Vec2FLOAT64:
            throw OpenCLFormatException("Unsupported data format: Vec2FLOAT64", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec2INT8:
            clFormat = cl::ImageFormat(CL_RG, CL_SNORM_INT8);
            break;

        case DataFormatEnums::Vec2INT16:
            clFormat = cl::ImageFormat(CL_RG, CL_SNORM_INT16);
            break;

        case DataFormatEnums::Vec2INT32:
            clFormat = cl::ImageFormat(CL_RG, CL_SIGNED_INT32);
            break;

        case DataFormatEnums::Vec2INT64:
            throw OpenCLFormatException("Unsupported data format: Vec2INT64", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec2UINT8:
            clFormat = cl::ImageFormat(CL_RG, CL_UNORM_INT8);
            break;

        case DataFormatEnums::Vec2UINT16:
            clFormat = cl::ImageFormat(CL_RG, CL_UNORM_INT16);
            break;

        case DataFormatEnums::Vec2UINT32:
            clFormat = cl::ImageFormat(CL_RG, CL_UNSIGNED_INT32);
            break;

        case DataFormatEnums::Vec2UINT64:
            throw OpenCLFormatException("Unsupported data format: Vec2UINT64", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec3FLOAT16:
            throw OpenCLFormatException("Unsupported data format: Vec3FLOAT16", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec3FLOAT32:
            throw OpenCLFormatException("Unsupported data format: Vec3FLOAT32", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec3FLOAT64:
            throw OpenCLFormatException("Unsupported data format: Vec3FLOAT64", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec3INT8:
            throw OpenCLFormatException("Unsupported data format: Vec3INT8", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec3INT16:
            throw OpenCLFormatException("Unsupported data format: Vec3INT16", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec3INT32:
            throw OpenCLFormatException("Unsupported data format: Vec3INT32", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec3INT64:
            throw OpenCLFormatException("Unsupported data format: Vec3INT64", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec3UINT8:
            throw OpenCLFormatException("Unsupported data format: Vec3UINT8", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec3UINT16:
            throw OpenCLFormatException("Unsupported data format: Vec3UINT16", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec3UINT32:
            throw OpenCLFormatException("Unsupported data format: Vec3UINT32", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec3UINT64:
            throw OpenCLFormatException("Unsupported data format: Vec3UINT64", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec4FLOAT16:
            clFormat = cl::ImageFormat(CL_RGBA, CL_HALF_FLOAT);
            break;

        case DataFormatEnums::Vec4FLOAT32:
            clFormat = cl::ImageFormat(CL_RGBA, CL_FLOAT);
            break;

        case DataFormatEnums::Vec4FLOAT64:
            throw OpenCLFormatException("Unsupported data format: Vec4FLOAT64", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec4INT8:
            clFormat = cl::ImageFormat(CL_RGBA, CL_SNORM_INT8);
            break;

        case DataFormatEnums::Vec4INT16:
            clFormat = cl::ImageFormat(CL_RGBA, CL_SNORM_INT16);
            break;

        case DataFormatEnums::Vec4INT32:
            clFormat = cl::ImageFormat(CL_RGBA, CL_SIGNED_INT32);
            break;

        case DataFormatEnums::Vec4INT64:
            throw OpenCLFormatException("Unsupported data format: Vec4INT64", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::Vec4UINT8:
            clFormat = cl::ImageFormat(CL_RGBA, CL_UNORM_INT8);
            break; // TODO: Find out why CL_UNORM_INT8 does not work

        case DataFormatEnums::Vec4UINT16:
            clFormat = cl::ImageFormat(CL_RGBA, CL_UNORM_INT16);
            break;

        case DataFormatEnums::Vec4UINT32:
            clFormat = cl::ImageFormat(CL_RGBA, CL_UNSIGNED_INT32);
            break;

        case DataFormatEnums::Vec4UINT64:
            throw OpenCLFormatException("Unsupported data format: Vec4UINT64", IvwContextCustom("dataFormatToCLImageFormat"));
            break;

        case DataFormatEnums::NUMBER_OF_FORMATS:
        default:
            // Should not be able to reach here
            throw OpenCLFormatException("cl::ImageFormat typeToImageFormat. Format not implmented yet");
            break;
    }

#ifdef IVW_DEBUG

    if (!inviwo::OpenCL::isValidImageFormat(inviwo::OpenCL::getPtr()->getContext(), clFormat)) {
        LogErrorCustom("cl::ImageFormat typeToImageFormat", "OpenCL device does not support format");
        ivwAssert(inviwo::OpenCL::isValidImageFormat(inviwo::OpenCL::getPtr()->getContext(), clFormat),
                  "cl::ImageFormat typeToImageFormat: OpenCL device does not support format");
    };

#endif
    return clFormat;
}

CLFormats::CLFormat CLFormats::getCLFormat(DataFormatEnums::Id id) const {
    if (CLFormatArray_[static_cast<int>(id)].valid) {
        return CLFormatArray_[static_cast<int>(id)];
    } else {
        std::stringstream error;
        error << "Format not supported by OpenCL. Data type: "
            << CLFormatArray_[static_cast<int>(id)].format.image_channel_data_type
            << " Channel order: "
            << CLFormatArray_[static_cast<int>(id)].format.image_channel_order;
        throw OpenCLFormatException(error.str(), IvwContext);
    }
}

} // namespace

