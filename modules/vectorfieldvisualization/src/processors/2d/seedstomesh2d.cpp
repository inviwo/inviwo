/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/2d/seedstomesh2d.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SeedsToMesh2D::processorInfo_{
    "org.inviwo.SeedsToMesh2D",  // Class identifier
    "Seeds To Mesh 2D",          // Display name
    "Seed Points",               // Category
    CodeState::Stable,           // Code state
    Tags::None,                  // Tags
};
const ProcessorInfo SeedsToMesh2D::getProcessorInfo() const { return processorInfo_; }

SeedsToMesh2D::SeedsToMesh2D() : Processor(), seedPointsIn_("seedsIn"), meshOut_("pointMeshOut") {

    addPort(seedPointsIn_);
    addPort(meshOut_);
}

void SeedsToMesh2D::process() {
    if (!seedPointsIn_.hasData()) {
        meshOut_.detachData();
        return;
    }

    auto mesh = std::make_shared<Mesh>(DrawType::Points, ConnectivityType::None);
    std::vector<vec3> positions;
    positions.reserve(seedPointsIn_.getData()->size());

    for (const auto& seeds : seedPointsIn_) {
        for (const auto& seed : *seeds) {
            positions.push_back(vec3{seed[0], seed[1], 0});
        }
    }
    auto posBuffer = util::makeBuffer(std::move(positions));
    mesh->addBuffer(BufferType::PositionAttrib, posBuffer);

    meshOut_.setData(mesh);
}

}  // namespace inviwo
