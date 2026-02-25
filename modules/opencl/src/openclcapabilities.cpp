/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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

#include <modules/opencl/openclcapabilities.h>
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
            type = "CL_SNORM_INT8";
            break;

        case CL_SNORM_INT16:
            type = "CL_SNORM_INT16";
            break;

        case CL_UNORM_INT8:
            type = "CL_UNORM_INT8";
            break;

        case CL_UNORM_INT16:
            type = "CL_UNORM_INT16";
            break;

        case CL_UNORM_SHORT_565:
            type = "CL_UNORM_SHORT_565";
            break;

        case CL_UNORM_SHORT_555:
            type = "CL_UNORM_SHORT_555";
            break;

        case CL_UNORM_INT_101010:
            type = "CL_UNORM_INT_101010";
            break;

        case CL_SIGNED_INT8:
            type = "CL_SIGNED_INT8";
            break;

        case CL_SIGNED_INT16:
            type = "CL_SIGNED_INT16";
            break;

        case CL_SIGNED_INT32:
            type = "CL_SIGNED_INT32";
            break;

        case CL_UNSIGNED_INT8:
            type = "CL_UNSIGNED_INT8";
            break;

        case CL_UNSIGNED_INT16:
            type = "CL_UNSIGNED_INT16";
            break;

        case CL_UNSIGNED_INT32:
            type = "CL_UNSIGNED_INT32";
            break;

        case CL_HALF_FLOAT:
            type = "CL_HALF_FLOAT";
            break;

        case CL_FLOAT:
            type = "CL_FLOAT";
            break;

        default:
            type = "unkown data type";
            break;
    }

    std::ostringstream stream;
    stream << "(" << order << ", " << type << ")";
    return stream.str();
}

OpenCLCapabilities::OpenCLCapabilities() {}

OpenCLCapabilities::~OpenCLCapabilities() {}

void OpenCLCapabilities::retrieveStaticInfo() {}

void OpenCLCapabilities::retrieveDynamicInfo() {}

void OpenCLCapabilities::printDetailedInfo() {
    OpenCLCapabilities::printDeviceInfo(OpenCL::getPtr()->getDevice());
    try {
        // Supported image 2D formats
        std::vector<cl::ImageFormat> formats;
        OpenCL::getPtr()->getContext().getSupportedImageFormats(CL_MEM_READ_WRITE,
                                                                CL_MEM_OBJECT_IMAGE2D, &formats);
        {
            std::ostringstream stream;
            stream << "Supported 2D READ_WRITE formats: ";
            for (::size_t i = 0; i < formats.size(); ++i) {
                stream << imageFormatToString(formats[i]);
                if (i != formats.size() - 1) {
                    stream << ", ";
                }
            }
            stream << std::endl;
            log::report(LogLevel::Info, stream.str());
        }
        formats.clear();
        {
            std::ostringstream stream;

            OpenCL::getPtr()->getContext().getSupportedImageFormats(
                CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE3D, &formats);
            stream << "Supported 3D READ_WRITE formats: ";
            for (::size_t i = 0; i < formats.size(); ++i) {
                stream << imageFormatToString(formats[i]);
                if (i != formats.size() - 1) {
                    stream << ", ";
                }
            }
            log::report(LogLevel::Info, stream.str());
        }
    } catch (cl::Error& e) {
        log::info("Device does not have the following info: {}", e.what());
    }
}
}  // namespace inviwo

// Copied macros __PARAM_NAME_INFO_X_X from cl.hpp
// but modified to only contain cl_device_info.
// Future versions just need to add the macro,
// make sure that template function deviceInfoToString
// is updated, and call the macro in OpenCL::printDeviceInfo(const cl::Device& device)
#define STRING_CLASS std::string

#define IVW_DEVICE_INFO_GENERAL(F)                     \
    F(cl_device_info, CL_DEVICE_TYPE, cl_device_type)  \
    F(cl_device_info, CL_DEVICE_NAME, STRING_CLASS)    \
    F(cl_device_info, CL_DEVICE_VENDOR, STRING_CLASS)  \
    F(cl_device_info, CL_DRIVER_VERSION, STRING_CLASS) \
    F(cl_device_info, CL_DEVICE_PROFILE, STRING_CLASS) \
    F(cl_device_info, CL_DEVICE_VERSION, STRING_CLASS)

