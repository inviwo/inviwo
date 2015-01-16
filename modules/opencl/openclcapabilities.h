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

#ifndef IVW_OPENCLCAPABILITIES_H
#define IVW_OPENCLCAPABILITIES_H


#include <inviwo/core/util/capabilities.h>
#include <modules/opencl/openclmoduledefine.h>
#include <modules/opencl/cl.hpp>
#include <modules/opencl/glmcl.h>
#include <inviwo/core/util/logcentral.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <fstream>


// This file contains macros and functions for printing OpenCL information.

namespace inviwo {

class IVW_MODULE_OPENCL_API OpenCLCapabilities: public Capabilities  {

public:
    OpenCLCapabilities();
    virtual ~OpenCLCapabilities();

    void printInfo();
    void printDetailedInfo();
    /**
     * Print all available information on device.
     */
    static void printDeviceInfo(const cl::Device& device);

protected:
    void retrieveStaticInfo();
    void retrieveDynamicInfo();
};


/** \brief Return string representation of  device info and corresponding value
 * @note Added
 * @param cl_device_info info OpenCL device info identifier.
 * @param const T & value
 * @param bool is_cl_bool Necessary since cl_bool is a cl_uint
 * @return string representation of device info and corresponding value
 */
template< typename T >
std::string deviceInfoToString(cl_device_info info, const T& value, bool is_cl_bool) {
    std::ostringstream stream;
    stream << std::boolalpha; // bool will print true/false

    switch (info) {
        case CL_DEVICE_TYPE:
            switch (value) {
                case CL_DEVICE_TYPE_CPU:
                    stream << "CPU";
                    break;

                case CL_DEVICE_TYPE_GPU:
                    stream << "GPU";
                    break;

                case CL_DEVICE_TYPE_ACCELERATOR:
                    stream << "accelerator";
                    break;
            }

            break;

        case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
            stream << "read only cache: " << static_cast<bool>((value & CL_READ_ONLY_CACHE) != 0) << ", ";
            stream << "read-write cache: " << static_cast<bool>((value & CL_READ_WRITE_CACHE) != 0);
            break;

        case CL_DEVICE_LOCAL_MEM_TYPE:
            switch (value) {
                case CL_LOCAL:
                    stream << "local";
                    break;

                case CL_GLOBAL:
                    stream << "global";
                    break;

                case CL_NONE:
                    stream << "none";
                    break;
            }

            break;

        case CL_DEVICE_EXECUTION_CAPABILITIES:
            stream << "kernel execution: " << static_cast<bool>((value & CL_EXEC_KERNEL) != 0) << ", ";
            stream << "native kernel execution: " << static_cast<bool>((value & CL_EXEC_NATIVE_KERNEL) != 0);
            break;

        case CL_DEVICE_QUEUE_PROPERTIES:
            stream << "out of order execution: " << static_cast<bool>((value & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) != 0) << ", ";
            stream << "profiling: " << static_cast<bool>((value & CL_QUEUE_PROFILING_ENABLE) != 0);
            break;
#if defined(CL_VERSION_1_1)

        case CL_DEVICE_SINGLE_FP_CONFIG:
        case CL_DEVICE_DOUBLE_FP_CONFIG:
            stream << "denorms: " << static_cast<bool>((value & CL_FP_DENORM) != 0) << ", ";
            stream << "INF and quiet NaN: " << static_cast<bool>((value & CL_FP_INF_NAN) != 0) << ", ";
            stream << "round to nearest: " << static_cast<bool>((value & CL_FP_ROUND_TO_NEAREST) != 0) << ", ";
            stream << "round to zero: " << static_cast<bool>((value & CL_FP_ROUND_TO_ZERO) != 0) << ", ";
            stream << "round to inf: " << static_cast<bool>((value & CL_FP_ROUND_TO_INF) != 0) << ", ";
            stream << "IEEE754-2008 fused multiply-add: " << static_cast<bool>((value & CL_FP_FMA) != 0) << ", ";
            //stream << "IEEE754 correct divide and sqrt round: " << static_cast<bool>(value & CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT!= 0) << ", ";
            stream << "software implementation: " << static_cast<bool>((value & CL_FP_SOFT_FLOAT) != 0);
            break;
#endif
#if defined(CL_VERSION_1_2)

        case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
            stream << "numa: " << static_cast<bool>((value & CL_DEVICE_AFFINITY_DOMAIN_NUMA) != 0) << ", ";
            stream << "L1 cache: " << static_cast<bool>((value & CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE) != 0) << ", ";
            stream << "L2 cache: " << static_cast<bool>((value & CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE) != 0) << ", ";
            stream << "L3 cache: " << static_cast<bool>((value & CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE) != 0) << ", ";
            stream << "L4 cache: " << static_cast<bool>((value & CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE) != 0) << ", ";
            stream << "next partitionable: " << static_cast<bool>((value & CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE) != 0);
            break;
#endif

        default:
            if (is_cl_bool) {
                stream << static_cast<bool>(value != 0);
            } else {
                stream << value;
            }
    }

    return stream.str();
}

template<typename T> inline
std::string deviceInfoToString(cl_device_info info, const std::vector<T>& value, bool) {
    std::ostringstream stream;

    if (info == CL_DEVICE_MAX_WORK_ITEM_SIZES) {
        stream << "(";

        for (size_t i = 0; i < value.size()-1; ++i) {
            stream << value[i] << ", ";
        }

        stream << "("<< value.back() << ")";
    }

#if defined(CL_VERSION_1_2)

    if (info == CL_DEVICE_PARTITION_PROPERTIES || info == CL_DEVICE_PARTITION_TYPE) {
        for (size_t i = 0; i < value.size(); ++i) {
            switch (value[i]) {
                case CL_DEVICE_PARTITION_EQUALLY:
                    stream << "partition equally";
                    break;

                case CL_DEVICE_PARTITION_BY_COUNTS:
                    stream << "partition by counts";
                    break;

                case CL_DEVICE_PARTITION_BY_COUNTS_LIST_END:
                    stream << "partition by counts list end";
                    break;

                case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
                    stream << "partition by affinity domain";
                    break;
            }

            if (i < value.size()-1)
                stream << ", ";
        }
    }

#endif
#if defined(USE_CL_DEVICE_FISSION)

    if (info == CL_DEVICE_AFFINITY_DOMAINS_EXT || info == CL_DEVICE_PARTITION_TYPES_EXT || Info == CL_DEVICE_PARTITION_STYLE_EXT) {
        for (size_t i = 0; i < value.size(); ++i) {
            switch (value[i]) {
                case CL_DEVICE_PARTITION_EQUALLY_EXT:
                    stream << "partition equally";
                    break;

                case CL_DEVICE_PARTITION_BY_COUNTS_EXT:
                    stream << "partition by counts";
                    break;

                case CL_DEVICE_PARTITION_BY_NAMES_EXT:
                    stream << "partition by names";
                    break;

                case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT:
                    stream << "partition by affinity domain";
                    break;

                case CL_PROPERTIES_LIST_END_EXT:
                    stream << "list end";
                    break;

                case CL_PARTITION_BY_COUNTS_LIST_END_EXT:
                    stream << "partition by counts list end";
                    break;

                case CL_PARTITION_BY_NAMES_LIST_END_EXT:
                    stream << "partition by names list end";
                    break;
            }

            if (i < value.size()-1)
                stream << ", ";
        }
    }

#endif // USE_CL_DEVICE_FISSION
    return stream.str();
}


template<> inline
std::string deviceInfoToString<std::string>(cl_device_info, const std::string& value, bool) {
    return value;
}
// Template specialization of these types are necessary since
// they cannot be in a switch statement. Note that neither
// specialization prints anything yet.cl_context;
template<> inline
std::string deviceInfoToString<cl_platform_id>(cl_device_info, const cl_platform_id& value, bool) {  return ""; }
template<> inline
std::string deviceInfoToString<cl_device_id>(cl_device_info, const cl_device_id& value, bool) {  return ""; }
template<> inline
std::string deviceInfoToString<cl_command_queue>(cl_device_info, const cl_command_queue& value, bool) {  return ""; }
template<> inline
std::string deviceInfoToString<cl_program>(cl_device_info, const cl_program& value, bool) {  return ""; }
template<> inline
std::string deviceInfoToString<cl_kernel>(cl_device_info, const cl_kernel& value, bool) {  return ""; }
template<> inline
std::string deviceInfoToString<cl_event>(cl_device_info, const cl_event& value, bool) {  return ""; }
template<> inline
std::string deviceInfoToString<cl_sampler>(cl_device_info, const cl_sampler& value, bool) {  return ""; }

} // end namespace

