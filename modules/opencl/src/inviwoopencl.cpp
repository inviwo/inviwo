/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/cl.hpp>
#include <modules/opencl/glmcl.h>
#include <modules/opencl/openclcapabilities.h>
#include <modules/opencl/openclexception.h>
#include <modules/opencl/openclformatexception.h>
#include <modules/opencl/syncclgl.h>
#include <inviwo/core/io/textfilereader.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/filesystem.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <fstream>
#if !(WIN32 || __APPLE__)  // LINUX
#include <GL/glx.h>        // glXCurrentContext()
#endif

#include <stdlib.h>
#include <stdio.h>

namespace cl {}

namespace inviwo {

OpenCL* OpenCL::instance_ = nullptr;

OpenCL::OpenCL() { initialize(true); }

void OpenCL::initialize(bool glSharing) {
    try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        if (platforms.size() == 0) {
            throw OpenCLException("No OpenCL platforms found", IVW_CONTEXT);
        }

        cl::Platform platform;
        cl::Device device;
        getBestGPUDeviceOnSystem(device, platform);
        setDevice(device, glSharing);
    } catch (cl::Error& err) {
        throw OpenCLException("Failed to initialize OpenCL. " + std::string(err.what()) + "(" +
                              toString(err.err()) + "), " + errorCodeToString(err.err()));
    }
}
bool OpenCL::isValidImageFormat(const cl::Context& context, const cl::ImageFormat& format) {
    std::vector<cl::ImageFormat> formats;
    context.getSupportedImageFormats(CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE2D, &formats);

    for (::size_t i = 0; i < formats.size(); ++i) {
        if (formats[i].image_channel_order == format.image_channel_order &&
            formats[i].image_channel_data_type == format.image_channel_data_type) {
            return true;
        }
    }

    return false;
}
bool OpenCL::isValidVolumeFormat(const cl::Context& context, const cl::ImageFormat& format) {
    std::vector<cl::ImageFormat> formats;
    context.getSupportedImageFormats(CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE3D, &formats);

    for (::size_t i = 0; i < formats.size(); ++i) {
        if (formats[i].image_channel_order == format.image_channel_order &&
            formats[i].image_channel_data_type == format.image_channel_data_type) {
            return true;
        }
    }

    return false;
}

bool OpenCL::isOpenGLSharingEnabled() const {
    auto contextProperties = gpuContext_.getInfo<CL_CONTEXT_PROPERTIES>();
    return (std::find_if(contextProperties.begin(), contextProperties.end(),
                         [](const cl_context_properties& p) {
#if __APPLE__
                             return p == CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE;
#else
                             return p == CL_GL_CONTEXT_KHR;
#endif
                         })) != contextProperties.end();
}

bool OpenCL::getBestGPUDeviceOnSystem(cl::Device& bestDevice, cl::Platform& onPlatform) {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    auto glVendor = std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));

    // Store found devices in a local struct to only have to query device info once
    struct Device {
        Device(cl::Device device) : device(device) {
            cl_int err;
            vendor = device.getInfo<CL_DEVICE_VENDOR>(&err);
            if (err != CL_SUCCESS) throw cl::Error(err, "Failed to get the device vendor");
            device_type = device.getInfo<CL_DEVICE_TYPE>(&err);
            if (err != CL_SUCCESS) throw cl::Error(err, "Failed to get the device type");
            max_compute_units = device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(&err);
            if (err != CL_SUCCESS)
                throw cl::Error(err, "Failed to get the device max number of compute units ");
        }
        cl::Device device;
        std::string vendor;
        cl_device_type device_type;
        cl_uint max_compute_units;
    };

    std::vector<Device> devices;
    for (auto& platform : platforms) {
        try {
            std::vector<cl::Device> devicesTmp;
            platform.getDevices(CL_DEVICE_TYPE_ALL, &devicesTmp);
            for (auto& d : devicesTmp) {
                try {
                    devices.emplace_back(d);
                } catch (cl::Error& e) {
                    // Error getting device info, continue with other devices
                    LogWarnCustom(
                        "InviwoOpenCL",
                        "Failed to get device info, skipping this device: (" << e.what() << ")");
                }
            }

        } catch (cl::Error&) {
            // Error getting devices, continue with next platform
        }
    }

    if (devices.empty()) return false;
    std::sort(devices.begin(), devices.end(), [&](Device& a, Device& b) {
        if (a.vendor == glVendor && b.vendor != glVendor) return true;
        if (a.vendor != glVendor && b.vendor == glVendor) return false;

        if (a.device_type != b.device_type) {
            if (a.device_type == CL_DEVICE_TYPE_GPU)
                return true;
            else if (b.device_type == CL_DEVICE_TYPE_GPU)
                return false;
        }

        return a.max_compute_units > b.max_compute_units;
    });

    bestDevice = devices.front().device;
    onPlatform = bestDevice.getInfo<CL_DEVICE_PLATFORM>();
    return true;
}

