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

#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/cl.hpp>
#include <modules/opencl/glmcl.h>
#include <modules/opencl/openclcapabilities.h>
#include <modules/opencl/syncclgl.h>
#include <inviwo/core/io/textfilereader.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/filesystem.h>
#include <pathsexternalmodules.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <fstream>
#if !(WIN32 || __APPLE__) // LINUX
#include <GL/glx.h> // glXCurrentContext()
#endif

#include <stdlib.h>
#include <stdio.h>

namespace cl {

}

namespace inviwo {


OpenCL::OpenCL() {
    initialize(true);
}

void OpenCL::initialize(bool glSharing) {
    try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        if (platforms.size() == 0) {
            LogError("No OpenCL platforms found" << std::endl);
            return;
        }

        cl::Platform platform;
        cl::Device device;
        getBestGPUDeviceOnSystem(device, platform);
        setDevice(device, glSharing);
    } catch (cl::Error& err) {
        LogError("ERROR: Failed to initialize OpenCL. " << err.what() << "(" << err.err() << "), " << errorCodeToString(err.err()) << std::endl);
    }
}
bool OpenCL::isValidImageFormat(const cl::Context& context, const cl::ImageFormat& format) {
    std::vector<cl::ImageFormat> formats;
    context.getSupportedImageFormats(CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE2D, &formats);

    for (::size_t i = 0; i < formats.size(); ++i) {
        if (formats[i].image_channel_order == format.image_channel_order && formats[i].image_channel_data_type == format.image_channel_data_type) {
            return true;
        }
    }

    return false;
}
bool OpenCL::isValidVolumeFormat(const cl::Context& context, const cl::ImageFormat& format) {
    std::vector<cl::ImageFormat> formats;
    context.getSupportedImageFormats(CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE3D, &formats);

    for (::size_t i = 0; i < formats.size(); ++i) {
        if (formats[i].image_channel_order == format.image_channel_order && formats[i].image_channel_data_type == format.image_channel_data_type) {
            return true;
        }
    }

    return false;
}

/*! \brief Get the device that has most compute units.
    *
    *  @param bestDevice Set to found device, if found.
    *  @param onPlatform Set to platform that device exist on, if found.
    *  \return True if any device found, false otherwise.
    */
bool OpenCL::getBestGPUDeviceOnSystem(cl::Device& bestDevice, cl::Platform& onPlatform) {
    bool foundDevice = false;
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl_uint maxComputeUnits = 0;
    cl_device_type deviceType = CL_DEVICE_TYPE_DEFAULT;
    // Search for best device
    for (::size_t i = 0; i < platforms.size(); ++i) {
        std::vector<cl::Device> devices;

        try {
            platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &devices);

            //platforms[i].getDevices(CL_DEVICE_TYPE_CPU, &devices);
            for (::size_t j = 0; j < devices.size(); ++j) {
                cl_uint tmpMaxComputeUnits;
                devices[j].getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &tmpMaxComputeUnits);
                cl_device_type otherDeviceType = devices[j].getInfo<CL_DEVICE_TYPE>();
                // Select if the current device is not a GPU device
                // or if the new one has more compute units than the previous GPU device
                if (deviceType != CL_DEVICE_TYPE_GPU || 
                    (otherDeviceType == CL_DEVICE_TYPE_GPU && maxComputeUnits < tmpMaxComputeUnits)) {
                    bestDevice = devices[j];
                    onPlatform = platforms[i];
                    maxComputeUnits = tmpMaxComputeUnits;
                    deviceType = otherDeviceType;
                    foundDevice = true;
                } 
            }
        }  catch (cl::Error&) {
            // Error getting device, continue with others
        }
    }

    return foundDevice;
}

