/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <modules/base/processors/meshplaneclipping.h>

#include <inviwo/core/datastructures/geometry/plane.h>  // for Plane
#include <inviwo/core/ports/datainport.h>               // for FlatMultiDataInport
#include <inviwo/core/ports/inportiterable.h>           // for InportIterable<>::const_iterator
#include <inviwo/core/ports/meshport.h>                 // for MeshInport, MeshOutport
#include <inviwo/core/ports/outportiterable.h>          // for OutportIterable
#include <inviwo/core/processors/processor.h>           // for Processor
#include <inviwo/core/processors/processorinfo.h>       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>      // for CodeState, CodeState::Experimental
#include <inviwo/core/processors/processortags.h>       // for Tags, Tags::None
#include <inviwo/core/properties/boolproperty.h>        // for BoolProperty
#include <inviwo/core/util/glmvec.h>                    // for uvec3
#include <modules/base/algorithm/mesh/meshclipping.h>   // for clipMeshAgainstPlane

#include <memory>       // for shared_ptr
#include <string_view>  // for string_view

#include <fmt/core.h>  // for format_to, basic_string_view, format

namespace inviwo {
class Mesh;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshPlaneClipping::processorInfo_{
    "org.inviwo.MeshPlaneClipping",  // Class identifier
    "Mesh Plane Clipping",           // Display name
    "Mesh Creation",                 // Category
    CodeState::Experimental,         // Code state
    Tags::None,                      // Tags
};
const ProcessorInfo& MeshPlaneClipping::getProcessorInfo() const { return processorInfo_; }

MeshPlaneClipping::MeshPlaneClipping()
    : Processor()
    , inputMesh_("inputMesh")
    , planes_("inputPlanes")
    , outputMesh_("outputMesh")
    , clippingEnabled_("clippingEnabled", "Enable Clipping", true)
    , capClippedHoles_("capClippedHoles", "Cap clipped holes", true) {

    addPort(inputMesh_);
    addPort(planes_);
    addPort(outputMesh_);
    addProperties(clippingEnabled_, capClippedHoles_);
}

void MeshPlaneClipping::process() {
    if (clippingEnabled_) {
        std::shared_ptr<const Mesh> currentMesh = inputMesh_.getData();
        for (const auto& plane : planes_) {
            currentMesh = meshutil::clipMeshAgainstPlane(*currentMesh, *plane, capClippedHoles_);
        }
        outputMesh_.setData(currentMesh);
    } else {
        outputMesh_.setData(inputMesh_.getData());
    }
}

}  // namespace inviwo