// Copied macros __PARAM_NAME_INFO_X_X from cl.hpp
// but modified to only contain cl_device_info.
// Future versions just need to add the macro,
// make sure that template function deviceInfoToString
// is updated, and call the macro in OpenCL::printDeviceInfo(const cl::Device& device)
#define STRING_CLASS std::string

#define __DEVICE_INFO_GENERAL(F) \
    F(cl_device_info, CL_DEVICE_TYPE, cl_device_type) \
    F(cl_device_info, CL_DEVICE_NAME, STRING_CLASS) \
    F(cl_device_info, CL_DEVICE_VENDOR, STRING_CLASS) \
    F(cl_device_info, CL_DRIVER_VERSION, STRING_CLASS) \
    F(cl_device_info, CL_DEVICE_PROFILE, STRING_CLASS) \
    F(cl_device_info, CL_DEVICE_VERSION, STRING_CLASS)

#define __DEVICE_INFO_1_0(F) \
    F(cl_device_info, CL_DEVICE_EXTENSIONS, STRING_CLASS) \
    F(cl_device_info, CL_DEVICE_MAX_COMPUTE_UNITS, cl_uint) \
    F(cl_device_info, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, cl_uint) \
    F(cl_device_info, CL_DEVICE_MAX_WORK_GROUP_SIZE, ::size_t) \
    F(cl_device_info, CL_DEVICE_MAX_WORK_ITEM_SIZES, VECTOR_CLASS< ::size_t>) \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, cl_uint) \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, cl_uint) \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, cl_uint) \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, cl_uint) \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, cl_uint) \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, cl_uint) \
    F(cl_device_info, CL_DEVICE_MAX_CLOCK_FREQUENCY, cl_uint) \
    F(cl_device_info, CL_DEVICE_ADDRESS_BITS, cl_uint) \
    F(cl_device_info, CL_DEVICE_MAX_READ_IMAGE_ARGS, cl_uint) \
    F(cl_device_info, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, cl_uint) \
    F(cl_device_info, CL_DEVICE_MAX_MEM_ALLOC_SIZE, cl_ulong) \
    F(cl_device_info, CL_DEVICE_IMAGE2D_MAX_WIDTH, ::size_t) \
    F(cl_device_info, CL_DEVICE_IMAGE2D_MAX_HEIGHT, ::size_t) \
    F(cl_device_info, CL_DEVICE_IMAGE3D_MAX_WIDTH, ::size_t) \
    F(cl_device_info, CL_DEVICE_IMAGE3D_MAX_HEIGHT, ::size_t) \
    F(cl_device_info, CL_DEVICE_IMAGE3D_MAX_DEPTH, ::size_t) \
    F(cl_device_info, CL_DEVICE_IMAGE_SUPPORT, cl_bool) \
    F(cl_device_info, CL_DEVICE_MAX_PARAMETER_SIZE, ::size_t) \
    F(cl_device_info, CL_DEVICE_MAX_SAMPLERS, cl_uint) \
    F(cl_device_info, CL_DEVICE_MEM_BASE_ADDR_ALIGN, cl_uint) \
    F(cl_device_info, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, cl_uint) \
    F(cl_device_info, CL_DEVICE_SINGLE_FP_CONFIG, cl_device_fp_config) \
    F(cl_device_info, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, cl_device_mem_cache_type) \
    F(cl_device_info, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, cl_uint)\
    F(cl_device_info, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, cl_ulong) \
    F(cl_device_info, CL_DEVICE_GLOBAL_MEM_SIZE, cl_ulong) \
    F(cl_device_info, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, cl_ulong) \
    F(cl_device_info, CL_DEVICE_MAX_CONSTANT_ARGS, cl_uint) \
    F(cl_device_info, CL_DEVICE_LOCAL_MEM_TYPE, cl_device_local_mem_type) \
    F(cl_device_info, CL_DEVICE_LOCAL_MEM_SIZE, cl_ulong) \
    F(cl_device_info, CL_DEVICE_ERROR_CORRECTION_SUPPORT, cl_bool) \
    F(cl_device_info, CL_DEVICE_PROFILING_TIMER_RESOLUTION, ::size_t) \
    F(cl_device_info, CL_DEVICE_ENDIAN_LITTLE, cl_bool) \
    F(cl_device_info, CL_DEVICE_AVAILABLE, cl_bool) \
    F(cl_device_info, CL_DEVICE_COMPILER_AVAILABLE, cl_bool) \
    F(cl_device_info, CL_DEVICE_EXECUTION_CAPABILITIES, cl_device_exec_capabilities) \
    F(cl_device_info, CL_DEVICE_QUEUE_PROPERTIES, cl_command_queue_properties) \
    F(cl_device_info, CL_DEVICE_PLATFORM, cl_platform_id)

