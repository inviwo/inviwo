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
#include <modules/discretedata/util/util.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <modules/base/algorithm/dataminmax.h>

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
    , positionChannel_("positionChannel", "Position Channel", &portInDataSet_,
                       DataChannelProperty::FilterPassDim<0>)
    , colorChannel_("colorChannel", "Color Channel", &portInDataSet_,
                    DataChannelProperty::FilterPassDim<0>)
    , textureChannel_("textureChannel", "Texture Coordinate Channel", &portInDataSet_,
                      DataChannelProperty::FilterPassDim<0>)
    , normalizeTextureCoords_("normalizeTextureCoords", "Normalize Tex Coords")
    , primitive_("primitive", "Mesh Primitive")
    , cutAtBorder_("cutAtBorder", "Don't connect borders", false)
    , invalidColor_("invalidColor", "Invalid Color",
                    {{"transparent", "Transparent", InvalidColor::Transparent},
                     {"black", "Black", InvalidColor::Black},
                     {"white", "White", InvalidColor::White}}) {

    addPort(portInDataSet_);
    addPort(portOutMesh_);
    addProperties(positionChannel_, colorChannel_, textureChannel_, normalizeTextureCoords_,
                  primitive_, cutAtBorder_, invalidColor_);
}

// namespace {
// template <typename ToType>
// struct NormalizeChannelToBufferDispatcher {

//     template <typename T, ind N>
//     std::shared_ptr<BufferBase> operator()(const DataChannel<T, N>* positions) {
//         typedef typename DataChannel<T, N>::DefaultVec DefaultVec;
//         ind numElements = positions->size();
//         std::vector<DefaultVec> data(numElements);
//         positions->fill(*data.data(), 0, numElements);
//         DefaultVec min, max;
//         auto ext = positions->getMinMax(min, max);
//         T* rawPtr = reinterpret_cast<T*>(data.data());
//         for (ind e = 0; e < numElements * N; ++e) {
//             rawPtr[e] = (rawPtr[e] - min) / (max - min);
//         }

//         std::vector<ToType> convBuffer;
//         convBuffer.reserve(data.size());

//         for (const DefaultVec& val : data) convBuffer.push_back(util::glm_convert<ToType>(val));

//         auto buffer = util::makeBuffer(std::move(convBuffer));
//         return buffer;
//     }
// };
// }  // namespace

