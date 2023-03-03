/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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

#include <modules/base/processors/volumeboundaryplanes.h>

#include <inviwo/core/datastructures/coordinatetransformer.h>  // for StructuredCoordinateTransf...
#include <inviwo/core/datastructures/geometry/plane.h>         // for Plane
#include <inviwo/core/ports/dataoutport.h>                     // for DataOutport
#include <inviwo/core/ports/outportiterable.h>                 // for OutportIterableImpl<>::con...
#include <inviwo/core/ports/volumeport.h>                      // for VolumeInport
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>             // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>              // for Tags, Tags::CPU
#include <inviwo/core/properties/boolproperty.h>               // for BoolProperty
#include <inviwo/core/util/glmmat.h>                           // for mat3
#include <inviwo/core/util/glmvec.h>                           // for vec4, vec3

#include <functional>   // for __base
#include <memory>       // for shared_ptr, make_shared
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

#include <fmt/core.h>      // for format
#include <glm/mat3x3.hpp>  // for operator*, mat, mat<>::col...
#include <glm/mat4x4.hpp>  // for operator*, mat
#include <glm/matrix.hpp>  // for inverse, transpose
#include <glm/vec3.hpp>    // for operator*
#include <glm/vec4.hpp>    // for operator*, operator+, oper...

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeBoundaryPlanes::processorInfo_{
    "org.inviwo.VolumeBoundaryPlanes",  // Class identifier
    "Volume Boundary Planes",           // Display name
    "Data Creation",                    // Category
    CodeState::Stable,                  // Code state
    Tags::CPU,                          // Tags
};
const ProcessorInfo VolumeBoundaryPlanes::getProcessorInfo() const { return processorInfo_; }

VolumeBoundaryPlanes::VolumeBoundaryPlanes()
    : Processor()
    , volume_("volumeInport")
    , planes_("planeOutport")
    , flipPlanes_("flipPlanes", "Flip planes", false) {

    addPort(volume_);
    addPort(planes_);
    addProperties(flipPlanes_);
}

void VolumeBoundaryPlanes::process() {
    const auto vol = volume_.getData();
    const auto dataToWorld = vol->getCoordinateTransformer().getDataToWorldMatrix();
    const auto basis = vol->getBasis();
    const auto p0 = dataToWorld * vec4(vec3(0.0f), 1.0f);
    const auto p1 = dataToWorld * vec4(1.0f);
    const mat3 worldNormal = glm::transpose(glm::inverse(vol->getWorldMatrix()));
    const float sign = flipPlanes_ ? -1.0f : 1.0f;

    auto planes = std::make_shared<std::vector<Plane>>();

    for (unsigned int i = 0; i < 3; i++) {
        planes->emplace_back(p0, worldNormal * (-sign * basis[i]));
    }

    for (unsigned int i = 0; i < 3; i++) {
        planes->emplace_back(p1, worldNormal * (sign * basis[i]));
    }

    planes_.setData(planes);
}

}  // namespace inviwo
