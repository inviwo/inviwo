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

#ifndef IVW_INVIWOOPENCL_H
#define IVW_INVIWOOPENCL_H
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/singleton.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <modules/opencl/cl.hpp>
#include <modules/opencl/clockcl.h>
#include <modules/opencl/glmcl.h>
#include <modules/opencl/openclmoduledefine.h>

#if defined(CL_VERSION_1_2)
//using Image2DGL cl::Image2D;
//using Image3DGL cl::Image3D;
namespace cl {
typedef ImageGL Image2DGL;
typedef ImageGL Image3DGL;
}
#endif
using glm::size2_t;
using glm::size3_t;

using cl::CommandQueue;
using cl::Context;
using cl::Device;
using cl::Platform;
using cl::Program;
using cl::ImageFormat;


namespace inviwo {

// This macro will create an OpenCL event called "profilingEvent" that should be
// used when executing the kernel. If IVW_PROFILING is defined it will 
// call LogInfo with the execution time.
// Example:
// IVW_OPENCL_PROFILING(profilingEvent, "")
// OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(*ernel_, cl::NullRange, globalWorkGroupSize, localWorkGroupSize, nullptr, profilingEvent);
// 

//#if IVW_PROFILING 
//    #define IVW_OPENCL_PROFILING(profilingEvent, "") cl::Event* profilingEvent = new cl::Event(); 
//#else 
//    #define IVW_OPENCL_PROFILING(profilingEvent, "") cl::Event* profilingEvent = nullptr; 
//#endif 
//#if IVW_PROFILING 
//    #define  \
//    try { \
//        profilingEvent->wait(); \
//        LogInfo("Exec time: " << profilingEvent->getElapsedTime() << " ms"); \
//    } catch (cl::Error& err) { \
//        LogError(getCLErrorString(err)); \
//    } \
//    delete profilingEvent; 
//#else 
//    #define 
//#endif
//
//#if IVW_PROFILING 
//#define IVW_OPENCL_PROFILING(profilingEvent, "")N(name) cl::Event* name = new cl::Event(); 
//#else 
//#define IVW_OPENCL_PROFILING(profilingEvent, "")N(name) cl::Event* name = nullptr; 
//#endif 
//#if IVW_PROFILING 
//#define N(name) \
//    try { \
//    name->wait(); \
//    LogInfo(#name " exec time: " << name->getElapsedTime() << " ms"); \
//} catch (cl::Error& err) { \
//    LogError(getCLErrorString(err)); \
//} \
//    delete name; 
//#else 
//#define N(name)
//#endif

/** \class OpenCL
 *
 * Singleton class that manages OpenCL context and queues.
 *
 */
class IVW_MODULE_OPENCL_API OpenCL: public Singleton<OpenCL> {
public:
    OpenCL();

    /**
     * Get queue that can perform tasks in serial (no need to explicitly manage events).
     * This should be the first choice of use. Expert users may want to use the
     * asynchronous queue.
     */
    const cl::CommandQueue& getQueue() const { return synchronosGPUQueue_; }
    /**
     * Get queue that can perform tasks in parallel (need to explicitly manage events).
     * This is for expert users. be the first choice of use. Expert users may want to use the
     * asynchronous queue.
     */
    const cl::CommandQueue& getAsyncQueue() const { return asyncGPUQueue_; }

    /**
     * Get default OpenCL context.
     */
    const cl::Context& getContext() const { return gpuContext_; }

    /**
     * Get default OpenCL device.
     */
    const cl::Device& getDevice() const { return gpuDevice_; }

    /**
     * Set default OpenCL device.
     * Note that this may have an effect on all allocated OpenCL resources (queue changes).
     * Therefore make sure that no OpenCL resources are used before calling this function.
     * This can be done by closing the workspace.
     */
    void setDevice(cl::Device device, bool glSharing);

    static cl::Program buildProgram(const std::string& fileName, const std::string& defines = "");
    static cl::Program buildProgram(const std::string& fileName, const std::string& defines, const cl::CommandQueue& queue);

    /**
     * Check if image format combination is valid.
     * @return True if image format combination is valid, false otherwise.
     */
    static bool isValidImageFormat(const cl::Context& context, const cl::ImageFormat& format);

    /**
     * Check if volume format (Image3D) combination is valid.
     * @return True if image format combination is valid, false otherwise.
     */
    static bool isValidVolumeFormat(const cl::Context& context, const cl::ImageFormat& format);

    /**
     * Outputs formatted build hint to logger.
     *
     * @param devices (const std::vector<cl::Device> &)
     * @param program (const cl::Program &)
     * @param filename (const std::string &)
     * @return (void)
     */
    static void printBuildError(const std::vector<cl::Device>& devices, const cl::Program& program, const std::string& filename = "");

