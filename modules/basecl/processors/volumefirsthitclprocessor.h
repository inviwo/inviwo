/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_VOLUME_FIRST_HIT_CL_H
#define IVW_VOLUME_FIRST_HIT_CL_H

#include <modules/basecl/baseclmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/kernelowner.h>

namespace inviwo {
/** \docpage{org.inviwo.VolumeFirstHitCL, Volume First Hit}
 * Computes the first point with non-zero opacity within a volume given entry and exit points in texture space. 
 * ### Inports
 *   * __VolumeInport__ The volume to intersect.
 *   * __ImageInport__ The entry point.
 *   * __ImageInport__ The exit point.
 *
 * ### Outports
 *   * __ImageOutport__ The first hit point.
 * 
 * ### Properties
 *   * __Sampling rate__ Number of sample per voxel to take.
 *   * __Transfer function__ Transfer function to map data values into color and opacity.
 *   * __Work group size__ OpenCL work group size (performance)
 *   * __Use OpenGL sharing__ Share data with OpenGL (performance and compability).
 */

/**
 * \brief Computes the first point with non-zero opacity within a volume given entry and exit points in texture space. 
 *
 */
class IVW_MODULE_BASECL_API VolumeFirstHitCLProcessor : public Processor, public ProcessorKernelOwner {
public:
    VolumeFirstHitCLProcessor();
    ~VolumeFirstHitCLProcessor();

    InviwoProcessorInfo();

    void initialize();
    void deinitialize();

protected:
    virtual void process();

    void firstHit(const cl::Image& volumeCL, const cl::Image& entryPoints,
                  const cl::Image& exitPoints, const cl::Image& transferFunctionCL,
                  const cl::Image& output, float stepSize, svec2 outportDim,
                  svec2 localWorkGroupSize, cl::Event* profilingEvent);

private:
    VolumeInport volumePort_;
    ImageInport entryPort_;
    ImageInport exitPort_;
    ImageOutport outport_;

    FloatProperty samplingRate_;
    TransferFunctionProperty transferFunction_;
    IntVec2Property workGroupSize_;
    BoolProperty useGLSharing_;

    cl::Kernel* kernel_;
};

}  // namespace

#endif  // IVW_VOLUME_FIRST_HIT_CL_H
