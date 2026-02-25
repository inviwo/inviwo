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

#pragma once

#include <inviwo/core/util/capabilities.h>
#include <modules/opencl/openclmoduledefine.h>

#include <modules/opencl/cl.hpp>

#include <modules/opencl/glmcl.h>
#include <inviwo/core/util/logcentral.h>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <fstream>

// This file contains macros and functions for printing OpenCL information.

namespace inviwo {

class IVW_MODULE_OPENCL_API OpenCLCapabilities : public Capabilities {

public:
    OpenCLCapabilities();
    virtual ~OpenCLCapabilities();

    virtual void printInfo() override;
    void printDetailedInfo();
    /**
     * Print all available information on device.
     */
    static void printDeviceInfo(const cl::Device& device);

protected:
    virtual void retrieveStaticInfo() override;
    virtual void retrieveDynamicInfo() override;
};

/** \brief Return string representation of  device info and corresponding value
 * @note Added
 * @param info OpenCL device info identifier.
 * @param value
 * @param is_cl_bool Necessary since cl_bool is a cl_uint
 * @return representation of device info and corresponding value
 */
template <typename T>
std::string deviceInfoToString(cl_device_info info, const T& value, bool is_cl_bool) {
    std::ostringstream stream;
    stream << std::boolalpha;  // bool will print true/false

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
            stream << "read only cache: " << static_cast<bool>((value & CL_READ_ONLY_CACHE) != 0)
                   << ", ";
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
            stream << "kernel execution: " << static_cast<bool>((value & CL_EXEC_KERNEL) != 0)
                   << ", ";
            stream << "native kernel execution: "
                   << static_cast<bool>((value & CL_EXEC_NATIVE_KERNEL) != 0);
            break;

        case CL_DEVICE_QUEUE_PROPERTIES:
            stream << "out of order execution: "
                   << static_cast<bool>((value & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) != 0)
                   << ", ";
            stream << "profiling: " << static_cast<bool>((value & CL_QUEUE_PROFILING_ENABLE) != 0);
            break;
#if defined(CL_VERSION_1_1)

        case CL_DEVICE_SINGLE_FP_CONFIG:
        case CL_DEVICE_DOUBLE_FP_CONFIG:
            stream << "denorms: " << static_cast<bool>((value & CL_FP_DENORM) != 0) << ", ";
            stream << "INF and quiet NaN: " << static_cast<bool>((value & CL_FP_INF_NAN) != 0)
                   << ", ";
            stream << "round to nearest: "
                   << static_cast<bool>((value & CL_FP_ROUND_TO_NEAREST) != 0) << ", ";
            stream << "round to zero: " << static_cast<bool>((value & CL_FP_ROUND_TO_ZERO) != 0)
                   << ", ";
            stream << "round to inf: " << static_cast<bool>((value & CL_FP_ROUND_TO_INF) != 0)
                   << ", ";
            stream << "IEEE754-2008 fused multiply-add: "
                   << static_cast<bool>((value & CL_FP_FMA) != 0) << ", ";
            // stream << "IEEE754 correct divide and sqrt round: " << static_cast<bool>(value &
            // CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT!= 0) << ", ";
            stream << "software implementation: "
                   << static_cast<bool>((value & CL_FP_SOFT_FLOAT) != 0);
            break;
#endif
#if defined(CL_VERSION_1_2)

        case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
            stream << "numa: " << static_cast<bool>((value & CL_DEVICE_AFFINITY_DOMAIN_NUMA) != 0)
                   << ", ";
            stream << "L1 cache: "
                   << static_cast<bool>((value & CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE) != 0) << ", ";
            stream << "L2 cache: "
                   << static_cast<bool>((value & CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE) != 0) << ", ";
            stream << "L3 cache: "
                   << static_cast<bool>((value & CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE) != 0) << ", ";
            stream << "L4 cache: "
                   << static_cast<bool>((value & CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE) != 0) << ", ";
            stream << "next partitionable: "
                   << static_cast<bool>((value & CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE) !=
                                        0);
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

template <typename T>
inline std::string deviceInfoToString(cl_device_info info, const std::vector<T>& value, bool) {
    std::ostringstream stream;

    if (info == CL_DEVICE_MAX_WORK_ITEM_SIZES) {
        stream << "(";

        for (size_t i = 0; i < value.size() - 1; ++i) {
            stream << value[i] << ", ";
        }

        stream << "(" << value.back() << ")";
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

            if (i < value.size() - 1) stream << ", ";
        }
    }

#endif
#if defined(USE_CL_DEVICE_FISSION)

    if (info == CL_DEVICE_AFFINITY_DOMAINS_EXT || info == CL_DEVICE_PARTITION_TYPES_EXT ||
        Info == CL_DEVICE_PARTITION_STYLE_EXT) {
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

            if (i < value.size() - 1) stream << ", ";
        }
    }

#endif  // USE_CL_DEVICE_FISSION
    return stream.str();
}

template <>
inline std::string deviceInfoToString<std::string>(cl_device_info, const std::string& value, bool) {
    return value;
}
// Template specialization of these types are necessary since
// they cannot be in a switch statement. Note that neither
// specialization prints anything yet.cl_context;
template <>
inline std::string deviceInfoToString<cl_platform_id>(cl_device_info,
                                                      const cl_platform_id& /*value*/, bool) {
    return "";
}
template <>
inline std::string deviceInfoToString<cl_device_id>(cl_device_info, const cl_device_id& /*value*/,
                                                    bool) {
    return "";
}
template <>
inline std::string deviceInfoToString<cl_command_queue>(cl_device_info,
                                                        const cl_command_queue& /*value*/, bool) {
    return "";
}
template <>
inline std::string deviceInfoToString<cl_program>(cl_device_info, const cl_program& /*value*/,
                                                  bool) {
    return "";
}
template <>
inline std::string deviceInfoToString<cl_kernel>(cl_device_info, const cl_kernel& /*value*/, bool) {
    return "";
}
template <>
inline std::string deviceInfoToString<cl_event>(cl_device_info, const cl_event& /*value*/, bool) {
    return "";
}
template <>
inline std::string deviceInfoToString<cl_sampler>(cl_device_info, const cl_sampler& /*value*/,
                                                  bool) {
    return "";
}

}  // namespace inviwo
