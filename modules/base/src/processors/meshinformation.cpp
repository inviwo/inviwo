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

#include <modules/base/processors/meshinformation.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshInformation::processorInfo_{
    "org.inviwo.MeshInformation",        // Class identifier
    "Mesh Information",                  // Display name
    "Information",                       // Category
    CodeState::Stable,                   // Code state
    "CPU, Mesh, Geometry, Information",  // Tags
};
const ProcessorInfo MeshInformation::getProcessorInfo() const { return processorInfo_; }

MeshInformation::MeshInformation()
    : Processor()
    , mesh_("mesh")
    , meshInfo_("dataInformation", "Data Information")
    , metaDataProperty_("metaData", "Meta Data") {

    addPort(mesh_);
    addProperty(meshInfo_);
    addProperty(metaDataProperty_);

    meshInfo_.setSerializationMode(PropertySerializationMode::None);

    setAllPropertiesCurrentStateAsDefault();
}

void MeshInformation::process() {
    auto mesh = mesh_.getData();

    meshInfo_.updateForNewMesh(*mesh);

    metaDataProps_.updateProperty(metaDataProperty_, mesh->getMetaDataMap());
}

}  // namespace inviwo
