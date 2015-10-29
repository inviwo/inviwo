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

#ifndef IVW_ENTRYEXITPOINTSCLPROCESSOR_H
#define IVW_ENTRYEXITPOINTSCLPROCESSOR_H

#include <modules/basecl/baseclmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/kernelowner.h>
#include <modules/basecl/meshentryexitpointscl.h>

namespace inviwo {
/** \docpage{org.inviwo.EntryExitPointsCL, Entry-exit points}
 * ![](org.inviwo.EntryExitPointsCL.png?classIdentifier=org.inviwo.EntryExitPointsCL)
 * Computes the entry and exit points of a triangle mesh from the camera position in texture space. 
 * The output color will be zero if no intersection is found, otherwise .
 * ### Inports
 *   * __MeshInport__ The mesh to intersect.
 *
 * ### Outports
 *   * __ImageOutport__ The first hit point.
 *   * __ImageOutport__ The last hit point.
 * 
 * ### Properties
 *   * __Camera__ Camera of the scene.
 *   * __Work group size__ OpenCL work group size (performance)
 *   * __Use OpenGL sharing__ Share data with OpenGL (performance and compability).
 */

/**
 * \brief Computes the entry and exit points of a triangle mesh from the camera position in texture space.
 *
 */
class IVW_MODULE_BASECL_API EntryExitPointsCLProcessor : public Processor, public KernelObserver {
public:
    EntryExitPointsCLProcessor();
    virtual ~EntryExitPointsCLProcessor();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void initialize() override;
    virtual void deinitialize() override;

protected:
    virtual void process() override;
    void onKernelCompiled( const cl::Kernel* kernel ) override { invalidate(InvalidationLevel::InvalidResources); }



private:
    MeshInport geometryPort_;
    ImageOutport entryPort_;
    ImageOutport exitPort_;

    CameraProperty camera_;
    IntVec2Property workGroupSize_;
    BoolProperty useGLSharing_;

    CameraTrackball trackball_;

    MeshEntryExitPointsCL entryExitPoints_;
};

}  // namespace

#endif  // IVW_ENTRYEXITPOINTSCLPROCESSOR_H
