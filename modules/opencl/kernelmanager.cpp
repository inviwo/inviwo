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

#include <modules/opencl/kernelmanager.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/opengl/openglmodule.h>
#include <modules/opencl/kernelowner.h>

namespace inviwo {

KernelManager::KernelManager() {
    InviwoApplication::getPtr()->registerFileObserver(this);
}

KernelManager::~KernelManager() {
    clear();
}

cl::Program* KernelManager::buildProgram(const std::string& fileName, const std::string& defines /*= ""*/, bool& wasBuilt) {
    wasBuilt = false;
    std::string absoluteFileName = fileName;
    if (!filesystem::fileExists(absoluteFileName)) {
        // Search in include directories added by modules
        const std::vector<std::string> openclSearchPaths = OpenCL::getPtr()->getCommonIncludeDirectories();

        for (size_t i=0; i<openclSearchPaths.size(); i++) {
            if (filesystem::fileExists(openclSearchPaths[i]+"/"+fileName)) {
                absoluteFileName = openclSearchPaths[i]+"/"+fileName;
                break;
            }
        }
    }

    std::pair <ProgramMap::iterator, ProgramMap::iterator> range = programs_.equal_range(absoluteFileName);

    for (ProgramMap::iterator it = range.first; it != range.second; ++it) {
        if (it->second.defines == defines) {
            return it->second.program;
        }
    }

    cl::Program* program = new cl::Program();

    try {
        *program = cl::Program(OpenCL::buildProgram(absoluteFileName, defines));

        try {
            std::vector<cl::Kernel> kernels;
            program->createKernels(&kernels);

            for (std::vector<cl::Kernel>::iterator kernelIt = kernels.begin(); kernelIt != kernels.end(); ++kernelIt) {
                kernels_.insert(std::pair<cl::Program*, cl::Kernel*>(program, new cl::Kernel(*kernelIt)));
            }
        } catch (cl::Error& err) {
            LogError(absoluteFileName << " Failed to create kernels, Error:" << err.what() << "(" << err.err() << "), " << errorCodeToString(
                         err.err()) << std::endl);
        }
    } catch (cl::Error&) {
    }

    ProgramIdentifier uniqueProgram;
    uniqueProgram.defines = defines;
    uniqueProgram.program = program;
    programs_.insert(std::pair<std::string, ProgramIdentifier>(absoluteFileName, uniqueProgram));
    startFileObservation(absoluteFileName);
    wasBuilt = true;
    return program;
}

cl::Kernel* KernelManager::getKernel(cl::Program* program, const std::string& kernelName, KernelOwner* owner) {
    std::pair<KernelMap::iterator, KernelMap::iterator> kernelRange = kernels_.equal_range(program);
    
    for (KernelMap::iterator kernelIt = kernelRange.first; kernelIt != kernelRange.second; ++kernelIt) {
        std::string thisKernelName;
        kernelIt->second->getInfo(CL_KERNEL_FUNCTION_NAME, &thisKernelName);

        if (thisKernelName == kernelName) {
            // Add KernelOwner and Kernel pair to map if owner is not NULL
            if (owner != NULL)
                kernelOwners_.insert(std::pair<cl::Kernel*, KernelOwner*>(kernelIt->second, owner));
            return kernelIt->second;
        }
    }
    LogError("Failed to find kernel:" << kernelName << std::endl);
    return NULL;
}

void KernelManager::fileChanged(std::string fileName) {
    std::pair <ProgramMap::iterator, ProgramMap::iterator> programRange = programs_.equal_range(fileName);

    for (ProgramMap::iterator programIt = programRange.first; programIt != programRange.second; ++programIt) {
        cl::Program* program = programIt->second.program;
        // Get all kernels associated with the program
        std::pair <KernelMap::iterator, KernelMap::iterator> kernelRange = kernels_.equal_range(program);
        std::vector<std::string> kernelNames;

        for (KernelMap::iterator kernelIt = kernelRange.first; kernelIt != kernelRange.second; ++kernelIt) {
            std::string thisKernelName;

            try {
                kernelIt->second->getInfo(CL_KERNEL_FUNCTION_NAME, &thisKernelName);
            } catch (cl::Error&) {
            }

            kernelNames.push_back(thisKernelName);
        }

        try {
            LogInfo(fileName + " building program with defines: " + programIt->second.defines);
            *program = OpenCL::buildProgram(fileName, programIt->second.defines);
            LogInfo(fileName + " finished building program");
            std::vector<cl::Kernel> newKernels;

            try {
                LogInfo(fileName + " creating kernels");
                program->createKernels(&newKernels);
            } catch (cl::Error& err) {
                LogError(fileName << " Failed to create kernels, error:" << err.what() << "(" << err.err() << "), " << errorCodeToString(
                             err.err()) << std::endl);
                throw err;
            }

            // Find corresponding old kernel for each newly compiled kernel.
            // Add kernel if it was not found
            for (std::vector<cl::Kernel>::iterator newKernelIt = newKernels.begin(); newKernelIt != newKernels.end(); ++newKernelIt) {
                std::string newKernelName;
                newKernelIt->getInfo(CL_KERNEL_FUNCTION_NAME, &newKernelName);
                std::vector<std::string>::iterator kernelNameIt = std::find(kernelNames.begin(), kernelNames.end(), newKernelName);

                if (kernelNameIt != kernelNames.end()) {
                    size_t index = kernelNameIt - kernelNames.begin();
                    KernelMap::iterator oldKernelIt(kernelRange.first);

                    for (size_t j = 0; j < index; ++j, ++oldKernelIt) {};

                    *(oldKernelIt->second) = *newKernelIt;
                    // Notify that kernel has been recompiled
                    std::pair<KernelOwnerMap::iterator, KernelOwnerMap::iterator> kernelOwnerRange = kernelOwners_.equal_range(oldKernelIt->second);
                    for (KernelOwnerMap::iterator kernelOwnerIt = kernelOwnerRange.first; kernelOwnerIt != kernelOwnerRange.second; ++kernelOwnerIt) {
                        kernelOwnerIt->second->notifyObserversKernelCompiled(&(*newKernelIt));
                    }
                } else {
                    // New kernel, no need to notify KernelOwner since there cannot be any
                    kernels_.insert(std::pair<cl::Program*, cl::Kernel*>(program, new cl::Kernel(*newKernelIt)));
                }

            }

            InviwoApplication::getPtr()->playSound(InviwoApplication::IVW_OK);

            std::vector<Processor*> processors = InviwoApplication::getPtr()->getProcessorNetwork()->getProcessors();

        } catch (cl::Error& err) {
            LogError(fileName << " Failed to create kernels, error:" << err.what() << "(" << err.err() << "), " << errorCodeToString(
                         err.err()) << std::endl);
            InviwoApplication::getPtr()->playSound(InviwoApplication::IVW_ERROR);
        }
    }
}

void KernelManager::clear() {
    for (ProgramMap::iterator programIt = programs_.begin(); programIt != programs_.end(); ++programIt) {
        InviwoApplication::getPtr()->stopFileObservation(programIt->first);
    }

    for (KernelMap::iterator kernelIt = kernels_.begin(); kernelIt != kernels_.end(); ++kernelIt) {
        delete kernelIt->second;
        kernelIt->second = NULL;
    }

    for (ProgramMap::iterator programIt = programs_.begin(); programIt != programs_.end(); ++programIt) {
        delete programIt->second.program;
        programIt->second.program = NULL;
    }
    kernelOwners_.clear();
}

void KernelManager::stopObservingKernel(cl::Kernel* kernel, KernelOwner* owner) {
    std::pair<KernelOwnerMap::iterator, KernelOwnerMap::iterator> startEnd = kernelOwners_.equal_range(kernel);
    for (KernelOwnerMap::iterator it = startEnd.first; it != startEnd.second; ++it) {
        if (it->second == owner) {
            kernelOwners_.erase(it);
            return;
        }
    }
}

void KernelManager::stopObservingKernels( KernelOwner* owner ) {
    std::vector<KernelOwnerMap::iterator> toBeErased;
    for (KernelOwnerMap::iterator kernelOwnerIt = kernelOwners_.begin(); kernelOwnerIt != kernelOwners_.end(); ++kernelOwnerIt) {
        if (kernelOwnerIt->second == owner) {
            toBeErased.push_back(kernelOwnerIt);
        }
    }
    for (std::vector<KernelOwnerMap::iterator>::iterator it=toBeErased.begin(); it!=toBeErased.end(); ++it) {
        kernelOwners_.erase(*it);
    }
}




} // namespace