void OpenCL::printBuildError(const std::vector<cl::Device>& devices, const cl::Program& program,
                             const std::string& filename) {
    for (::size_t i = 0; i < devices.size(); ++i) {
        cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(devices[i]);

        // Houston, we have a problem
        if (status == CL_BUILD_ERROR) {
            std::string buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[i]);
            LogErrorCustom("OpenCL", filename << " build error:" << std::endl << buildLog);
        }
    }
}
void OpenCL::printBuildError(const cl::Device& device, const cl::Program& program,
                             const std::string& filename) {
    printBuildError(std::vector<cl::Device>(1, device), program, filename);
}

std::vector<cl_context_properties> OpenCL::getGLSharingContextProperties() {
    std::vector<cl_context_properties> sharingProperties;
#if WIN32
    // Obtain a handle to the current OpenGL rendering context of the calling thread
    auto glContext = (cl_context_properties)wglGetCurrentContext();
    // Obtains a handle to the device context that is associated with the current OpenGL rendering
    // context of the calling thread
    auto deviceContext = wglGetCurrentDC();
    // Make sure that the calling thread has an OpenGL rendering context
    if (glContext && deviceContext) {
        sharingProperties = {CL_GL_CONTEXT_KHR, (cl_context_properties)glContext, CL_WGL_HDC_KHR,
                             (cl_context_properties)deviceContext};
    }
#elif __APPLE__
    // Obtain a handle to the current OpenGL rendering context of the calling thread
    auto glContext = CGLGetCurrentContext();
    // Get the OpenCL share group for the current context
    auto shareGroup = CGLGetShareGroup(glContext);
    // Make sure that the calling thread has an OpenGL rendering context
    if (glContext && shareGroup) {
        sharingProperties = {CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
                             (cl_context_properties)shareGroup};
    }
#else  // LINUX
    // Obtain a handle to the current OpenGL rendering context of the calling thread
    auto glContext = glXGetCurrentContext();
    // Get the display for the current context
    auto display = glXGetCurrentDisplay();
    // Make sure that the calling thread has an OpenGL rendering context
    if (glContext && display) {
        sharingProperties = {CL_GL_CONTEXT_KHR, (cl_context_properties)glContext,
                             CL_GLX_DISPLAY_KHR, (cl_context_properties)display};
    }
#endif
    return sharingProperties;
}

std::vector<cl::Device> OpenCL::getAllDevices() {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    // Retrieve all devices
    std::vector<cl::Device> allDevices;

    for (::size_t i = 0; i < platforms.size(); ++i) {
        // Use a set here to get unique devices.
        // Some weird bug on NVIDIA cards causes duplicate platforms
        // to be returned (when reinstalling CUDA 5.5...)
        bool foundDuplicate = false;
        for (::size_t j = i + 1; j < platforms.size(); j++) {
            if (platforms[i]() == platforms[j]()) {
                foundDuplicate = true;
                break;
            }
        }
        if (foundDuplicate) {
            continue;
        }
        std::vector<cl::Device> devices;

        try {
            platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &devices);
            allDevices.insert(allDevices.end(), devices.begin(), devices.end());
        } catch (cl::Error&) {
            // Error getting device, continue with others
        }
    }

    return allDevices;
}

