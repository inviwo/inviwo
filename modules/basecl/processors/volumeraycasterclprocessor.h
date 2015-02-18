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

#ifndef IVW_VOLUMERAYCASTERCLPROCESSOR_H
#define IVW_VOLUMERAYCASTERCLPROCESSOR_H

#include <modules/basecl/baseclmoduledefine.h>
#include <modules/basecl/volumeraycastercl.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>



#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/image/layerclbase.h>
#include <modules/opencl/kernelowner.h>
#include <modules/opencl/volume/volumeclbase.h>

namespace inviwo {
/** \docpage{org.inviwo.VolumeRaycasterCL, Volume Raycaster}
 * Perform volume rendering on the input volume. 
 * ### Inports
 *   * __VolumeInport__ The volume data to render.
 *   * __ImageInport__ The entry point.
 *   * __ImageInport__ The exit point.
 *
 * ### Outports
 *   * __ImageOutport__ Light reaching the camera through the volume.
 * 
 * ### Properties
 *   * __Sampling rate__ Number of sample per voxel to take.
 *   * __Transfer function__ Transfer function to map data values into color and opacity.
 *   * __Work group size__ OpenCL work group size (performance)
 *   * __Use OpenGL sharing__ Share data with OpenGL (performance and compability).
 */

/**
 * \brief Perform volume rendering on the input volume. 
 *
 */
class IVW_MODULE_BASECL_API VolumeRaycasterCLProcessor : public Processor, public KernelObserver {
public:
    VolumeRaycasterCLProcessor();
    ~VolumeRaycasterCLProcessor();

    InviwoProcessorInfo();

    void initialize();
    void deinitialize();

    virtual bool isReady() const;

    void onKernelCompiled( const cl::Kernel* kernel ) {
        invalidate(INVALID_RESOURCES);
    }
protected:
    virtual void process();

private:
    void onParameterChanged();
    VolumeInport volumePort_;
    ImageInport entryPort_;
    ImageInport exitPort_;
    ImageOutport outport_;

    FloatProperty samplingRate_;
    TransferFunctionProperty transferFunction_;
    IntVec2Property workGroupSize_;

    BoolProperty useGLSharing_;

    SimpleLightingProperty lighting_;
    CameraProperty camera_;

    VolumeRaycasterCL volumeRaycaster_;
};

} // namespace

#endif // IVW_VOLUMERAYCASTERCLPROCESSOR_H
