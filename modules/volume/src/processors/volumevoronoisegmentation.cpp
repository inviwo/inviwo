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

#include <inviwo/volume/processors/volumevoronoisegmentation.h>
#include <modules/base/algorithm/volume/volumevoronoi.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeVoronoiSegmentation::processorInfo_{
    "org.inviwo.VolumeVoronoiSegmentation",  // Class identifier
    "Volume Voronoi Segmentation",           // Display name
    "Volume Operation",                      // Category
    CodeState::Experimental,                 // Code state
    Tags::CPU,                               // Tags
};
const ProcessorInfo VolumeVoronoiSegmentation::getProcessorInfo() const { return processorInfo_; }

VolumeVoronoiSegmentation::VolumeVoronoiSegmentation()
    : Processor()
    , volume_("inputVolume")
    , inputMesh_("inputPositions")
    , outport_("outport")
    , weighted_("weighted", "Weighted voronoi", true) {

    addPort(volume_);
    addPort(inputMesh_);
    addPort(outport_);

    addProperty(weighted_);
}

void VolumeVoronoiSegmentation::process() {
    auto inputMesh = inputMesh_.getData();

    // Get seed points
    // TODO: see if this can be done in a better way
    auto posIt = util::find_if(inputMesh->getBuffers(), [](const auto& buf) {
        return buf.first.type == BufferType::PositionAttrib;
    });
    if (posIt == inputMesh->getBuffers().end()) {
        return;
    }
    const auto positionRam = posIt->second->getRepresentation<BufferRAM>();
    std::vector<dvec3> positions;
    for (std::size_t i = 0; i < positionRam->getSize(); ++i) {
        positions.push_back(positionRam->getAsDVec3(i));
    }

    // Get indices for seed points (if exists??)
    // TODO: should probably have some fallback solution if there are no indices...?
    auto indexIt = util::find_if(inputMesh->getBuffers(), [](const auto& buf) {
        return buf.first.type == BufferType::IndexAttrib;
    });
    if (indexIt == inputMesh->getBuffers().end()) {
        return;
    }
    // TODO: see if this can be done in a better way
    const auto& indices = static_cast<const BufferRAMPrecision<uint32_t, BufferTarget::Data>*>(
                              indexIt->second->getRepresentation<BufferRAM>())
                              ->getDataContainer();

    if (positions.size() != indices.size()) {
        throw Exception("Unexpected dimension missmatch", IVW_CONTEXT);
    }

    // Assuming the positions and indices come in the same order...
    std::vector<std::pair<dvec3, uint32_t>> seedPointsWithIndices;
    for (std::size_t i = 0; i < positions.size(); ++i) {
        seedPointsWithIndices.push_back({positions[i], indices[i]});
    }

    // In the case of the cube data volume,
    // the voxel positions needs to be divided with the dimensions of the volume
    const auto volume = volume_.getData();
    const auto basis = volume->getBasis();
    const auto dimensions = volume->getDimensions();
    const auto scaledBasis =
        mat3{basis[0] / dimensions.x, basis[1] / dimensions.y, basis[2] / dimensions.z};

    outport_.setData(
        util::voronoiSegmentation(volume, scaledBasis, seedPointsWithIndices, weighted_));
}

}  // namespace inviwo