cl::Program OpenCL::buildProgram(const std::string& fileName, const std::string& header,
                                 const std::string& defines, const cl::CommandQueue& queue) {
    cl::Context context = queue.getInfo<CL_QUEUE_CONTEXT>();
    cl::Device device = queue.getInfo<CL_QUEUE_DEVICE>();
    // build the program from the source in the file
    std::ifstream file(fileName.c_str());
    TextFileReader fileReader(fileName);
    std::string prog;

    try {
        prog = fileReader.read();
    } catch (std::ifstream::failure&) {
    }
    if (prog.size() > 3 && !header.empty()) {
        size_t offset = 0;
        // https://en.wikipedia.org/wiki/Byte_order_mark
        std::array<char, 3> utf8 = {static_cast<char>(0xEFu), static_cast<char>(0xBBu),
                                    static_cast<char>(0xBFu)};
        std::array<char, 2> utf16BE = {static_cast<char>(0xFEu), static_cast<char>(0xFFu)};
        std::array<char, 2> utf16LE = {static_cast<char>(0xFEu), static_cast<char>(0xFEu)};
        std::array<char, 4> utf32BE = {0x00, 0x00, static_cast<char>(0xFEu),
                                       static_cast<char>(0xFFu)};
        std::array<char, 4> utf32LE = {static_cast<char>(0xFFu), static_cast<char>(0xFEu), 0x00,
                                       0x00};

        if (std::equal(std::begin(utf8), std::end(utf8), std::begin(prog)))
            offset = utf8.size();
        else if (std::equal(std::begin(utf16BE), std::end(utf16BE), std::begin(prog)))
            offset = utf16BE.size();
        else if (std::equal(std::begin(utf16LE), std::end(utf16LE), std::begin(prog)))
            offset = utf16LE.size();
        else if (std::equal(std::begin(utf16LE), std::end(utf16LE), std::begin(prog)))
            offset = utf16LE.size();
        else if (std::equal(std::begin(utf32BE), std::end(utf32BE), std::begin(prog)))
            offset = utf32BE.size();
        else if (std::equal(std::begin(utf32LE), std::end(utf32LE), std::begin(prog)))
            offset = utf32LE.size();

        prog.insert(offset, header);
    }
    std::string concatenatedDefines = OpenCL::getPtr()->getIncludeDefine() + defines;
    cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length() + 1));
    cl::Program program(context, source);

    try {
        program.build(std::vector<cl::Device>(1, device), concatenatedDefines.c_str());
        std::string buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);

        // Output log if it contains any info
        if (buildLog.size() > 1)
            LogInfoCustom("OpenCL", fileName << " build info:" << std::endl << buildLog);
    } catch (cl::Error& e) {
        OpenCL::printBuildError(std::vector<cl::Device>(1, device), program, fileName);
        throw e;
    }

    return program;
}

cl::Program OpenCL::buildProgram(const std::string& fileName, const std::string& header /*= ""*/,
                                 const std::string& defines /*= ""*/) {
    return OpenCL::buildProgram(fileName, header, defines, OpenCL::getPtr()->getQueue());
}

void OpenCL::addCommonIncludeDirectory(const std::string& directoryPath) {
    if (filesystem::directoryExists(directoryPath)) includeDirectories_.push_back(directoryPath);
}

void OpenCL::removeCommonIncludeDirectory(const std::string& directoryPath) {
    std::vector<std::string>::iterator it =
        std::find(includeDirectories_.begin(), includeDirectories_.end(), directoryPath);

    if (it != includeDirectories_.end()) {
        includeDirectories_.erase(it);
    }
}

std::string OpenCL::getIncludeDefine() const {
    std::string result;

    for (std::vector<std::string>::const_iterator it = includeDirectories_.begin();
         it != includeDirectories_.end(); ++it) {
        result += " -I " + *it;
    }

    return result;
}

