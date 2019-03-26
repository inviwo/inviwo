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
#include <modules/discretedata/processors/segmentationvoxelizer.h>

namespace inviwo {
namespace discretedata {

/////// TODO DELETEEEE //////////
struct TestDispatcher {
    template <typename Return, typename Scalar, ind N>
    Return operator()() {
        assert(N == 2);
        return N;
    }
} tester;

// struct TestChannelDispatcher {
//     template <typename Return, typename T, ind N>
//     Return operator()(DataChannel<T, N>* channel) {
//         LogWarn("Channel is called " + channel->getName());
//         LogWarn("Format is " + std::string(DataFormatBase::get(T)->getString()));
//         LogWarn("Dimension is " + std::to_string(N));
//     }
// } channelTester;
// dispatch([&](const auto lrprecision) {
//         layerRAMDistanceTransform(lrprecision, outDistanceField, inLayer->getBasis(), upsample,
//                                   predicate, valueTransform, callback);
//     });
//////////////////////////////////

const ProcessorInfo SegmentationVoxelizer::processorInfo_{
    "org.inviwo.discretedata.SegmentationVoxelizer",  // Class identifier
    "Create Voxel Mesh from Segmentation",            // Display name
    "Data Set",                                       // Category
    CodeState::Experimental,                          // Code state
    Tags::None                                        // Tags
};

SegmentationVoxelizer::SegmentationVoxelizer()
    : Processor()
    , portInData_("DataSet")
    , portOutMesh_("MeshOutput")
    , segmentationChannel_(portInData_, "segmentationChannel", "Segmentation Data")
    , vertexChannel_(portInData_, "vertexChannel", "Vertex positions") {
    addPort(portInData_);
    addPort(portOutMesh_);
    addProperty(segmentationChannel_);
    addProperty(vertexChannel_);
}

void SegmentationVoxelizer::process() {

    // Get data
    auto pInDataSet = portInData_.getData();
    if (!pInDataSet) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    auto pGrid = pInDataSet->getGrid();
    if (!pGrid || pGrid->getDimension() != GridPrimitive::Volume) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    auto segChannel = segmentationChannel_.getCurrentChannel();
    if (!segChannel) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    auto vertChannel = vertexChannel_.getCurrentChannel();
    if (!vertChannel) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    // Testing basic dispatching.
    ind dim = 2;
    int N = channeldispatching::dispatch<int, dispatching::filter::Scalars, 1, 3>(
        DataFormatId::Float32, dim, tester);
    LogWarn(N);

    // Testing dipatching on channel directly.
    Mesh* voxels = new Mesh();
    segChannel->dispatch<Mesh, dispatching::filter::Scalars, 1, 1>([&](const auto* segDataChannel) {
        return createVoxels(*voxels, *pGrid, *segDataChannel, *vertChannel);
    });

    // switch (segChannel->getDataFormatId()) {
    //     case DataFormatId::Float16:
    //         result = convert<f16>(*channel, *pStructuredGrid);
    //         break;
    //     case DataFormatId::Float32:
    //         result = convert<glm::f32>(*channel, *pStructuredGrid);
    //         break;
    //     case DataFormatId::Float64:
    //         result = convert<glm::f64>(*channel, *pStructuredGrid);
    //         break;
    //     case DataFormatId::Int8:
    //         result = convert<glm::i8>(*channel, *pStructuredGrid);
    //         break;
    //     case DataFormatId::Int16:
    //         result = convert<glm::i16>(*channel, *pStructuredGrid);
    //         break;
    //     case DataFormatId::Int32:
    //         result = convert<glm::i32>(*channel, *pStructuredGrid);
    //         break;
    //     case DataFormatId::Int64:
    //         result = convert<glm::i64>(*channel, *pStructuredGrid);
    //         break;
    //     case DataFormatId::UInt8:
    //         result = convert<glm::u8>(*channel, *pStructuredGrid);
    //         break;
    //     case DataFormatId::UInt16:
    //         result = convert<glm::u16>(*channel, *pStructuredGrid);
    //         break;
    //     case DataFormatId::UInt32:
    //         result = convert<glm::u32>(*channel, *pStructuredGrid);
    //         break;
    //     case DataFormatId::UInt64:
    //         result = convert<glm::u64>(*channel, *pStructuredGrid);
    //         break;
    //     default:
    //         break;
    // }
    LogWarn("made volume");

    portOutMesh_.setData(voxels);
}

const ProcessorInfo SegmentationVoxelizer::getProcessorInfo() const { return processorInfo_; }

}  // namespace discretedata
}  // namespace inviwo