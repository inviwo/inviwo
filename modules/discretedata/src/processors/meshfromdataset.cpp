/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/discretedata/processors/meshfromdataset.h>
#include <modules/discretedata/connectivity/structuredgrid.h>
#include <modules/discretedata/util.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshFromDataSet::processorInfo_{
    "org.inviwo.MeshFromDataSet",  // Class identifier
    "Mesh From Data Set",          // Display name
    "Data Set",                    // Category
    CodeState::Experimental,       // Code state
    Tags::None,                    // Tags
};

const ProcessorInfo MeshFromDataSet::getProcessorInfo() const { return processorInfo_; }

MeshFromDataSet::MeshFromDataSet()
    : portInDataSet_("InDataSet")
    , portOutMesh_("OutMesh")
    , positionChannel_(portInDataSet_, "positionChannel", "Position Channel",
                       DataChannelProperty::FilterPassDim<0>)
    , colorChannel_(portInDataSet_, "colorChannel", "Color Channel",
                    DataChannelProperty::FilterPassDim<0>)
    , primitive_("primitive", "Mesh Primitive") {

    addPort(portInDataSet_);
    addPort(portOutMesh_);
    addProperty(positionChannel_);
    addProperty(colorChannel_);
    addProperty(primitive_);
}

void MeshFromDataSet::process() {

    // Get data
    auto pInDataSet = portInDataSet_.getData();
    if (!pInDataSet) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    auto posChannel = positionChannel_.getCurrentChannel();
    auto colorChannel = colorChannel_.getCurrentChannel();
    if (!posChannel || !colorChannel) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    if (portInDataSet_.isChanged()) {
        ind maxDim = std::min((ind)pInDataSet->getGrid()->getDimension(), ind(2));
        primitive_.clearOptions();
        for (ind dim = 0; dim <= maxDim; ++dim) {
            GridPrimitive gridDim = (GridPrimitive)dim;
            primitive_.addOption(primitiveName(gridDim), primitiveName(gridDim), gridDim);
        }
        primitive_.set(GridPrimitive(maxDim));
    }

    DrawType primType = (DrawType)((int)primitive_.get() + 1);
    inviwo::Mesh* result = new Mesh(primType, ConnectivityType::None);

    // Add positions.
    dd_util::ChannelToBufferDispatcher<vec3> posToBuffer;
    auto positions =
        posChannel->dispatch<std::shared_ptr<BufferBase>, dispatching::filter::Scalars, 1, 4>(
            posToBuffer);
    result->addBuffer(BufferType::PositionAttrib, positions);

    // Add colors.
    std::shared_ptr<BufferBase> colors;
    if (colorChannel->getNumComponents() == 4)
        colors =
            colorChannel->dispatch<std::shared_ptr<BufferBase>, dispatching::filter::Scalars, 4, 4>(
                dd_util::ChannelToBufferDispatcher<vec4>());
    else
        colors =
            colorChannel->dispatch<std::shared_ptr<BufferBase>, dispatching::filter::Scalars, 1, 3>(
                dd_util::ChannelToBufferDispatcher<vec3>());

    result->addBuffer(BufferType::ColorAttrib, colors);

    std::vector<ind> indexData;
    for (auto element : pInDataSet->getGrid()->all(primitive_.get()))
        pInDataSet->getGrid()->getConnections(indexData, element.getIndex(), primitive_.get(),
                                              GridPrimitive::Vertex);
    std::vector<std::uint32_t> indexMeshData;
    indexMeshData.reserve(indexData.size());
    for (auto val : indexData) indexMeshData.push_back(static_cast<std::uint32_t>(val));

    auto indexBuff = util::makeIndexBuffer(std::move(indexMeshData));
    result->addIndicies(Mesh::MeshInfo(primType, ConnectivityType::None), indexBuff);

    portOutMesh_.setData(result);
}

}  // namespace discretedata
}  // namespace inviwo
