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

#pragma once

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/meshport.h>
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>

namespace inviwo {

/** \docpage{org.inviwo.SeedsToMesh, Seeds To Mesh}
 * ![](org.inviwo.SeedsToMesh.png?classIdentifier=org.inviwo.SeedsToMesh)
 * Create a point mesh from a set of seed points.
 */
template <unsigned N>
class IVW_MODULE_VECTORFIELDVISUALIZATION_API SeedsToMesh : public Processor {
public:
    SeedsToMesh();
    virtual ~SeedsToMesh() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataInport<SeedPointVector<N>> seedPointsIn_;
    MeshOutport meshOut_;
};

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
template <unsigned N>
const ProcessorInfo SeedsToMesh<N>::processorInfo_{
    fmt::format("org.inviwo.SeedsToMesh{}D", N),  // Class identifier
    fmt::format("Seeds To Mesh {}D", N),          // Display name
    "Seed Points",                                // Category
    CodeState::Stable,                            // Code state
    Tags::None,                                   // Tags
};
template <unsigned N>
const ProcessorInfo SeedsToMesh<N>::getProcessorInfo() const {
    return processorInfo_;
}
template <unsigned N>
SeedsToMesh<N>::SeedsToMesh() : Processor(), seedPointsIn_("seedsIn"), meshOut_("pointMeshOut") {
    addPort(seedPointsIn_);
    addPort(meshOut_);
}

template <unsigned N>
void SeedsToMesh<N>::process() {
    if (!seedPointsIn_.hasData()) {
        meshOut_.detachData();
        return;
    }

    auto mesh = std::make_shared<Mesh>(DrawType::Points, ConnectivityType::None);
    std::vector<vec3> positions;
    positions.reserve(seedPointsIn_.getData()->size());

    for (const auto& seeds : seedPointsIn_) {
        for (const auto& seed : *seeds) {
            vec3 seed3D{0, 0, 0};
            for (unsigned n = 0; n < std::min(N, 3u); ++n) seed3D[n] = seed[n];
            positions.push_back(seed3D);
        }
    }
    auto posBuffer = util::makeBuffer(std::move(positions));
    mesh->addBuffer(BufferType::PositionAttrib, posBuffer);

    meshOut_.setData(mesh);
}

using SeedsToMesh2D = SeedsToMesh<2>;
using SeedsToMesh3D = SeedsToMesh<3>;
using SeedsToMesh4D = SeedsToMesh<4>;

}  // namespace inviwo
