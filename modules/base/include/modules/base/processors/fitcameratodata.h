/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/meshport.h>

#include <modules/base/algorithm/camerautils.h>

namespace inviwo {

/** \docpage{org.inviwo.FitCameraToData, Fit Camera To Data}
 * ![](org.inviwo.FitCameraToData.png?classIdentifier=org.inviwo.FitCameraToData)
 *
 * Processor show how camerautil::FitCameraPropertiesHelper can be used to update a camera based on
 * data on volume/mesh ports.
 *
 *
 * ### Inports
 *   * __volume___ Describe port.
 *   * __mesh__ Describe port.
 *   * __meshMultiInport__ Describe port.
 *   * __meshFlatMultiInport__ Describe port.
 *
 * ### Properties
 *   * __Camera__ The camera that will be updated, can be linked to other processors.
 *   * __Fit Camera To Volume__ Contains buttons to update the camera based on the volume on the
 * volume inport.
 *   * __Fit Camera To Mesh__ Contains buttons to update the camera based on the mesh in the mesh
 * inport.
 *   * __Fit Camera To Multi Mesh__ Contains buttons to update the camera based on the meshes in the
 * multi mesh inport.
 *   * __Fit Camera To Multi Flat Mesh__ Contains buttons to update the camera based on the meshes
 * in the flat multi mesh inport.
 */

class IVW_MODULE_BASE_API FitCameraToData : public Processor {
public:
    FitCameraToData();
    virtual ~FitCameraToData() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport volume_;
    MeshInport mesh_;
    MeshMultiInport meshMultiInport_;
    MeshFlatMultiInport meshFlatMultiInport_;
    CameraProperty camera_;

    camerautil::FitCameraPropertiesHelper fitterVolume_;
    camerautil::FitCameraPropertiesHelper fitterMesh_;
    camerautil::FitCameraPropertiesHelper fitterMeshMulti_;
    camerautil::FitCameraPropertiesHelper fitterMeshFlat_;
};

}  // namespace inviwo
