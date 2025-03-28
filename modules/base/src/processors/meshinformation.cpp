/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/core/algorithm/markdown.h>                   // for operator""_help, operator""...
#include <inviwo/core/ports/meshport.h>                       // for MeshInport
#include <inviwo/core/processors/processor.h>                 // for Processor
#include <inviwo/core/processors/processorinfo.h>             // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>            // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>             // for Tags
#include <inviwo/core/properties/compositeproperty.h>         // for CompositeProperty
#include <inviwo/core/properties/valuewrapper.h>              // for PropertySerializationMode
#include <inviwo/core/util/metadatatoproperty.h>              // for MetaDataToProperty
#include <modules/base/properties/meshinformationproperty.h>  // for MeshInformationProperty

#include <memory>       // for shared_ptr, shared_ptr<>::e...
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshInformation::processorInfo_{
    "org.inviwo.MeshInformation",        // Class identifier
    "Mesh Information",                  // Display name
    "Information",                       // Category
    CodeState::Stable,                   // Code state
    "CPU, Mesh, Geometry, Information",  // Tags
    R"(
    Shows available information provided by the input mesh including metadata.
    )"_unindentHelp};
const ProcessorInfo& MeshInformation::getProcessorInfo() const { return processorInfo_; }

MeshInformation::MeshInformation()
    : Processor()
    , mesh_("mesh", "Input mesh"_help)
    , meshInfo_("dataInformation", "Data Information")
    , metaDataProperty_(
          "metaData", "Meta Data",
          "Composite property listing all the metadata stored in the input Image"_help) {

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
