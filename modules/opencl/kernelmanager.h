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

#ifndef IVW_KERNEL_MANAGER_H
#define IVW_KERNEL_MANAGER_H

#include <modules/opencl/openclmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/singleton.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/kernelowner.h>
#include <map>

namespace inviwo {

class IVW_MODULE_OPENCL_API KernelManager : public Singleton<KernelManager>, public FileObserver {
    /** \class KernelManager
    *
    * Manages building of OpenCL programs and kernels.
    * Reloads and builds program when file changes. Notifies processor when the program has been rebuilt.
    */
    // TODO: Parallel building of program.
    // TODO: Make sure that processor is not evaluated while building or build failed.
public:
    // Multiple programs can be created from the same file but with different defines
    struct ProgramIdentifier {
        cl::Program* program;
        std::string defines;
    };
    typedef std::multimap<std::string, ProgramIdentifier> ProgramMap; ///< File name and unique identifier for program
    typedef std::multimap<cl::Program*, cl::Kernel*> KernelMap; ///< All kernels belonging to a program
    typedef std::multimap<cl::Kernel*, KernelOwner*> KernelOwnerMap; ///< Owners of the kernels, enables invalidation of owner when kernel changed

    KernelManager();
    virtual ~KernelManager();
    /**
     * Creates and builds an OpenCL program. Will automatically reload the program and
     * kernels when the file changes.
     * Only provide the file name as the file will be searched for in all the include paths.
     * The KernelManager will make sure that the processor is not evaluated if building fails.
     * @note KernelManager manages pointer memory, do not delete it.
     *
     * @param fileName (const std::string &) File name
     * @param defines (const std::string &) Compiler defines
     * @return (cl::Program*) Pointer to a program no matter if it was succesfully built or not.
     */
    cl::Program* buildProgram(const std::string& fileName, const std::string& defines = "");


    /**
     * Creates a kernel from a previously created cl::Program.
     * Makes sure that it is up to date when program is rebuilt.
     *
     * @see cl::Program, cl::Kernel
     * @param program The program containing the Kernel
     * @param kernelName Name of kernel
     * @return (cl::Kernel*) Pointer to cl::Kernel, NULL if not found. KernelManager manages memory of Kernel, do not delete it.
     */
    cl::Kernel* getKernel(cl::Program* program, const std::string& kernelName, KernelOwner* owner);

    void stopObservingKernels(KernelOwner* owner);

    /**
     * Reloads programs from file and notifies processors.
     *
     */
    virtual void fileChanged(std::string fileName);

    /**
     * Remove all kernels and stop observing the files.
     * DO NOT call this function when kernels are being used!
     */
    void clear();

private:
    ProgramMap programs_;
    KernelMap kernels_;
    KernelOwnerMap kernelOwners_;
};

} // namespace

#endif // IVW_KERNEL_MANAGER_H
