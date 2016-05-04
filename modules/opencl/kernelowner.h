/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_KERNEL_OWNER_H
#define IVW_KERNEL_OWNER_H

#include <modules/opencl/openclmoduledefine.h>
#include <modules/opencl/inviwoopencl.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/observer.h>

#include <set>

namespace inviwo {

class Processor;

class IVW_MODULE_OPENCL_API KernelObserver: public Observer {
public:
    KernelObserver() = default;

    /**
    * This method will be called when observed object changes.
    * Override it to add behavior.
    */
    virtual void onKernelCompiled(const cl::Kernel* kernel) {};

};

class IVW_MODULE_OPENCL_API KernelObservable: public Observable<KernelObserver> {
public:
    KernelObservable() = default;

    virtual void notifyObserversKernelCompiled(const cl::Kernel* kernel);
};

/** \class KernelOwner
 * Interface for classes that use OpenCL features.
 * Using this class will enable KernelObservs to react
 * when a kernel has been compiled.
 * Since we do not wish to alter the cl::Kernel class 
 * we provide wrappers to be used in the framework instead.
 * @see ProcessorKernelOwner
 * @see KernelObserver
 */
class IVW_MODULE_OPENCL_API KernelOwner: public KernelObservable {
public:
    KernelOwner() {};
    virtual ~KernelOwner();

    /**
     * Builds, adds and returns the kernel if successfully built. 
     * Only provide the file name as the file will be searched for in all the include paths.
     *
     * @note Do not delete the returned kernel
     * @param filePath Name of the file containing kernel, i.e. myfile.cl
     * @param kernelName Name of kernel
     * @param header Added before file contents. Example usage "#define DataType float \n"
     * @param defines Defines to be set when building kernel
     * @return bool Kernel if successfully built, otherwise nullptr
     */
    cl::Kernel* addKernel(const std::string& fileName, const std::string& kernelName, const std::string& header = "", const std::string& defines = "");

    /** 
     * \brief Remove kernel and effectively stop observing the kernel 
     * 
     * @param cl::Kernel * kernel Kernel owned by the KernelOwner
     */
    void removeKernel(cl::Kernel* kernel);

    const std::set<cl::Kernel*>& getKernels() const { return kernels_; }

protected:
    std::set<cl::Kernel*> kernels_;
};

/** \class ProcessorKernelOwner
 * Convenience class for processors that use OpenCL features.
 * Inherit from this class to enable invalidation of processor
 * when a kernel has been compiled.
 * @see ProcessorKernelOwner
 */
class IVW_MODULE_OPENCL_API ProcessorKernelOwner: public KernelOwner {
public:
    ProcessorKernelOwner(Processor* processor);
    virtual ~ProcessorKernelOwner() {};

    /**
     * Notifies kernel observers that the kernel was compiled and
     * calls invalidate(InvalidationLevel::InvalidResources) on Processor specified in the constructor.
     * Override this method to perform operations after a successful kernel 
     * compilation. 
     * @param kernel The kernel that was compiled
     */
    virtual void notifyObserversKernelCompiled(const cl::Kernel* kernel);
protected:
    Processor* processor_;
};

} // namespace

#endif // IVW_KERNEL_OWNER_H
