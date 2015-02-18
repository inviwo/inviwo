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

#include "openclcapabilities.h"
#include <inviwo/core/util/logcentral.h>
#include <modules/opencl/inviwoopencl.h>

namespace inviwo {

std::string imageFormatToString(const cl::ImageFormat& format) {
    std::string order;

    switch (format.image_channel_order) {
        case CL_R:
            order = "CL_R";
            break;

        case CL_A:
            order = "CL_A";
            break;

        case CL_RG:
            order = "CL_RG";
            break;

        case CL_RA:
            order = "CL_RA";
            break;

        case CL_RGB:
            order = "CL_RGB";
            break;

        case CL_RGBA:
            order = "CL_RGBA";
            break;

        case CL_BGRA:
            order = "CL_BGRA";
            break;

        case CL_ARGB:
            order = "CL_ARGB";
            break;

        case CL_INTENSITY:
            order = "CL_INTENSITY";
            break;

        case CL_LUMINANCE:
            order = "CL_LUMINANCE";
            break;
#ifdef CL_Rx

        case CL_Rx:
            order = "CL_Rx";
            break;

        case CL_RGx:
            order = "CL_RGx";
            break;

        case CL_RGBx:
            order = "CL_RGBx";
            break;
#endif

        default:
            order = "unkown channel order";
            break;
    }

    std::string type;

    switch (format.image_channel_data_type) {
        case CL_SNORM_INT8:
            type =          "CL_SNORM_INT8";
            break;

        case CL_SNORM_INT16:
            type =         "CL_SNORM_INT16";
            break;

        case CL_UNORM_INT8:
            type =          "CL_UNORM_INT8";
            break;

        case CL_UNORM_INT16:
            type =         "CL_UNORM_INT16";
            break;

        case CL_UNORM_SHORT_565:
            type =     "CL_UNORM_SHORT_565";
            break;

        case CL_UNORM_SHORT_555:
            type =     "CL_UNORM_SHORT_555";
            break;

        case CL_UNORM_INT_101010:
            type =    "CL_UNORM_INT_101010";
            break;

        case CL_SIGNED_INT8:
            type =         "CL_SIGNED_INT8";
            break;

        case CL_SIGNED_INT16:
            type =        "CL_SIGNED_INT16";
            break;

        case CL_SIGNED_INT32:
            type =        "CL_SIGNED_INT32";
            break;

        case CL_UNSIGNED_INT8:
            type =       "CL_UNSIGNED_INT8";
            break;

        case CL_UNSIGNED_INT16:
            type =      "CL_UNSIGNED_INT16";
            break;

        case CL_UNSIGNED_INT32:
            type =      "CL_UNSIGNED_INT32";
            break;

        case CL_HALF_FLOAT:
            type =          "CL_HALF_FLOAT";
            break;

        case CL_FLOAT:
            type =               "CL_FLOAT";
            break;

        default:
            type =                     "unkown data type";
            break;
    }

    std::ostringstream stream;
    stream << "(" << order << ", " << type << ")";
    return stream.str();
}

OpenCLCapabilities::OpenCLCapabilities() {}

OpenCLCapabilities::~OpenCLCapabilities() {}

void OpenCLCapabilities::retrieveStaticInfo() {
}

void OpenCLCapabilities::retrieveDynamicInfo() {
}

void OpenCLCapabilities::printInfo() {
    const cl::Device& device = OpenCL::getPtr()->getDevice();
    try
    {
    __DEVICE_INFO_GENERAL(__CL_PRINT_DEVICE_INFO)
    }
    catch (cl::Error& e)
    {
        LogInfoCustom("OpenCL", "Device does not have the following info: " << e.what());
    }
}

void OpenCLCapabilities::printDetailedInfo() {
    OpenCLCapabilities::printDeviceInfo(OpenCL::getPtr()->getDevice());
    try 
    {
        // Supported image 2D formats
        std::vector<cl::ImageFormat> formats;
        OpenCL::getPtr()->getContext().getSupportedImageFormats(CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE2D, &formats);
        {
            std::ostringstream stream;
            stream << "Supported 2D READ_WRITE formats: ";
            for(::size_t i = 0; i < formats.size(); ++i) {
                stream << imageFormatToString(formats[i]);
                if (i != formats.size()-1) {
                    stream << ", ";
                }
            }
            stream << std::endl;
            LogInfo(stream.str())
        }
        formats.clear();
        {
            std::ostringstream stream;

            OpenCL::getPtr()->getContext().getSupportedImageFormats(CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE3D, &formats);
            stream << "Supported 3D READ_WRITE formats: ";
            for(::size_t i = 0; i < formats.size(); ++i) {
                stream << imageFormatToString(formats[i]);
                if (i != formats.size()-1) {
                    stream << ", ";
                }
            }
            LogInfo(stream.str())
        }
    } catch (cl::Error& e)
    {
        LogInfoCustom("OpenCL", "Device does not have the following info: " << e.what());
    }
}

void OpenCLCapabilities::printDeviceInfo(const cl::Device& device) {
    try
    {
        // Macros will print print supported device info
        __DEVICE_INFO_GENERAL(__CL_PRINT_DEVICE_INFO)
        __DEVICE_INFO_1_0(__CL_PRINT_DEVICE_INFO)
#if defined(__DEVICE_INFO_1_1)
        __DEVICE_INFO_1_1(__CL_PRINT_DEVICE_INFO)
#endif
#if defined(__DEVICE_INFO_1_2)
        __DEVICE_INFO_1_2(__CL_PRINT_DEVICE_INFO)
#endif
#if defined(USE_CL_DEVICE_FISSION)
        __PARAM_NAME_DEVICE_FISSION(__CL_PRINT_DEVICE_INFO)
#endif
    }
    catch (cl::Error& e)
    {
        LogInfoCustom("OpenCL", "Device does not have the following info: " << e.what());
    }
}



} // namespace

