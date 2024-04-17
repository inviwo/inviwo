/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <modules/base/processors/volumeboundingbox.h>

#include <inviwo/core/ports/meshport.h>              // for MeshOutport
#include <inviwo/core/ports/volumeport.h>            // for VolumeInport
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>   // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>    // for Tags, Tags::None
#include <inviwo/core/properties/ordinalproperty.h>  // for FloatVec4Property
#include <inviwo/core/util/glmvec.h>                 // for vec4
#include <modules/base/algorithm/meshutils.h>        // for boundingBoxAdjacency

#include <memory>       // for shared_ptr, shared_ptr<>::element_...
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeBoundingBox::processorInfo_{
    "org.inviwo.VolumeBoundingBox",  // Class identifier
    "Volume Bounding Box",           // Display name
    "Volume Operation",              // Category
    CodeState::Stable,               // Code state
    Tags::CPU,                       // Tags
    "Creates a mesh containing the bounding box of the volume, that is lines with adjacency "
    "information."_help};
const ProcessorInfo VolumeBoundingBox::getProcessorInfo() const { return processorInfo_; }

VolumeBoundingBox::VolumeBoundingBox()
    : Processor()
    , volume_("volume", "Input volume"_help)
    , mesh_("mesh", "The bounding box mesh"_help)
    , color_("color", "Color", util::ordinalColor(vec4{1.0f}).set("Line color of the mesh"_help)) {

    addPorts(volume_, mesh_);
    addProperty(color_);
}

void VolumeBoundingBox::process() {
    auto mesh = meshutil::boundingBoxAdjacency(volume_.getData()->getModelMatrix(), color_);
    mesh->setWorldMatrix(volume_.getData()->getWorldMatrix());
    mesh_.setData(mesh);
}

}  // namespace inviwo