void OpenCL::printBuildError(const std::vector<cl::Device>& devices, const cl::Program& program, const std::string& filename) {
    for (::size_t i = 0; i < devices.size(); ++i) {
        cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(devices[i]);

        // Houston, we have a problem
        if (status == CL_BUILD_ERROR) {
            std::string buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[i]);
            LogErrorCustom("OpenCL", filename << " build error:" << std::endl << buildLog);
        }
    }
}
void OpenCL::printBuildError(const cl::Device& device, const cl::Program& program, const std::string& filename) {
    printBuildError(std::vector<cl::Device>(1, device), program, filename);
}

std::vector<cl_context_properties> OpenCL::getGLSharingContextProperties() {
#if WIN32
    cl_context_properties props[] = {
        CL_GL_CONTEXT_KHR, (cl_context_properties) wglGetCurrentContext(),
        CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC()
    };
#elif __APPLE__
    CGLContextObj glContext = CGLGetCurrentContext();
    CGLShareGroupObj shareGroup = CGLGetShareGroup(glContext);
    cl_context_properties props[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)shareGroup
    };
#else // LINUX
    cl_context_properties props[] = {
        CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay()
    };
#endif
    return std::vector<cl_context_properties>(props, props + sizeof(props)/sizeof(cl_context_properties));
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
        for (::size_t j = i+1; j < platforms.size(); j++) {
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
        }  catch (cl::Error&) {
            // Error getting device, continue with others
        }
    }

    return allDevices;
}

cl::Program OpenCL::buildProgram(const std::string& fileName, const std::string& defines, const cl::CommandQueue& queue) {
    cl::Context context = queue.getInfo<CL_QUEUE_CONTEXT>();
    cl::Device device = queue.getInfo<CL_QUEUE_DEVICE>();
    // build the program from the source in the file
    std::ifstream file(fileName.c_str());
    TextFileReader fileReader(fileName);
    std::string prog;

    try {
        prog = fileReader.read();
    } catch (std::ifstream::failure&) {}

    std::string concatenatedDefines = OpenCL::getPtr()->getIncludeDefine() + defines;
    cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length()+1));
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

cl::Program OpenCL::buildProgram(const std::string& fileName, const std::string& defines /*= ""*/){
    return OpenCL::buildProgram(fileName, defines, OpenCL::getPtr()->getQueue());
}

void OpenCL::addCommonIncludeDirectory(const std::string& directoryPath){
    if(filesystem::directoryExists(directoryPath))
        includeDirectories_.push_back(directoryPath);
}

void OpenCL::addCommonIncludeDirectory(InviwoApplication::PathType pathType, const std::string& relativePath) {
    addCommonIncludeDirectory(InviwoApplication::getPtr()->getPath(pathType) + relativePath);
#ifdef IVW_EXTERNAL_MODULES_PATH_COUNT
    if(pathType == InviwoApplication::PATH_MODULES){
        for(int i=0; i < IVW_EXTERNAL_MODULES_PATH_COUNT; ++i){
            addCommonIncludeDirectory(externalModulePaths_[i] + "/" + relativePath);
        }
    }
#endif
}

void OpenCL::removeCommonIncludeDirectory(const std::string& directoryPath){
    std::vector<std::string>::iterator it = std::find(includeDirectories_.begin(), includeDirectories_.end(), directoryPath);

    if (it != includeDirectories_.end()) {
        includeDirectories_.erase(it);
    }
}

std::string OpenCL::getIncludeDefine() const{
    std::string result;

    for (std::vector<std::string>::const_iterator it = includeDirectories_.begin(); it != includeDirectories_.end(); ++it) {
        result += " -I " + *it;
    }

    return result;
}

