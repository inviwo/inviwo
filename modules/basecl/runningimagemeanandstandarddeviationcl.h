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

#ifndef IVW_RUNNINGIMAGESTANDARDDEVIATIONCL_H
#define IVW_RUNNINGIMAGESTANDARDDEVIATIONCL_H

#include <modules/basecl/baseclmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/image/layerclbase.h>
#include <modules/opencl/kernelowner.h>

namespace inviwo {
/** \class RunningImageMeanAndStandardDeviationCL
 *
 * Computes per-pixel standard deviation and mean value.
 * Can be used both with external data and by letting the class manage 
 * the temporary and current data.
 * http://en.wikipedia.org/wiki/Standard_deviation
 */
class IVW_MODULE_BASECL_API RunningImageMeanAndStandardDeviationCL : public KernelOwner { 
public:
    /** 
     * \brief Initiate with layer dimension and work group size. 
     *
     * Managed layers will be resized if a different dimension is used during computation.
     * 
     * @param layerDimension    Initial guess on layer dimension
     * @param workgroupSize     Work group size 
     */
    RunningImageMeanAndStandardDeviationCL(const uvec2& layerDimension = uvec2(32), const size2_t& workgroupSize = uvec2(16));
    virtual ~RunningImageMeanAndStandardDeviationCL(){}

    virtual void onKernelCompiled(const cl::Kernel* kernel) {};


    /** 
     * \brief Computes mean and standard deviation of given new values and iteration number.
     *
     * Computes mean and standard deviation of all channels (RBGA) separately.
     * This class manages mean and standard deviation layers. 
     * Output pointers are only valid if true is returned.
     * 
     * @param newSamples                New samples to be used for computation.
     * @param iteration                 Current iteration number, 0 being the first
     * @param outMean                   Will contain mean value upon return. This class owns the data.
     * @param outStandardDeviation      Will contain standard deviation upon return. This class owns the data.
     * @param useGLSharing              Specifies if OpenCL/OpenGL sharing be used.
     * @param waitForEvents             Events to wait for before starting.
     * @param event                     Event that will be signaled on completion.
     * @return bool                     True if successful, false otherwise.
     */
    bool computeMeanAndStandardDeviation(const Layer* newSamples, int iteration, Layer*& outMean, Layer*& outStandardDeviation, bool useGLSharing, const VECTOR_CLASS<cl::Event> *waitForEvents = nullptr, cl::Event *event = nullptr);

    /** 
     * \brief Computes mean and standard deviation of given new values and iteration number without managing the data 
     * or OpenCL/OpenGL synchronization.
     *
     * Computes mean and standard deviation of all channels (RBGA) separately.
     * Output pointers are only valid if true is returned.
     * 
     * @param nSamples                  Dimension if samples layer
     * @param newSamples                New samples to be used for computation.
     * @param iteration                 Current iteration number, 0 being the first
     * @param prevMean                  Mean value of previous iteration
     * @param nextMean                  Mean value after computation is done
     * @param prevStandardDeviation     Standard deviation of previous iteration
     * @param nextStandardDeviation     Standard deviation after computation is done
     * @param workGroupSize             Work group size to be used during computation
     * @param waitForEvents             Events to wait for before starting.
     * @param event                     Event that will be signaled on completion.
     * @return void 
     */
    void computeMeanAndStandardDeviation(const uvec2& nSamples, const LayerCLBase* newSamples, int iteration, const LayerCLBase* prevMean, LayerCLBase* nextMean, const LayerCLBase* prevStandardDeviation, LayerCLBase* nextStandardDeviation, const size2_t& workGroupSize, const VECTOR_CLASS<cl::Event> *waitForEvents = nullptr, cl::Event *event = nullptr);

    size2_t WorkGroupSize() const { return workGroupSize_; }
    void WorkGroupSize(size2_t val) { workGroupSize_ = val; }
private:
    Layer standardDeviation_[2]; ///< Standard deviation for each pixel of previous and current iteration. 
    Layer mean_[2]; ///< Mean value for each pixel of previous and current iteration. 
    int pingPongIndex_;
    size2_t workGroupSize_;

    cl::Kernel* kernel_;
};

} // namespace

#endif // IVW_RUNNINGIMAGESTANDARDDEVIATIONCL_H