void MeshFromDataSet::process() {

    // Get data
    auto pInDataSet = portInDataSet_.getData();
    if (!pInDataSet || !pInDataSet->size() ||
        !pInDataSet->getGrid()->getNumElements(GridPrimitive::Vertex)) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    auto posChannel = positionChannel_.getCurrentChannel();
    auto colorChannel = colorChannel_.getCurrentChannel();
    auto textureChannel = textureChannel_.getCurrentChannel();
    if (!posChannel) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    if (portInDataSet_.isChanged()) {
        ind maxDim = std::min((ind)pInDataSet->getGrid()->getDimension(), ind(2));
        GridPrimitive former = GridPrimitive::Vertex;
        if (primitive_.getSelectedIndex() < primitive_.size()) former = primitive_.get();

        primitive_.clearOptions();
        for (ind dim = 0; dim <= maxDim; ++dim) {
            GridPrimitive gridDim = (GridPrimitive)dim;
            primitive_.addOption(primitiveName(gridDim), primitiveName(gridDim), gridDim);
        }
        primitive_.set(GridPrimitive((former == GridPrimitive::Vertex) ? maxDim : (ind)former));
    }

    DrawType primType = (DrawType)(std::max((int)primitive_.get() + 1, (int)DrawType::Triangles));
    inviwo::Mesh* result = new Mesh(primType, ConnectivityType::None);

    // Add positions.
    dd_util::ChannelToBufferDispatcher<vec3> posToBuffer;
    auto positions =
        posChannel->dispatch<std::shared_ptr<BufferBase>, dispatching::filter::Scalars, 1, 4>(
            posToBuffer);
    result->addBuffer(BufferType::PositionAttrib, positions);

    // Add colors.
    dvec4 invalidColor;
    switch (invalidColor_.get()) {
        case InvalidColor::Black:
            invalidColor = {0, 0, 0, 1};
            break;
        case InvalidColor::White:
            invalidColor = {1, 1, 1, 1};
            break;
        default:
            invalidColor = {0, 0, 0, 0};
            break;
    }
    if (colorChannel) {
        std::shared_ptr<BufferBase> colors;
        if (colorChannel->getNumComponents() == 4) {
            colors =
                colorChannel
                    ->dispatch<std::shared_ptr<BufferBase>, dispatching::filter::Scalars, 4, 4>(
                        dd_util::ChannelToBufferDispatcher<vec4>(invalidColor));
        } else {
            colors =
                colorChannel
                    ->dispatch<std::shared_ptr<BufferBase>, dispatching::filter::Scalars, 1, 3>(
                        dd_util::ChannelToBufferDispatcher<vec3>(invalidColor));
        }

        result->addBuffer(BufferType::ColorAttrib, colors);
    }

    // Add texture coordinates.
    if (textureChannel) {
        std::shared_ptr<BufferBase> texCoords;
        if (normalizeTextureCoords_.get() && textureChannel->getNumComponents() == 1) {
            auto minMax = dd_util::getMinMax(textureChannel.get());
            texCoords =
                textureChannel
                    ->dispatch<std::shared_ptr<BufferBase>, dispatching::filter::Scalars, 1, 1>(
                        dd_util::ChannelToBufferDispatcher<vec2>(),
                        [min = minMax.first.x, range = minMax.second.x - minMax.first.x](
                            const auto& vec) { return vec2((double(vec) - min) / range, 0); });
        } else if (normalizeTextureCoords_.get()) {
            auto minMax = dd_util::getMinMax(textureChannel.get());
            texCoords =
                textureChannel
                    ->dispatch<std::shared_ptr<BufferBase>, dispatching::filter::Scalars, 2, 4>(
                        dd_util::ChannelToBufferDispatcher<vec2>(),
                        [min = minMax.first,
                         range = minMax.second - minMax.first](const auto& vec) -> vec2 {
                            return vec2((double(vec[0]) - min[0]) / range[0],
                                        (double(vec[1]) - min[1]) / range[1]);
                        });
        } else {
            texCoords =
                textureChannel
                    ->dispatch<std::shared_ptr<BufferBase>, dispatching::filter::Scalars, 1, 3>(
                        dd_util::ChannelToBufferDispatcher<vec2>());
        }

        result->addBuffer(BufferType::TexcoordAttrib, texCoords);
    }

    if (primitive_.get() != GridPrimitive::Vertex) {
        std::vector<ind> indexData;
        std::vector<std::uint32_t> indexMeshData;

        switch (primitive_.get()) {
            case GridPrimitive::Face:
                for (auto element : pInDataSet->getGrid()->all(GridPrimitive::Face)) {
                    indexData.clear();
                    pInDataSet->getGrid()->getConnections(
                        indexData, element.getIndex(), GridPrimitive::Face, GridPrimitive::Vertex,
                        cutAtBorder_.get());

                    // Add triagnle "fans".
                    ind tri = 0;
                    for (; tri < static_cast<ind>(indexData.size()) - 3; tri += 2) {
                        indexMeshData.push_back(static_cast<std::uint32_t>(indexData[tri]));
                        indexMeshData.push_back(static_cast<std::uint32_t>(indexData[tri + 1]));
                        indexMeshData.push_back(static_cast<std::uint32_t>(indexData[tri + 2]));
                        indexMeshData.push_back(static_cast<std::uint32_t>(indexData[tri + 2]));
                        indexMeshData.push_back(static_cast<std::uint32_t>(indexData[tri + 1]));
                        indexMeshData.push_back(static_cast<std::uint32_t>(indexData[tri + 3]));
                    }
                    if (tri == static_cast<ind>(indexData.size() - 3)) {
                        indexMeshData.push_back(static_cast<std::uint32_t>(indexData[tri]));
                        indexMeshData.push_back(static_cast<std::uint32_t>(indexData[tri + 1]));
                        indexMeshData.push_back(static_cast<std::uint32_t>(indexData[tri + 2]));
                    }
                }
                break;
            case GridPrimitive::Edge:
                for (auto element : pInDataSet->getGrid()->all(GridPrimitive::Edge)) {
                    indexData.clear();
                    pInDataSet->getGrid()->getConnections(
                        indexData, element.getIndex(), GridPrimitive::Edge, GridPrimitive::Vertex,
                        cutAtBorder_.get());
                    ivwAssert(indexData.size() == 2 || indexData.size() == 0,
                              "An edge not made out of 2 points...");
                    if (indexData.size() == 2) {
                        indexMeshData.push_back(static_cast<std::uint32_t>(indexData[0]));
                        indexMeshData.push_back(static_cast<std::uint32_t>(indexData[1]));
                    }
                }
                break;
            default:
                break;
        }

        auto indexBuff = util::makeIndexBuffer(std::move(indexMeshData));
        result->addIndices(Mesh::MeshInfo(primType, ConnectivityType::None), indexBuff);
    }

    portOutMesh_.setData(result);
}

}  // namespace discretedata
}  // namespace inviwo