void OpenCL::setDevice(cl::Device device, bool glSharing) {
    try {
        // Check if we are setting the same device
        if (gpuDevice_() == device()) {
            std::vector<cl_context_properties> sharingProperties = getGLSharingContextProperties();
            std::vector<cl_context_properties> contextProperties = gpuContext_.getInfo<CL_CONTEXT_PROPERTIES>();
            bool sharingEnabled = (std::find(contextProperties.begin(), contextProperties.end(),
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

        if (glSharing)
            properties = getGLSharingContextProperties();

        cl_context_properties platformProperties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform(), 0};
        properties.insert(properties.end(), platformProperties, platformProperties+ sizeof(platformProperties)/sizeof(cl_context_properties));

        try
        {
            gpuContext_ = cl::Context(gpuDevice_, &properties[0]);
        } catch (cl::Error&)
        {
            LogInfo("ERROR: Unable to create OpenCL context. Trying to create without openGL sharing... ");
            properties.clear();
            properties.insert(properties.end(), platformProperties, platformProperties+ sizeof(platformProperties)/sizeof(cl_context_properties));
            gpuContext_ = cl::Context(gpuDevice_, &properties[0]);
            LogInfo("Succeeded creating OpenCL without OpenGL sharing. ");
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
        LogError("Faile to set OpenCL device. " << err.what() << "(" << err.err() << "), " << errorCodeToString(err.err()) << std::endl);
    }
}

void LogOpenCLError(cl_int err, const char* message) {
    if (err != CL_SUCCESS) {
        std::ostringstream errorMessage;
        errorMessage << "OpenCL Error " << err << ": " << errorCodeToString(err) << " " << getCLErrorResolveHint(err) << std::endl << message;
        LogErrorCustom("OpenCL", errorMessage.str());
    }
}

std::string errorCodeToString(cl_int err) {
    std::string error;

    switch (err) {
        case CL_SUCCESS:
            error = "CL_SUCCESS";
            break; //                                                                         0

        case CL_DEVICE_NOT_FOUND:
            error = "CL_DEVICE_NOT_FOUND";
            break; //                                                      -1

        case CL_DEVICE_NOT_AVAILABLE:
            error = "CL_DEVICE_NOT_AVAILABLE";
            break; //                                              -2

        case CL_COMPILER_NOT_AVAILABLE:
            error = "CL_COMPILER_NOT_AVAILABLE";
            break; //                                          -3

        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            error = "CL_MEM_OBJECT_ALLOCATION_FAILURE";
            break; //                            -4

        case CL_OUT_OF_RESOURCES:
            error = "CL_OUT_OF_RESOURCES";
            break; //                                                      -5

        case CL_OUT_OF_HOST_MEMORY:
            error = "CL_OUT_OF_HOST_MEMORY";
            break; //                                                  -6

        case CL_PROFILING_INFO_NOT_AVAILABLE:
            error = "CL_PROFILING_INFO_NOT_AVAILABLE";
            break; //                              -7

        case CL_MEM_COPY_OVERLAP:
            error = "CL_MEM_COPY_OVERLAP";
            break; //                                                      -8

        case CL_IMAGE_FORMAT_MISMATCH:
            error = "CL_IMAGE_FORMAT_MISMATCH";
            break; //                                            -9

        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            error = "CL_IMAGE_FORMAT_NOT_SUPPORTED";
            break; //                                  -10

        case CL_BUILD_PROGRAM_FAILURE:
            error = "CL_BUILD_PROGRAM_FAILURE";
            break; //                                            -11

        case CL_MAP_FAILURE:
            error = "CL_MAP_FAILURE";
            break; //                                                                -12
#if defined(CL_VERSION_1_1)

        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            error = "CL_MISALIGNED_SUB_BUFFER_OFFSET";
            break; //                              -13

        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            error = "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
            break; //    -14
#endif

        case CL_INVALID_VALUE:
            error = "CL_INVALID_VALUE";
            break; //                                                            -30

        case CL_INVALID_DEVICE_TYPE:
            error = "CL_INVALID_DEVICE_TYPE";
            break; //                                                -31

        case CL_INVALID_PLATFORM:
            error = "CL_INVALID_PLATFORM";
            break; //                                                      -32

        case CL_INVALID_DEVICE:
            error = "CL_INVALID_DEVICE";
            break; //                                                          -33

        case CL_INVALID_CONTEXT:
            error = "CL_INVALID_CONTEXT";
            break; //                                                        -34

        case CL_INVALID_QUEUE_PROPERTIES:
            error = "CL_INVALID_QUEUE_PROPERTIES";
            break; //                                      -35

        case CL_INVALID_COMMAND_QUEUE:
            error = "CL_INVALID_COMMAND_QUEUE";
            break; //                                            -36

        case CL_INVALID_HOST_PTR:
            error = "CL_INVALID_HOST_PTR";
            break; //                                                      -37

        case CL_INVALID_MEM_OBJECT:
            error = "CL_INVALID_MEM_OBJECT";
            break; //                                                  -38

        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            error = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
            break; //                        -39

        case CL_INVALID_IMAGE_SIZE:
            error = "CL_INVALID_IMAGE_SIZE";
            break; //                                                  -40

        case CL_INVALID_SAMPLER:
            error = "CL_INVALID_SAMPLER";
            break; //                                                        -41

        case CL_INVALID_BINARY:
            error = "CL_INVALID_BINARY";
            break; //                                                          -42

        case CL_INVALID_BUILD_OPTIONS:
            error = "CL_INVALID_BUILD_OPTIONS";
            break; //                                            -43

        case CL_INVALID_PROGRAM:
            error = "CL_INVALID_PROGRAM";
            break; //                                                        -44

        case CL_INVALID_PROGRAM_EXECUTABLE:
            error = "CL_INVALID_PROGRAM_EXECUTABLE";
            break; //                                  -45

        case CL_INVALID_KERNEL_NAME:
            error = "CL_INVALID_KERNEL_NAME";
            break; //                                                -46

        case CL_INVALID_KERNEL_DEFINITION:
            error = "CL_INVALID_KERNEL_DEFINITION";
            break; //                                    -47

        case CL_INVALID_KERNEL:
            error = "CL_INVALID_KERNEL";
            break; //                                                          -48

        case CL_INVALID_ARG_INDEX:
            error = "CL_INVALID_ARG_INDEX";
            break; //                                                    -49

        case CL_INVALID_ARG_VALUE:
            error = "CL_INVALID_ARG_VALUE";
            break; //                                                    -50

        case CL_INVALID_ARG_SIZE:
            error = "CL_INVALID_ARG_SIZE";
            break; //                                                      -51

        case CL_INVALID_KERNEL_ARGS:
            error = "CL_INVALID_KERNEL_ARGS";
            break; //                                                -52

        case CL_INVALID_WORK_DIMENSION:
            error = "CL_INVALID_WORK_DIMENSION";
            break; //                                          -53

        case CL_INVALID_WORK_GROUP_SIZE:
            error = "CL_INVALID_WORK_GROUP_SIZE";
            break; //                                        -54

        case CL_INVALID_WORK_ITEM_SIZE:
            error = "CL_INVALID_WORK_ITEM_SIZE";
            break; //                                          -55

        case CL_INVALID_GLOBAL_OFFSET:
            error = "CL_INVALID_GLOBAL_OFFSET";
            break; //                                            -56

        case CL_INVALID_EVENT_WAIT_LIST:
            error = "CL_INVALID_EVENT_WAIT_LIST";
            break; //                                        -57

        case CL_INVALID_EVENT:
            error = "CL_INVALID_EVENT";
            break; //                                                            -58

        case CL_INVALID_OPERATION:
            error = "CL_INVALID_OPERATION";
            break; //                                                    -59

        case CL_INVALID_GL_OBJECT:
            error = "CL_INVALID_GL_OBJECT";
            break; //                                                    -60

        case CL_INVALID_BUFFER_SIZE:
            error = "CL_INVALID_BUFFER_SIZE";
            break; //                                                -61

        case CL_INVALID_MIP_LEVEL:
            error = "CL_INVALID_MIP_LEVEL";
            break; //                                                    -62

        case CL_INVALID_GLOBAL_WORK_SIZE:
            error = "CL_INVALID_GLOBAL_WORK_SIZE";
            break; //                                      -63
#if defined(CL_INVALID_PROPERTY)

        case CL_INVALID_PROPERTY:
            error = "CL_INVALID_PROPERTY";
            break; //                                                      -64
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
            break; //                                                                      0

        case CL_DEVICE_NOT_FOUND:
            hint = "";
            break; //                                                             -1

        case CL_DEVICE_NOT_AVAILABLE:
            hint = "";
            break; //                                                         -2

        case CL_COMPILER_NOT_AVAILABLE:
            hint = "";
            break; //                                                       -3

        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            hint = "";
            break; //                                                -4

        case CL_OUT_OF_RESOURCES:
            hint = "";
            break; //                                                             -5

        case CL_OUT_OF_HOST_MEMORY:
            hint = "";
            break; //                                                           -6

        case CL_PROFILING_INFO_NOT_AVAILABLE:
            hint = "";
            break; //                                                 -7

        case CL_MEM_COPY_OVERLAP:
            hint = "";
            break; //                                                             -8

        case CL_IMAGE_FORMAT_MISMATCH:
            hint = "";
            break; //                                                        -9

        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            hint = "";
            break; //                                                   -10

        case CL_BUILD_PROGRAM_FAILURE:
            hint = "";
            break; //                                                        -11

        case CL_MAP_FAILURE:
            hint = "";
            break; //                                                                  -12
#if defined(CL_VERSION_1_1)

        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            hint = "";
            break; //                                                 -13

        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            hint = "";
            break; //                                    -14
#endif

        case CL_INVALID_VALUE:
            hint = "";
            break; //                                                                -30

        case CL_INVALID_DEVICE_TYPE:
            hint = "";
            break; //                                                          -31

        case CL_INVALID_PLATFORM:
            hint = "";
            break; //                                                             -32

        case CL_INVALID_DEVICE:
            hint = "";
            break; //                                                               -33

        case CL_INVALID_CONTEXT:
            hint = "";
            break; //                                                              -34

        case CL_INVALID_QUEUE_PROPERTIES:
            hint = "";
            break; //                                                     -35

        case CL_INVALID_COMMAND_QUEUE:
            hint = "";
            break; //                                                        -36

        case CL_INVALID_HOST_PTR:
            hint = "";
            break; //                                                             -37

        case CL_INVALID_MEM_OBJECT:
            hint = "";
            break; //                                                           -38

        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            hint = "";
            break; //                                              -39

        case CL_INVALID_IMAGE_SIZE:
            hint = "";
            break; //                                                           -40

        case CL_INVALID_SAMPLER:
            hint = "";
            break; //                                                              -41

        case CL_INVALID_BINARY:
            hint = "";
            break; //                                                               -42

        case CL_INVALID_BUILD_OPTIONS:
            hint = "";
            break; //                                                        -43

        case CL_INVALID_PROGRAM:
            hint = "";
            break; //                                                              -44

        case CL_INVALID_PROGRAM_EXECUTABLE:
            hint = "";
            break; //                                                   -45

        case CL_INVALID_KERNEL_NAME:
            hint = "";
            break; //                                                          -46

        case CL_INVALID_KERNEL_DEFINITION:
            hint = "";
            break; //                                                    -47

        case CL_INVALID_KERNEL:
            hint = "";
            break; //                                                               -48

        case CL_INVALID_ARG_INDEX:
            hint = "";
            break; //                                                            -49

        case CL_INVALID_ARG_VALUE:
            hint = "";
            break; //                                                            -50

        case CL_INVALID_ARG_SIZE:
            hint = "";
            break; //                                                             -51

        case CL_INVALID_KERNEL_ARGS:
            hint = "Has all kernel argument values been specified?";
            break; //            -52

        case CL_INVALID_WORK_DIMENSION:
            hint = "";
            break; //                                                       -53

        case CL_INVALID_WORK_GROUP_SIZE:
            hint = "";
            break; //                                                      -54

        case CL_INVALID_WORK_ITEM_SIZE:
            hint = "";
            break; //                                                       -55

        case CL_INVALID_GLOBAL_OFFSET:
            hint = "";
            break; //                                                        -56

        case CL_INVALID_EVENT_WAIT_LIST:
            hint = "";
            break; //                                                      -57

        case CL_INVALID_EVENT:
            hint = "";
            break; //                                                                -58

        case CL_INVALID_OPERATION:
            hint = "";
            break; //                                                            -59

        case CL_INVALID_GL_OBJECT:
            hint = "";
            break; //                                                            -60

        case CL_INVALID_BUFFER_SIZE:
            hint = "";
            break; //                                                          -61

        case CL_INVALID_MIP_LEVEL:
            hint = "";
            break; //                                                            -62

        case CL_INVALID_GLOBAL_WORK_SIZE:
            hint = "";
            break; //                                                     -63
#if defined(CL_INVALID_PROPERTY)

        case CL_INVALID_PROPERTY:
            hint = "";
            break; //                                                             -64
#endif

        default:
            hint = "";
            break;
    }

    return hint;
}

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
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::FLOAT16:
            clFormat = cl::ImageFormat(CL_R, CL_HALF_FLOAT);
            break;

        case DataFormatEnums::FLOAT32:
            clFormat = cl::ImageFormat(CL_R, CL_FLOAT);
            break;

        case DataFormatEnums::FLOAT64:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
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
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
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
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::Vec2FLOAT16:
            clFormat = cl::ImageFormat(CL_RG, CL_HALF_FLOAT);
            break;

        case DataFormatEnums::Vec2FLOAT32:
            clFormat = cl::ImageFormat(CL_RG, CL_FLOAT);
            break;

        case DataFormatEnums::Vec2FLOAT64:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
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
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
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
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::Vec3FLOAT16:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::Vec3FLOAT32:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::Vec3FLOAT64:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::Vec3INT8:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::Vec3INT16:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::Vec3INT32:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::Vec3INT64:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::Vec3UINT8:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::Vec3UINT16:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::Vec3UINT32:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::Vec3UINT64:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::Vec4FLOAT16:
            clFormat = cl::ImageFormat(CL_RGBA, CL_HALF_FLOAT);
            break;

        case DataFormatEnums::Vec4FLOAT32:
            clFormat = cl::ImageFormat(CL_RGBA, CL_FLOAT);
            break;

        case DataFormatEnums::Vec4FLOAT64:
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
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
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
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
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Invalid conversion");
            break;

        case DataFormatEnums::NUMBER_OF_FORMATS:
        default:
            // Should not be able to reach here
            LogErrorCustom("cl::ImageFormat typeToImageFormat", "Format not implmented yet");
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

std::string getCLErrorString(const cl::Error& err)
{
    std::ostringstream ss;
    ss << "Error:" << err.what() << "(" << err.err() << "), " << errorCodeToString(err.err()) << " " << getCLErrorResolveHint(
           err.err()) << std::endl;
    return ss.str();
}

size_t getGlobalWorkGroupSize(size_t nItems, size_t localWorkGroupSize)
{
    return localWorkGroupSize*static_cast<size_t>(ceil(static_cast<float>(nItems) / static_cast<float>(localWorkGroupSize)));
}

svec2 getGlobalWorkGroupSize(svec2 nItems, glm::svec2 localWorkGroupSize)
{
    return localWorkGroupSize*svec2(glm::ceil(vec2(nItems) / vec2(localWorkGroupSize)));
}

svec3 getGlobalWorkGroupSize(svec3 nItems, glm::svec3 localWorkGroupSize)
{
    return localWorkGroupSize*svec3(glm::ceil(vec3(nItems) / vec3(localWorkGroupSize)));
}

}