#define IVW_DEVICE_INFO_1_0(F)                                                       \
    F(cl_device_info, CL_DEVICE_EXTENSIONS, STRING_CLASS)                            \
    F(cl_device_info, CL_DEVICE_MAX_COMPUTE_UNITS, cl_uint)                          \
    F(cl_device_info, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, cl_uint)                   \
    F(cl_device_info, CL_DEVICE_MAX_WORK_GROUP_SIZE, ::size_t)                       \
    F(cl_device_info, CL_DEVICE_MAX_WORK_ITEM_SIZES, VECTOR_CLASS<::size_t>)         \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, cl_uint)                \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, cl_uint)               \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, cl_uint)                 \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, cl_uint)                \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, cl_uint)               \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, cl_uint)              \
    F(cl_device_info, CL_DEVICE_MAX_CLOCK_FREQUENCY, cl_uint)                        \
    F(cl_device_info, CL_DEVICE_ADDRESS_BITS, cl_uint)                               \
    F(cl_device_info, CL_DEVICE_MAX_READ_IMAGE_ARGS, cl_uint)                        \
    F(cl_device_info, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, cl_uint)                       \
    F(cl_device_info, CL_DEVICE_MAX_MEM_ALLOC_SIZE, cl_ulong)                        \
    F(cl_device_info, CL_DEVICE_IMAGE2D_MAX_WIDTH, ::size_t)                         \
    F(cl_device_info, CL_DEVICE_IMAGE2D_MAX_HEIGHT, ::size_t)                        \
    F(cl_device_info, CL_DEVICE_IMAGE3D_MAX_WIDTH, ::size_t)                         \
    F(cl_device_info, CL_DEVICE_IMAGE3D_MAX_HEIGHT, ::size_t)                        \
    F(cl_device_info, CL_DEVICE_IMAGE3D_MAX_DEPTH, ::size_t)                         \
    F(cl_device_info, CL_DEVICE_IMAGE_SUPPORT, cl_bool)                              \
    F(cl_device_info, CL_DEVICE_MAX_PARAMETER_SIZE, ::size_t)                        \
    F(cl_device_info, CL_DEVICE_MAX_SAMPLERS, cl_uint)                               \
    F(cl_device_info, CL_DEVICE_MEM_BASE_ADDR_ALIGN, cl_uint)                        \
    F(cl_device_info, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, cl_uint)                   \
    F(cl_device_info, CL_DEVICE_SINGLE_FP_CONFIG, cl_device_fp_config)               \
    F(cl_device_info, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, cl_device_mem_cache_type)     \
    F(cl_device_info, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, cl_uint)                  \
    F(cl_device_info, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, cl_ulong)                     \
    F(cl_device_info, CL_DEVICE_GLOBAL_MEM_SIZE, cl_ulong)                           \
    F(cl_device_info, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, cl_ulong)                  \
    F(cl_device_info, CL_DEVICE_MAX_CONSTANT_ARGS, cl_uint)                          \
    F(cl_device_info, CL_DEVICE_LOCAL_MEM_TYPE, cl_device_local_mem_type)            \
    F(cl_device_info, CL_DEVICE_LOCAL_MEM_SIZE, cl_ulong)                            \
    F(cl_device_info, CL_DEVICE_ERROR_CORRECTION_SUPPORT, cl_bool)                   \
    F(cl_device_info, CL_DEVICE_PROFILING_TIMER_RESOLUTION, ::size_t)                \
    F(cl_device_info, CL_DEVICE_ENDIAN_LITTLE, cl_bool)                              \
    F(cl_device_info, CL_DEVICE_AVAILABLE, cl_bool)                                  \
    F(cl_device_info, CL_DEVICE_COMPILER_AVAILABLE, cl_bool)                         \
    F(cl_device_info, CL_DEVICE_EXECUTION_CAPABILITIES, cl_device_exec_capabilities) \
    F(cl_device_info, CL_DEVICE_QUEUE_PROPERTIES, cl_command_queue_properties)       \
    F(cl_device_info, CL_DEVICE_PLATFORM, cl_platform_id)

