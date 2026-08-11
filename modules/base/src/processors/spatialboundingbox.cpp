/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/base/processors/spatialboundingbox.h>

#include <modules/base/algorithm/meshutils.h>

#include <ranges>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SpatialBoundingBox::processorInfo_{
    "org.inviwo.SpatialBoundingBox",  // Class identifier
    "Spatial Bounding Box",           // Display name
    "Undefined",                      // Category
    CodeState::Stable,                // Code state
    Tags::CPU,                        // Tags
    R"(<Explanation of how to use the processor.>)"_unindentHelp,
};

const ProcessorInfo& SpatialBoundingBox::getProcessorInfo() const { return processorInfo_; }

SpatialBoundingBox::SpatialBoundingBox()
    : Processor{}
    , inport_("inport", "Input Spatial Entity"_help)
    , mesh_("mesh", "The bounding box mesh"_help)
    , color_("color", "Color", util::ordinalColor(vec4{1.0f}).set("Line color of the mesh"_help)) {

    addPorts(inport_, mesh_);
    addProperty(color_);
}

void SpatialBoundingBox::process() {
    auto mesh = meshutil::boundingBoxAdjacency(inport_.getData()->getModelMatrix(), color_);
    mesh->setWorldMatrix(inport_.getData()->getWorldMatrix());

    for (auto i : std::views::iota(0uz, 3uz)) {
        if (auto* axis = inport_.getData()->getAxis(i)) {
            mesh->axes[i] = *axis;
        }
    }

    mesh_.setData(mesh);
}

}  // namespace inviwo
