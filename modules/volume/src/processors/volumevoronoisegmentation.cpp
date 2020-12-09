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
    , dataFrame_("seedPoints")
    , outport_("outport")
    , weighted_("weighted", "Weighted voronoi", false) {

    addPort(volume_);
    addPort(dataFrame_);
    addPort(outport_);

    addProperty(weighted_);
}

void VolumeVoronoiSegmentation::process() {
    auto dataFrame = dataFrame_.getData();

    auto indices =
        dataFrame->getIndexColumn()
            ->getBuffer()
            ->getRepresentation<BufferRAM>()
            ->dispatch<std::vector<uint32_t>, dispatching::filter::Scalars>([](auto buf) {
                auto& data = buf->getDataContainer();
                std::vector<uint32_t> dst(data.size(), 0.0f);
                std::transform(data.begin(), data.end(), dst.begin(),
                               [&](auto v) { return static_cast<uint32_t>(v); });
                return dst;
            });

    auto getColumnDataAsFloats = [dataFrame](const std::string columnName) {
        return dataFrame->getColumn(columnName)
            ->getBuffer()
            ->getRepresentation<BufferRAM>()
            ->dispatch<std::vector<float>, dispatching::filter::Scalars>([](auto buf) {
                auto& data = buf->getDataContainer();
                std::vector<float> dst(data.size(), 0.0f);
                std::transform(data.begin(), data.end(), dst.begin(),
                               [&](auto v) { return static_cast<float>(v); });
                return dst;
            });
    };

    auto xPos = getColumnDataAsFloats("x");
    auto yPos = getColumnDataAsFloats("y");
    auto zPos = getColumnDataAsFloats("z");
    auto radii = getColumnDataAsFloats("r");

    if (xPos.size() != yPos.size() || xPos.size() != zPos.size() || xPos.size() != radii.size()) {
        throw Exception("Unexpected dimension missmatch", IVW_CONTEXT);
    }

    std::vector<std::pair<uint32_t, vec3>> seedPointsWithIndices;
    for (std::size_t i = 0; i < xPos.size(); ++i) {
        seedPointsWithIndices.push_back({indices[i], vec3{xPos[i], yPos[i], zPos[i]}});
    }

    // TODO: make sure the voxel positions and seed positions are in the same space?
    const auto volume = volume_.getData();
    outport_.setData(util::voronoiSegmentation(
        volume->getDimensions(), volume->getModelMatrix(), volume->getWorldMatrix(),
        volume->getCoordinateTransformer().getIndexToModelMatrix(), volume->getWrapping(),
        seedPointsWithIndices, radii, weighted_.get()));
}
}  // namespace inviwo
