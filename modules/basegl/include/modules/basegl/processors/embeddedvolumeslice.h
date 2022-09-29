/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>                // for IVW_MODULE_BASEGL_API

#include <inviwo/core/datastructures/geometry/typedmesh.h>    // for PositionsBuffer, TypedMesh
#include <inviwo/core/interaction/cameratrackball.h>          // for CameraTrackball
#include <inviwo/core/interaction/pickingmapper.h>            // for PickingMapper
#include <inviwo/core/ports/imageport.h>                      // for ImageInport, ImageOutport
#include <inviwo/core/ports/volumeport.h>                     // for VolumeInport
#include <inviwo/core/processors/processor.h>                 // for Processor
#include <inviwo/core/processors/processorinfo.h>             // for ProcessorInfo
#include <inviwo/core/properties/cameraproperty.h>            // for CameraProperty
#include <inviwo/core/properties/ordinalproperty.h>           // for FloatVec3Property
#include <inviwo/core/properties/transferfunctionproperty.h>  // for TransferFunctionProperty
#include <modules/opengl/shader/shader.h>                     // for Shader

namespace inviwo {
class PickingEvent;

/** \docpage{org.inviwo.EmbeddedVolumeSlice, Embedded Volume Slice}
 * ![](org.inviwo.EmbeddedVolumeSlice.png?classIdentifier=org.inviwo.EmbeddedVolumeSlice)
 *
 * Render an arbitrary slice of a volume in place, i.e. the slice will be oriented as it would
 * have been in the volume.
 *
 * ### Inports
 *   * __volume__ The input volume
 *   * __background__ Optional background image
 * ### Outports
 *   * __outport__ Rendered slice
 *
 * ### Properties
 *   * __Plane Normal__ Defines the normal of the plane in texture/data space [0,1]
 *   * __Plane Position__ Defines a point in the plane in texture/data space [0,1]
 *   * __Transfer Function__ Defines the transfer function for mapping voxel values to color and
 *                           opacity
 *   * __Camera__ Camera used for rendering
 *   * __Trackball__ Trackball for handling interaction
 */

class IVW_MODULE_BASEGL_API EmbeddedVolumeSlice : public Processor {
public:
    EmbeddedVolumeSlice();
    virtual ~EmbeddedVolumeSlice() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void initializeResources() override;

protected:
    virtual void process() override;
    void planeSettingsChanged();
    void handlePicking(PickingEvent* p);

private:
    VolumeInport inport_;
    ImageInport backgroundPort_;
    ImageOutport outport_;
    Shader shader_;

    FloatVec3Property planeNormal_;
    FloatVec3Property planePosition_;

    TransferFunctionProperty transferFunction_;

    CameraProperty camera_;
    CameraTrackball trackball_;

    TypedMesh<buffertraits::PositionsBuffer> embeddedMesh_;
    PickingMapper picking_;
};

}  // namespace inviwo