    static void printBuildError(const cl::Device& device, const cl::Program& program, const std::string& filename = "");

    /**
     * Get OpenGL sharing properties depending on operating system.
     */
    static std::vector<cl_context_properties> getGLSharingContextProperties();

    /**
     * Get all devices on the system.
     */
    static std::vector<cl::Device> getAllDevices();

    /**
     * Add a directory as an include path to be used when compiling OpenCL kernels.
     *
     * @param directoryPath Directory path to include
     */
    void addCommonIncludeDirectory(const std::string& directoryPath);

    /**
     * Add a relative directory as an include path to be used when compiling OpenCL kernels.
     *
     * @param pathType PathType of directory
     * @param relativePath Relative directory path to include
     */
    void addCommonIncludeDirectory(InviwoApplication::PathType pathType, const std::string& relativePath);

    /**
     * Remove common include path.
     *
     * @param directoryPath Directory path to remove
     */
    void removeCommonIncludeDirectory(const std::string& directoryPath);

    const std::vector<std::string>& getCommonIncludeDirectories() const { return includeDirectories_; }

    /**
     *  Get the device that has most compute units.
     *  @param bestDevice Set to found device, if found.
     *  @param onPlatform Set to platform that device exist on, if found.
     *  @return True if any device found, false otherwise.
     */
    static bool getBestGPUDeviceOnSystem(cl::Device& bestDevice, cl::Platform& onPlatform);
private:
    OpenCL(OpenCL const&) {};
    void operator=(OpenCL const&) {};

    /// Initialize OpenCL context and queues.
    void initialize(bool enableOpenGLSharing);

    /**
     * Merges all include directories into a string. Each include directory will be preceded by -I
     *
     * @return Define to be used when building the programs
     */
    std::string getIncludeDefine() const;

    /// Queue which can perform tasks in serial (no need to explicitly manage events)
    cl::CommandQueue synchronosGPUQueue_;
    /// Queue which can perform tasks in parallel
    cl::CommandQueue asyncGPUQueue_;
    /// Default context associated with queues and device
    cl::Context gpuContext_;
    /// Default device associated with queues and device
    cl::Device gpuDevice_;
    // Include directories define
    std::vector<std::string> includeDirectories_;

};

/**
 * Computes the nearest multiple of local work group size.
 * global work group size = localWorkGroupSize*ceil((float)nItems/(float)localWorkGroupSize)
 *
 * @param nItems
 * @param localWorkGroupSize Local work group size of kernel
 * @return
 */
IVW_MODULE_OPENCL_API size_t getGlobalWorkGroupSize(size_t nItems, size_t localWorkGroupSize);

IVW_MODULE_OPENCL_API size2_t getGlobalWorkGroupSize(size2_t nItems, glm::size2_t localWorkGroupSize);
IVW_MODULE_OPENCL_API size3_t getGlobalWorkGroupSize(size3_t nItems, glm::size3_t localWorkGroupSize);


/**
 * Creates a readable hint report from an OpenCL exception.
 * Example usage: LogError(getCLErrorString(err))
 *
 * @param err OpenCL exception
 * @return Error report string.
 */
IVW_MODULE_OPENCL_API std::string getCLErrorString(const cl::Error& err);

/**
 * Creates an error report and outputs it using the log functionality.
 *
 * @param err OpenCL error code
 * @param message Message to be passed along with the error.
 */
IVW_MODULE_OPENCL_API void LogOpenCLError(cl_int err, const char* message = "");

/** \brief Get string representation of hint code according to definitions in CL/cl.h
 *
 *  \return The hint code string.
 */
IVW_MODULE_OPENCL_API std::string errorCodeToString(cl_int err);

/**
 * Returns hints on how to resolve a particular OpenCL hint.
 *
 * @param err
 * @return A hint on what could cause the hint.
 */
IVW_MODULE_OPENCL_API std::string getCLErrorResolveHint(cl_int err);

#if defined(IVW_DEBUG)
#define LogCLError #if defined(__CL_ENABLE_EXCEPTIONS) \\LogOpenCLError(error)
#else
#define LogCLError
#endif

//
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

class CLFormats {

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

    CLFormat getCLFormat(DataFormatEnums::Id id) const {
        ivwAssert(CLFormatArray_[static_cast<int>(id)].valid, "Accessing invalid CLFormat");
        return CLFormatArray_[static_cast<int>(id)];
    };

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
}


#endif // IVW_INVIWOOPENGL_H
