/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/seedsfrommesh.h>
#include <modules/base/algorithm/meshutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SeedsFromMesh::processorInfo_{
    "org.inviwo.SeedsFromMesh",  // Class identifier
    "Seeds From Mesh",           // Display name
    "Seed Points",               // Category
    CodeState::Stable,           // Code state
    Tags::CPU,                   // Tags
};
const ProcessorInfo SeedsFromMesh::getProcessorInfo() const { return processorInfo_; }

SeedsFromMesh::SeedsFromMesh() : Processor(), mesh_("mesh"), seedPoints_("seedPoints") {

    addPort(mesh_);
    addPort(seedPoints_);
}

void SeedsFromMesh::process() {
    if (!mesh_.hasData()) return;
    auto points = std::make_shared<std::vector<vec3>>();
    const auto& mesh = *mesh_.getData();
    auto positions = mesh.getBuffer(0);

    LogWarn("Buffer named " << positions->dataName);
    if (!positions) {
        throw Exception("Input mesh has no position buffer",
                        IVW_CONTEXT_CUSTOM("meshutil::calculateMeshNormals"));
    }

    auto vertices = positions->getRepresentation<BufferRAM>();
    vertices->dispatch<void, dispatching::filter::Floats>([&](auto ram) {
        const auto& vert = ram->getDataContainer();
        for (const auto& p : vert) {
            points->push_back(util::glm_convert<vec3>(p));
            points->back().z /= 45;
        }
    });

    seedPoints_.setData(points);
}

}  // namespace inviwo