#if defined(CL_VERSION_1_1)
#define __DEVICE_INFO_1_1(F) \
    F(cl_device_info, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, cl_uint) \
    F(cl_device_info, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, cl_uint) \
    F(cl_device_info, CL_DEVICE_DOUBLE_FP_CONFIG, cl_device_fp_config) \
    F(cl_device_info, CL_DEVICE_HALF_FP_CONFIG, cl_device_fp_config) \
    F(cl_device_info, CL_DEVICE_HOST_UNIFIED_MEMORY, cl_bool) \
    F(cl_device_info, CL_DEVICE_OPENCL_C_VERSION, STRING_CLASS)
#endif
#if defined(CL_VERSION_1_2)
#define __DEVICE_INFO_1_2(F) \
    F(cl_device_info, CL_DEVICE_PARENT_DEVICE, cl_device_id) \
    F(cl_device_info, CL_DEVICE_PARTITION_PROPERTIES, VECTOR_CLASS<cl_device_partition_property>) \
    F(cl_device_info, CL_DEVICE_PARTITION_TYPE, VECTOR_CLASS<cl_device_partition_property>)  \
    F(cl_device_info, CL_DEVICE_REFERENCE_COUNT, cl_uint) \
    F(cl_device_info, CL_DEVICE_PREFERRED_INTEROP_USER_SYNC, cl_bool) \
    F(cl_device_info, CL_DEVICE_PARTITION_AFFINITY_DOMAIN, cl_device_affinity_domain) \
    F(cl_device_info, CL_DEVICE_BUILT_IN_KERNELS, STRING_CLASS)
#endif // #if defined(CL_VERSION_1_2)

#define __CL_PRINT_DEVICE_INFO(token, param_name, T) \
    try { \
        if(std::string(#T).compare("cl_bool") == 0) { \
            LogInfoCustom("OpenCLInfo", #param_name << ": " << inviwo::deviceInfoToString(param_name, device.getInfo<param_name>(), true)) \
        } else {                                        \
            LogInfoCustom("OpenCLInfo", #param_name << ": " << inviwo::deviceInfoToString(param_name, device.getInfo<param_name>(), false)) \
        } \
    } catch(cl::Error&) { \
        LogInfoCustom("OpenCLInfo", "Device info missing: " << #param_name); \
    }

#endif

