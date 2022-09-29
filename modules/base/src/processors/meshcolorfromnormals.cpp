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

#include <modules/base/processors/meshcolorfromnormals.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for BufferBase
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferType, Buffe...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh, Mesh::Buffe...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/ports/meshport.h>                                 // for MeshInport, MeshO...
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::None
#include <inviwo/core/properties/optionproperty.h>                      // for OptionPropertyOption
#include <inviwo/core/util/exception.h>                                 // for Exception
#include <inviwo/core/util/formatdispatching.h>                         // for PrecisionValueType
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT
#include <inviwo/core/util/staticstring.h>                              // for operator+

#include <algorithm>                                                    // for transform
#include <memory>                                                       // for shared_ptr, share...
#include <type_traits>                                                  // for remove_extent_t
#include <unordered_map>                                                // for unordered_map
#include <unordered_set>                                                // for unordered_set

#include <glm/common.hpp>                                               // for abs
#include <glm/vec2.hpp>                                                 // for operator+, operator/
#include <glm/vec3.hpp>                                                 // for operator+, operator/
#include <glm/vec4.hpp>                                                 // for operator+, operator/
#include <half/half.hpp>                                                // for operator+, operator/

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshColorFromNormals::processorInfo_{
    "org.inviwo.MeshColorFromNormals",  // Class identifier
    "Mesh Color From Normals",          // Display name
    "Mesh Processing",                  // Category
    CodeState::Experimental,            // Code state
    Tags::None,                         // Tags
};
const ProcessorInfo MeshColorFromNormals::getProcessorInfo() const { return processorInfo_; }

MeshColorFromNormals::MeshColorFromNormals()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , transform_("transform", "Transform to color",
                 {{"none", "None", Transform::None},
                  {"abs", "Use Absolute Value", Transform::Abs},
                  {"shift", "Shift to positive (i.e: (N+1)/2)", Transform::Shift}}) {
    addPort(inport_);
    addPort(outport_);

    addProperties(transform_);
}

void MeshColorFromNormals::process() {
    auto inMesh = inport_.getData();

    auto mesh = std::shared_ptr<Mesh>(inMesh->clone());
    while (auto cbuf = mesh->getBuffer(BufferType::ColorAttrib)) {
        mesh->removeBuffer(cbuf);
    }

    if (auto normalsBuffer = mesh->getBuffer(BufferType::NormalAttrib)) {
        auto newBuf = std::shared_ptr<BufferBase>(normalsBuffer->clone());
        auto transform = transform_.get();
        if (transform != Transform::None) {
            newBuf->getEditableRepresentation<BufferRAM>()->dispatch<void>([transform](auto ram) {
                using T = util::PrecisionValueType<decltype(ram)>;
                auto& vec = ram->getDataContainer();
                std::transform(vec.begin(), vec.end(), vec.begin(), [transform](T n) -> T {
                    switch (transform) {
                        case Transform::Abs:
                            return glm::abs(n);
                        case Transform::Shift:
                            return (n + T{1}) / T{2};
                        case Transform::None:
                            [[fallthrough]];
                        default:
                            return n;
                            break;
                    }
                });
            });
        }

        mesh->addBuffer(Mesh::BufferInfo{BufferType::ColorAttrib}, newBuf);
    } else {
        throw Exception("Input mesh has no normals", IVW_CONTEXT);
    }

    outport_.setData(mesh);
}

}  // namespace inviwo
