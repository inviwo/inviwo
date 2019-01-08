/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#include <modules/opencl/kernelowner.h>
#include <modules/opencl/kernelmanager.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/processors/processor.h>

namespace inviwo {

void KernelObservable::notifyObserversKernelCompiled(const cl::Kernel* kernel) {
    forEachObserver([&](KernelObserver* o) { o->onKernelCompiled(kernel); });
}

cl::Kernel* KernelOwner::addKernel(const std::string& fileName, const std::string& kernelName,
                                   const std::string& header, const std::string& defines) {
    if (fileName.length() > 0) {
        bool wasBuilt;
        cl::Program* program =
            KernelManager::getPtr()->buildProgram(fileName, header, defines, wasBuilt);

        cl::Kernel* kernel = KernelManager::getPtr()->getKernel(program, kernelName, this);
        if (kernel) {
            kernels_.insert(kernel);
        }
        return kernel;

    } else {
        return nullptr;
    }
}

KernelOwner::~KernelOwner() {
    // Make sure that we are not notifed after destruction
    KernelManager::getPtr()->stopObservingKernels(this);
}

void KernelOwner::removeKernel(cl::Kernel* kernel) {
    auto kernelIt = kernels_.find(kernel);
    if (kernelIt != kernels_.end()) {
        kernels_.erase(kernels_.find(kernel));
        KernelManager::getPtr()->stopObservingKernel(kernel, this);
    }
}

ProcessorKernelOwner::ProcessorKernelOwner(Processor* processor)
    : KernelOwner(), processor_(processor) {}

void ProcessorKernelOwner::notifyObserversKernelCompiled(const cl::Kernel* kernel) {
    KernelOwner::notifyObserversKernelCompiled(kernel);
    processor_->invalidate(InvalidationLevel::InvalidResources);
}

void KernelObserver::onKernelCompiled(const cl::Kernel*) {}

}  // namespace inviwo
