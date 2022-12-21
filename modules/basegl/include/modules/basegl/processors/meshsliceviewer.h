/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#pragma once

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/rendering/meshdrawer.h>
#include <modules/opengl/shader/shader.h>


namespace inviwo {

/** \docpage{org.inviwo.MeshSliceViewer, Mesh Slice Viewer}
 * ![](org.inviwo.MeshSliceViewer.png?classIdentifier=org.inviwo.MeshSliceViewer)
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __volumeIn___ Volume inport to get volume meta info.
 *   * __inport___ Mesh in to cut with planes.
 *
 * ### Outports
 *   * __meshOut___ Sliced Mesh.
 *
 * ### Properties
 *   * __position___ Plane position, connect to volume slicing plane positioin
 *   * __normal___ Plane normal, connect to volume slicing plane normal
 *   * __sliceResolution___ Resolution of the Canvas showing slices
 *   * __camera___ Camera property to link to Mesh renderer for the sliced mesh
 */
class IVW_MODULE_BASEGL_API MeshSliceViewer : public Processor {
public:
    MeshSliceViewer();
    virtual ~MeshSliceViewer() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport volumeIn_;
    MeshInport inport_;
    MeshOutport meshOut_;
    FloatVec3Property position_;
    FloatVec3Property normal_;
    IntSize2Property sliceResolution_;
    CameraProperty camera_;

    void planeSettingsChanged();

    mat4 sliceRotation_;
    mat4 inverseSliceRotation_;
};

}  // namespace inviwo
