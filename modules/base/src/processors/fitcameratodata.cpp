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

#include <modules/base/processors/fitcameratodata.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo FitCameraToData::processorInfo_{
    "org.inviwo.FitCameraToData",  // Class identifier
    "Fit Camera To Data",          // Display name
    "Utility",                     // Category
    CodeState::Stable,             // Code state
    Tags::CPU,                     // Tags
};
const ProcessorInfo FitCameraToData::getProcessorInfo() const { return processorInfo_; }

FitCameraToData::FitCameraToData()
    : Processor()
    , volume_("volume_")
    , mesh_("mesh")
    , meshMultiInport_("meshMultiInport")
    , meshFlatMultiInport_("meshFlatMultiInport")
    , camera_("camera", "Camera")
    , fitterVolume_("fitterVolume", "Fit Camera To Volume", camera_, volume_)
    , fitterMesh_("fitterMesh", "Fit Camera To Mesh", camera_, mesh_)
    , fitterMeshMulti_("fitterMeshMulti", "Fit Camera To Multi Mesh", camera_, meshMultiInport_)
    , fitterMeshFlat_("fitterMeshFlat", "Fit Camera To Multi Flat Mesh", camera_,
                      meshFlatMultiInport_)

{

    volume_.setOptional(true);
    mesh_.setOptional(true);
    meshMultiInport_.setOptional(true);
    meshFlatMultiInport_.setOptional(true);

    fitterVolume_.getCompositeProperty().setVisible(false);
    fitterMesh_.getCompositeProperty().setVisible(false);
    fitterMeshMulti_.getCompositeProperty().setVisible(false);
    fitterMeshFlat_.getCompositeProperty().setVisible(false);

    fitterVolume_.getCompositeProperty().setCollapsed(true);
    fitterMesh_.getCompositeProperty().setCollapsed(true);
    fitterMeshMulti_.getCompositeProperty().setCollapsed(true);
    fitterMeshFlat_.getCompositeProperty().setCollapsed(true);

    fitterVolume_.getCompositeProperty().setCurrentStateAsDefault();
    fitterMesh_.getCompositeProperty().setCurrentStateAsDefault();
    fitterMeshMulti_.getCompositeProperty().setCurrentStateAsDefault();
    fitterMeshFlat_.getCompositeProperty().setCurrentStateAsDefault();

    volume_.onConnect([this] { fitterVolume_.getCompositeProperty().setVisible(true); });
    volume_.onDisconnect([this] { fitterVolume_.getCompositeProperty().setVisible(false); });

    mesh_.onConnect([this] { fitterMesh_.getCompositeProperty().setVisible(true); });
    mesh_.onDisconnect([this] { fitterMesh_.getCompositeProperty().setVisible(false); });

    meshMultiInport_.onConnect(
        [this] { fitterMeshMulti_.getCompositeProperty().setVisible(true); });
    meshMultiInport_.onDisconnect(
        [this] { fitterMeshMulti_.getCompositeProperty().setVisible(false); });

    meshFlatMultiInport_.onConnect(
        [this] { fitterMeshFlat_.getCompositeProperty().setVisible(true); });
    meshFlatMultiInport_.onDisconnect(
        [this] { fitterMeshFlat_.getCompositeProperty().setVisible(false); });

    addPort(volume_);
    addPort(mesh_);
    addPort(meshMultiInport_);
    addPort(meshFlatMultiInport_);
    addProperties(camera_, fitterVolume_.getCompositeProperty(), fitterMesh_.getCompositeProperty(),
                  fitterMeshMulti_.getCompositeProperty(), fitterMeshFlat_.getCompositeProperty());

    camera_.setCollapsed(true);
}

void FitCameraToData::process() {}

}  // namespace inviwo