void OpenCL::setDevice(cl::Device device, bool glSharing) {
    try {
        // Check if we are setting the same device
        if (gpuDevice_() == device()) {
            auto sharingProperties = getGLSharingContextProperties();
            auto contextProperties = gpuContext_.getInfo<CL_CONTEXT_PROPERTIES>();
            bool sharingEnabled =
                (std::find(contextProperties.begin(), contextProperties.end(),
                           *sharingProperties.begin()) != contextProperties.end());

            if (sharingEnabled == glSharing) {
                // The device and sharing properties are the same.
                // No need to update the device
                return;
            }
        }

        gpuDevice_ = device;
        cl::Platform platform = device.getInfo<CL_DEVICE_PLATFORM>();
        std::vector<cl_context_properties> properties;

        if (glSharing) {
            properties = getGLSharingContextProperties();
            if (properties.empty()) {
                LogWarn("Unable to find OpenGL context. Will create OpenCL without OpenGL sharing");
            }
        }

        std::array<cl_context_properties, 3> platformProperties = {
            CL_CONTEXT_PLATFORM, (cl_context_properties)platform(), 0};
        properties.insert(properties.end(), platformProperties.begin(), platformProperties.end());

        try {
            cl_int error = CL_SUCCESS;
            gpuContext_ = cl::Context(gpuDevice_, &properties[0], nullptr, nullptr, &error);
            if (error != CL_SUCCESS) {
                throw OpenCLException("Error creating OpenCL context", IVW_CONTEXT);
            }

        } catch (cl::Error&) {
            LogWarn("Unable to create OpenCL context. Trying to without OpenGL sharing");
            properties.clear();
            properties.insert(properties.end(), platformProperties.begin(),
                              platformProperties.end());
            cl_int error = CL_SUCCESS;
            gpuContext_ = cl::Context(gpuDevice_, &properties[0], nullptr, nullptr, &error);
            if (error != CL_SUCCESS) {
                throw OpenCLException("Error creating OpenCL context", IVW_CONTEXT);
            }
            LogInfo("Succeeded creating OpenCL without OpenGL sharing.");
        }

        cl_command_queue_properties queueProperties = 0;
        cl_command_queue_properties supportedQueueProperties;
        gpuDevice_.getInfo(CL_DEVICE_QUEUE_PROPERTIES, &supportedQueueProperties);
#if IVW_PROFILING

        if (supportedQueueProperties & CL_QUEUE_PROFILING_ENABLE)
            queueProperties |= CL_QUEUE_PROFILING_ENABLE;

#endif
        synchronosGPUQueue_ = cl::CommandQueue(gpuContext_, gpuDevice_, queueProperties);

        if (supportedQueueProperties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
            queueProperties |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;

        asyncGPUQueue_ = cl::CommandQueue(gpuContext_, gpuDevice_, queueProperties);
        STRING_CLASS deviceExtensions = gpuDevice_.getInfo<CL_DEVICE_EXTENSIONS>();
        size_t foundAt = deviceExtensions.find_first_of("cl_khr_gl_event");

        if (foundAt != std::string::npos) {
            // Efficient cl/gl synchronization possible
        }
    } catch (cl::Error& err) {
        LogError("Failed to set OpenCL device. " << err.what() << "(" << err.err() << "), "
                                                 << errorCodeToString(err.err()) << std::endl);
    }
}

void LogOpenCLError(cl_int err, const char* message) {
    if (err != CL_SUCCESS) {
        std::ostringstream errorMessage;
        errorMessage << "OpenCL Error " << err << ": " << errorCodeToString(err) << " "
                     << getCLErrorResolveHint(err) << std::endl
                     << message;
        LogErrorCustom("OpenCL", errorMessage.str());
    }
}

std::string errorCodeToString(cl_int err) {
    std::string error;

    switch (err) {
        case CL_SUCCESS:
            error = "CL_SUCCESS";
            break;  //                                                        0

        case CL_DEVICE_NOT_FOUND:
            error = "CL_DEVICE_NOT_FOUND";
            break;  //                                                       -1

        case CL_DEVICE_NOT_AVAILABLE:
            error = "CL_DEVICE_NOT_AVAILABLE";
            break;  //                                                       -2

        case CL_COMPILER_NOT_AVAILABLE:
            error = "CL_COMPILER_NOT_AVAILABLE";
            break;  //                                                       -3

        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            error = "CL_MEM_OBJECT_ALLOCATION_FAILURE";
            break;  //                                                       -4

        case CL_OUT_OF_RESOURCES:
            error = "CL_OUT_OF_RESOURCES";
            break;  //                                                       -5

        case CL_OUT_OF_HOST_MEMORY:
            error = "CL_OUT_OF_HOST_MEMORY";
            break;  //                                                       -6

        case CL_PROFILING_INFO_NOT_AVAILABLE:
            error = "CL_PROFILING_INFO_NOT_AVAILABLE";
            break;  //                                                       -7

        case CL_MEM_COPY_OVERLAP:
            error = "CL_MEM_COPY_OVERLAP";
            break;  //                                                       -8

        case CL_IMAGE_FORMAT_MISMATCH:
            error = "CL_IMAGE_FORMAT_MISMATCH";
            break;  //                                                       -9

        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            error = "CL_IMAGE_FORMAT_NOT_SUPPORTED";
            break;  //                                                       -10

        case CL_BUILD_PROGRAM_FAILURE:
            error = "CL_BUILD_PROGRAM_FAILURE";
            break;  //                                                       -11

        case CL_MAP_FAILURE:
            error = "CL_MAP_FAILURE";
            break;  //                                                       -12
#if defined(CL_VERSION_1_1)

        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            error = "CL_MISALIGNED_SUB_BUFFER_OFFSET";
            break;  //                                                       -13

        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            error = "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
            break;  //                                                       -14
#endif

        case CL_INVALID_VALUE:
            error = "CL_INVALID_VALUE";
            break;  //                                                       -30

        case CL_INVALID_DEVICE_TYPE:
            error = "CL_INVALID_DEVICE_TYPE";
            break;  //                                                       -31

        case CL_INVALID_PLATFORM:
            error = "CL_INVALID_PLATFORM";
            break;  //                                                       -32

        case CL_INVALID_DEVICE:
            error = "CL_INVALID_DEVICE";
            break;  //                                                       -33

        case CL_INVALID_CONTEXT:
            error = "CL_INVALID_CONTEXT";
            break;  //                                                       -34

        case CL_INVALID_QUEUE_PROPERTIES:
            error = "CL_INVALID_QUEUE_PROPERTIES";
            break;  //                                                       -35

        case CL_INVALID_COMMAND_QUEUE:
            error = "CL_INVALID_COMMAND_QUEUE";
            break;  //                                                       -36

        case CL_INVALID_HOST_PTR:
            error = "CL_INVALID_HOST_PTR";
            break;  //                                                       -37

        case CL_INVALID_MEM_OBJECT:
            error = "CL_INVALID_MEM_OBJECT";
            break;  //                                                       -38

        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            error = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
            break;  //                                                       -39

        case CL_INVALID_IMAGE_SIZE:
            error = "CL_INVALID_IMAGE_SIZE";
            break;  //                                                       -40

        case CL_INVALID_SAMPLER:
            error = "CL_INVALID_SAMPLER";
            break;  //                                                       -41

        case CL_INVALID_BINARY:
            error = "CL_INVALID_BINARY";
            break;  //                                                       -42

        case CL_INVALID_BUILD_OPTIONS:
            error = "CL_INVALID_BUILD_OPTIONS";
            break;  //                                                       -43

        case CL_INVALID_PROGRAM:
            error = "CL_INVALID_PROGRAM";
            break;  //                                                       -44

        case CL_INVALID_PROGRAM_EXECUTABLE:
            error = "CL_INVALID_PROGRAM_EXECUTABLE";
            break;  //                                                       -45

        case CL_INVALID_KERNEL_NAME:
            error = "CL_INVALID_KERNEL_NAME";
            break;  //                                                       -46

        case CL_INVALID_KERNEL_DEFINITION:
            error = "CL_INVALID_KERNEL_DEFINITION";
            break;  //                                                       -47

        case CL_INVALID_KERNEL:
            error = "CL_INVALID_KERNEL";
            break;  //                                                       -48

        case CL_INVALID_ARG_INDEX:
            error = "CL_INVALID_ARG_INDEX";
            break;  //                                                       -49

        case CL_INVALID_ARG_VALUE:
            error = "CL_INVALID_ARG_VALUE";
            break;  //                                                       -50

        case CL_INVALID_ARG_SIZE:
            error = "CL_INVALID_ARG_SIZE";
            break;  //                                                       -51

        case CL_INVALID_KERNEL_ARGS:
            error = "CL_INVALID_KERNEL_ARGS";
            break;  //                                                       -52

        case CL_INVALID_WORK_DIMENSION:
            error = "CL_INVALID_WORK_DIMENSION";
            break;  //                                                       -53

        case CL_INVALID_WORK_GROUP_SIZE:
            error = "CL_INVALID_WORK_GROUP_SIZE";
            break;  //                                                       -54

        case CL_INVALID_WORK_ITEM_SIZE:
            error = "CL_INVALID_WORK_ITEM_SIZE";
            break;  //                                                       -55

        case CL_INVALID_GLOBAL_OFFSET:
            error = "CL_INVALID_GLOBAL_OFFSET";
            break;  //                                                       -56

        case CL_INVALID_EVENT_WAIT_LIST:
            error = "CL_INVALID_EVENT_WAIT_LIST";
            break;  //                                                       -57

        case CL_INVALID_EVENT:
            error = "CL_INVALID_EVENT";
            break;  //                                                       -58

        case CL_INVALID_OPERATION:
            error = "CL_INVALID_OPERATION";
            break;  //                                                       -59

        case CL_INVALID_GL_OBJECT:
            error = "CL_INVALID_GL_OBJECT";
            break;  //                                                       -60

        case CL_INVALID_BUFFER_SIZE:
            error = "CL_INVALID_BUFFER_SIZE";
            break;  //                                                       -61

        case CL_INVALID_MIP_LEVEL:
            error = "CL_INVALID_MIP_LEVEL";
            break;  //                                                       -62

        case CL_INVALID_GLOBAL_WORK_SIZE:
            error = "CL_INVALID_GLOBAL_WORK_SIZE";
            break;  //                                                       -63
#if defined(CL_INVALID_PROPERTY)

        case CL_INVALID_PROPERTY:
            error = "CL_INVALID_PROPERTY";
            break;  //                                                       -64
#endif

        default:
            error = "unknown OpenCL error";
            break;
    }

    return error;
}

std::string getCLErrorResolveHint(cl_int err) {
    // Add more hints when hint sources are discovered
    // Separate hints with comma:
    // Hint1, hint2
    std::string hint;

    switch (err) {
        case CL_SUCCESS:
            hint = "";
            break;  //                                                                      0

        case CL_DEVICE_NOT_FOUND:
            hint = "";
            break;  //                                                             -1

        case CL_DEVICE_NOT_AVAILABLE:
            hint = "";
            break;  //                                                         -2

        case CL_COMPILER_NOT_AVAILABLE:
            hint = "";
            break;  //                                                       -3

        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            hint = "";
            break;  //                                                -4

        case CL_OUT_OF_RESOURCES:
            hint = "";
            break;  //                                                             -5

        case CL_OUT_OF_HOST_MEMORY:
            hint = "";
            break;  //                                                           -6

        case CL_PROFILING_INFO_NOT_AVAILABLE:
            hint = "";
            break;  //                                                 -7

        case CL_MEM_COPY_OVERLAP:
            hint = "";
            break;  //                                                             -8

        case CL_IMAGE_FORMAT_MISMATCH:
            hint = "";
            break;  //                                                        -9

        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            hint = "";
            break;  //                                                   -10

        case CL_BUILD_PROGRAM_FAILURE:
            hint = "";
            break;  //                                                        -11

        case CL_MAP_FAILURE:
            hint = "";
            break;  //                                                                  -12
#if defined(CL_VERSION_1_1)

        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            hint = "";
            break;  //                                                 -13

        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            hint = "";
            break;  //                                    -14
#endif

        case CL_INVALID_VALUE:
            hint = "";
            break;  //                                                                -30

        case CL_INVALID_DEVICE_TYPE:
            hint = "";
            break;  //                                                          -31

        case CL_INVALID_PLATFORM:
            hint = "";
            break;  //                                                             -32

        case CL_INVALID_DEVICE:
            hint = "";
            break;  //                                                               -33

        case CL_INVALID_CONTEXT:
            hint = "";
            break;  //                                                              -34

        case CL_INVALID_QUEUE_PROPERTIES:
            hint = "";
            break;  //                                                     -35

        case CL_INVALID_COMMAND_QUEUE:
            hint = "";
            break;  //                                                        -36

        case CL_INVALID_HOST_PTR:
            hint = "";
            break;  //                                                             -37

        case CL_INVALID_MEM_OBJECT:
            hint = "";
            break;  //                                                           -38

        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            hint = "";
            break;  //                                              -39

        case CL_INVALID_IMAGE_SIZE:
            hint = "";
            break;  //                                                           -40

        case CL_INVALID_SAMPLER:
            hint = "";
            break;  //                                                              -41

        case CL_INVALID_BINARY:
            hint = "";
            break;  //                                                               -42

        case CL_INVALID_BUILD_OPTIONS:
            hint = "";
            break;  //                                                        -43

        case CL_INVALID_PROGRAM:
            hint = "";
            break;  //                                                              -44

        case CL_INVALID_PROGRAM_EXECUTABLE:
            hint = "";
            break;  //                                                   -45

        case CL_INVALID_KERNEL_NAME:
            hint = "";
            break;  //                                                          -46

        case CL_INVALID_KERNEL_DEFINITION:
            hint = "";
            break;  //                                                    -47

        case CL_INVALID_KERNEL:
            hint = "";
            break;  //                                                               -48

        case CL_INVALID_ARG_INDEX:
            hint = "";
            break;  //                                                            -49

        case CL_INVALID_ARG_VALUE:
            hint = "";
            break;  //                                                            -50

        case CL_INVALID_ARG_SIZE:
            hint = "";
            break;  //                                                             -51

        case CL_INVALID_KERNEL_ARGS:
            hint = "Has all kernel argument values been specified?";
            break;  //            -52

        case CL_INVALID_WORK_DIMENSION:
            hint = "";
            break;  //                                                       -53

        case CL_INVALID_WORK_GROUP_SIZE:
            hint = "";
            break;  //                                                      -54

        case CL_INVALID_WORK_ITEM_SIZE:
            hint = "";
            break;  //                                                       -55

        case CL_INVALID_GLOBAL_OFFSET:
            hint = "";
            break;  //                                                        -56

        case CL_INVALID_EVENT_WAIT_LIST:
            hint = "";
            break;  //                                                      -57

        case CL_INVALID_EVENT:
            hint = "";
            break;  //                                                                -58

        case CL_INVALID_OPERATION:
            hint = "";
            break;  //                                                            -59

        case CL_INVALID_GL_OBJECT:
            hint = "";
            break;  //                                                            -60

        case CL_INVALID_BUFFER_SIZE:
            hint = "";
            break;  //                                                          -61

        case CL_INVALID_MIP_LEVEL:
            hint = "";
            break;  //                                                            -62

        case CL_INVALID_GLOBAL_WORK_SIZE:
            hint = "";
            break;  //                                                     -63
#if defined(CL_INVALID_PROPERTY)

        case CL_INVALID_PROPERTY:
            hint = "";
            break;  //                                                             -64
#endif

        default:
            hint = "";
            break;
    }

    return hint;
}

std::string getCLErrorString(const cl::Error& err) {
    std::ostringstream ss;
    ss << "Error:" << err.what() << "(" << err.err() << "), " << errorCodeToString(err.err()) << " "
       << getCLErrorResolveHint(err.err()) << std::endl;
    return ss.str();
}

size_t getGlobalWorkGroupSize(size_t nItems, size_t localWorkGroupSize) {
    return localWorkGroupSize * static_cast<size_t>(ceil(static_cast<float>(nItems) /
                                                         static_cast<float>(localWorkGroupSize)));
}

size2_t getGlobalWorkGroupSize(size2_t nItems, glm::size2_t localWorkGroupSize) {
    return localWorkGroupSize * size2_t(glm::ceil(vec2(nItems) / vec2(localWorkGroupSize)));
}

size3_t getGlobalWorkGroupSize(size3_t nItems, glm::size3_t localWorkGroupSize) {
    return localWorkGroupSize * size3_t(glm::ceil(vec3(nItems) / vec3(localWorkGroupSize)));
}
}  // namespace inviwo