#if defined(CL_VERSION_1_1)
#define IVW_DEVICE_INFO_1_1(F)                                         \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, cl_uint)  \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, cl_uint)     \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, cl_uint)    \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, cl_uint)      \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, cl_uint)     \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, cl_uint)    \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, cl_uint)   \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, cl_uint)     \
    F(cl_device_info, CL_DEVICE_DOUBLE_FP_CONFIG, cl_device_fp_config) \
    F(cl_device_info, CL_DEVICE_HALF_FP_CONFIG, cl_device_fp_config)   \
    F(cl_device_info, CL_DEVICE_HOST_UNIFIED_MEMORY, cl_bool)          \
    F(cl_device_info, CL_DEVICE_OPENCL_C_VERSION, STRING_CLASS)
#endif
#if defined(CL_VERSION_1_2)
#define IVW_DEVICE_INFO_1_2(F)                                                                    \
    F(cl_device_info, CL_DEVICE_PARENT_DEVICE, cl_device_id)                                      \
    F(cl_device_info, CL_DEVICE_PARTITION_PROPERTIES, VECTOR_CLASS<cl_device_partition_property>) \
    F(cl_device_info, CL_DEVICE_PARTITION_TYPE, VECTOR_CLASS<cl_device_partition_property>)       \
    F(cl_device_info, CL_DEVICE_REFERENCE_COUNT, cl_uint)                                         \
    F(cl_device_info, CL_DEVICE_PREFERRED_INTEROP_USER_SYNC, cl_bool)                             \
    F(cl_device_info, CL_DEVICE_PARTITION_AFFINITY_DOMAIN, cl_device_affinity_domain)             \
    F(cl_device_info, CL_DEVICE_BUILT_IN_KERNELS, STRING_CLASS)
#endif  // #if defined(CL_VERSION_1_2)

#define IVW_CL_PRINT_DEVICE_INFO(token, param_name, T)                                             \
    try {                                                                                          \
        if (std::string(#T).compare("cl_bool") == 0) {                                             \
            log::info("{} : {}", #param_name,                                                      \
                      inviwo::deviceInfoToString(param_name, device.getInfo<param_name>(), true)); \
        } else {                                                                                   \
            log::info(                                                                             \
                "{} : {}", #param_name,                                                            \
                inviwo::deviceInfoToString(param_name, device.getInfo<param_name>(), false));      \
        }                                                                                          \
    } catch (cl::Error&) {                                                                         \
        log::info("Device info missing: {}", #param_name);                                         \
    }

void inviwo::OpenCLCapabilities::printInfo() {
    const cl::Device& device = OpenCL::getPtr()->getDevice();
    try {
        IVW_DEVICE_INFO_GENERAL(IVW_CL_PRINT_DEVICE_INFO)
    } catch (cl::Error& e) {
        log::error("Device does not have the following info: {}", e.what());
    }
}

void inviwo::OpenCLCapabilities::printDeviceInfo(const cl::Device& device) {
    try {
        // Macros will print print supported device info
        IVW_DEVICE_INFO_GENERAL(IVW_CL_PRINT_DEVICE_INFO)
        IVW_DEVICE_INFO_1_0(IVW_CL_PRINT_DEVICE_INFO)
#if defined(IVW_DEVICE_INFO_1_1)
        IVW_DEVICE_INFO_1_1(IVW_CL_PRINT_DEVICE_INFO)
#endif
#if defined(IVW_DEVICE_INFO_1_2)
        IVW_DEVICE_INFO_1_2(IVW_CL_PRINT_DEVICE_INFO)
#endif
#if defined(USE_CL_DEVICE_FISSION)
        __PARAM_NAME_DEVICE_FISSION(IVW_CL_PRINT_DEVICE_INFO)
#endif
    } catch (cl::Error& e) {
        log::info("Device does not have the following info: {}", e.what());
    }
}
