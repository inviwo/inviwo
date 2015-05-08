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

#include "runningimagemeanandstandarddeviationcl.h"
#include <modules/opencl/image/layercl.h>
#include <modules/opencl/image/layerclgl.h>
#include <modules/opencl/syncclgl.h>

namespace inviwo {

RunningImageMeanAndStandardDeviationCL::RunningImageMeanAndStandardDeviationCL(const uvec2& layerDimension, const svec2& workgroupSize)
    : pingPongIndex_(0)
    , workGroupSize_(workgroupSize)
    , kernel_(nullptr) {
    standardDeviation_[0] = Layer(layerDimension, DataVec4FLOAT32::get());
    standardDeviation_[1] = Layer(layerDimension, DataVec4FLOAT32::get());
    mean_[0] = Layer(layerDimension, DataVec4FLOAT32::get());
    mean_[1] = Layer(layerDimension, DataVec4FLOAT32::get());
    kernel_ = addKernel("statistics/runningmeanandstandarddeviationkernel.cl", "runningMeanAndStandardDeviationKernel");
}

bool RunningImageMeanAndStandardDeviationCL::computeMeanAndStandardDeviation(const Layer* newSamples, int iteration, Layer*& outMean, Layer*& outStandardDeviation, bool useGLSharing, const VECTOR_CLASS<cl::Event> *waitForEvents, cl::Event *event) {
    if (kernel_ == nullptr) {
        return false;
    }
    if (glm::any(glm::notEqual(newSamples->getDimensions(), standardDeviation_[0].getDimensions()))) {
        standardDeviation_[0].setDimensions(newSamples->getDimensions());
        standardDeviation_[1].setDimensions(newSamples->getDimensions());
        mean_[0].setDimensions(newSamples->getDimensions());
        mean_[1].setDimensions(newSamples->getDimensions());
    }

    //IVW_OPENCL_PROFILING(profilingEvent, "")
    int prevStdId = pingPongIndex_;
    int nextStdId = (pingPongIndex_ + 1) % 2;
    try {
        if (useGLSharing) {
            SyncCLGL glSync;
            
            const LayerCLGL* samples = newSamples->getRepresentation<LayerCLGL>();
            LayerCLGL* prevMeanCL = mean_[prevStdId].getEditableRepresentation<LayerCLGL>();
            LayerCLGL* nextMeanCL = mean_[nextStdId].getEditableRepresentation<LayerCLGL>();
            LayerCLGL* prevStandardDeviation = standardDeviation_[prevStdId].getEditableRepresentation<LayerCLGL>();
            LayerCLGL* nextStandardDeviation = standardDeviation_[nextStdId].getEditableRepresentation<LayerCLGL>();

            // Acquire shared representations before using them in OpenGL
            // The SyncCLGL object will take care of synchronization between OpenGL and OpenCL
            glSync.addToAquireGLObjectList(samples);
            glSync.addToAquireGLObjectList(prevMeanCL);
            glSync.addToAquireGLObjectList(nextMeanCL);
            glSync.addToAquireGLObjectList(prevStandardDeviation);
            glSync.addToAquireGLObjectList(nextStandardDeviation);

            glSync.aquireAllObjects();
            computeMeanAndStandardDeviation(newSamples->getDimensions(), samples, iteration, prevMeanCL, nextMeanCL, prevStandardDeviation, nextStandardDeviation, workGroupSize_, waitForEvents, event);
        } else {
            LayerCL* prevMeanCL = mean_[prevStdId].getEditableRepresentation<LayerCL>();
            LayerCL* nextMeanCL = mean_[nextStdId].getEditableRepresentation<LayerCL>();
            const LayerCL* samples = newSamples->getRepresentation<LayerCL>();
            LayerCL* prevStandardDeviation = standardDeviation_[prevStdId].getEditableRepresentation<LayerCL>();
            LayerCL* nextStandardDeviation = standardDeviation_[nextStdId].getEditableRepresentation<LayerCL>();
            computeMeanAndStandardDeviation(newSamples->getDimensions(), samples, iteration, prevMeanCL, nextMeanCL, prevStandardDeviation, nextStandardDeviation, workGroupSize_, waitForEvents, event);
        }
    } catch (cl::Error& err) {
        LogError(getCLErrorString(err));
        return false;
    }
    pingPongIndex_ = nextStdId;

    outMean = &mean_[nextStdId];
    outStandardDeviation = &standardDeviation_[nextStdId];
    return true;
}

void RunningImageMeanAndStandardDeviationCL::computeMeanAndStandardDeviation(const uvec2& nSamples, const LayerCLBase* samples, int iteration, const LayerCLBase* prevMean, LayerCLBase* nextMean, const LayerCLBase* prevStandardDeviation, LayerCLBase* nextStandardDeviation, const svec2& workGroupSize, const VECTOR_CLASS<cl::Event> *waitForEvents, cl::Event *event) {
    
    
    size_t workGroupSizeX = static_cast<size_t>(workGroupSize.x); size_t workGroupSizeY = static_cast<size_t>(workGroupSize.y);
    size_t globalWorkSizeX = getGlobalWorkGroupSize(nSamples.x, workGroupSizeX);
    size_t globalWorkSizeY = getGlobalWorkGroupSize(nSamples.y, workGroupSizeY);

    int argIndex = 0;
    kernel_->setArg(argIndex++, nSamples);
    kernel_->setArg(argIndex++, *samples);
    kernel_->setArg(argIndex++, static_cast<float>(iteration+1));
    kernel_->setArg(argIndex++, *prevMean);
    kernel_->setArg(argIndex++, *nextMean);
    kernel_->setArg(argIndex++, *prevStandardDeviation);
    kernel_->setArg(argIndex++, *nextStandardDeviation);
    OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(*kernel_, cl::NullRange, svec2(globalWorkSizeX, globalWorkSizeY) , svec2(workGroupSizeX, workGroupSizeY), waitForEvents, event);
}

} // namespace

