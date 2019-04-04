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

#include <modules/discretedata/processors/volumefromdataset.h>
#include <modules/discretedata/connectivity/structuredgrid.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeFromDataSet::processorInfo_{
    "org.inviwo.VolumeFromDataSet",  // Class identifier
    "Volume From Data Set",          // Display name
    "Data Set",                     // Category
    CodeState::Experimental,         // Code state
    Tags::None,                      // Tags
};

const ProcessorInfo VolumeFromDataSet::getProcessorInfo() const { return processorInfo_; }

VolumeFromDataSet::VolumeFromDataSet()
    : portInDataSet("InDataSet")
    , portOutVolume("OutVolume")
    , channelName(portInDataSet, "channelName", "Data Channel")
    //, useVoxelData("useVoxelData", "Use Voxel Data", false)
    , floatVolumeOutput("floatVolumeOutput", "Convert Data to Float 32") {
    addPort(portInDataSet);
    addPort(portOutVolume);
    addProperty(channelName);
    // addProperty(useVoxelData);
    addProperty(floatVolumeOutput);
}

void VolumeFromDataSet::process() {
    // Get data
    auto pInDataSet = portInDataSet.getData();
    if (!pInDataSet) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    auto pStructuredGrid = pInDataSet->getGrid<StructuredGrid>();
    if (!pStructuredGrid || pStructuredGrid->getDimension() != GridPrimitive::Volume) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    auto channel = channelName.getCurrentChannel();
    if (!channel) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    inviwo::Volume* result = nullptr;
    switch (channel->getDataFormatId()) {
        case DataFormatId::Float16:
            result = convert<f16>(*channel, *pStructuredGrid);
            break;
        case DataFormatId::Float32:
            result = convert<glm::f32>(*channel, *pStructuredGrid);
            break;
        case DataFormatId::Float64:
            result = convert<glm::f64>(*channel, *pStructuredGrid);
            break;
        case DataFormatId::Int8:
            result = convert<glm::i8>(*channel, *pStructuredGrid);
            break;
        case DataFormatId::Int16:
            result = convert<glm::i16>(*channel, *pStructuredGrid);
            break;
        case DataFormatId::Int32:
            result = convert<glm::i32>(*channel, *pStructuredGrid);
            break;
        case DataFormatId::Int64:
            result = convert<glm::i64>(*channel, *pStructuredGrid);
            break;
        case DataFormatId::UInt8:
            result = convert<glm::u8>(*channel, *pStructuredGrid);
            break;
        case DataFormatId::UInt16:
            result = convert<glm::u16>(*channel, *pStructuredGrid);
            break;
        case DataFormatId::UInt32:
            result = convert<glm::u32>(*channel, *pStructuredGrid);
            break;
        case DataFormatId::UInt64:
            result = convert<glm::u64>(*channel, *pStructuredGrid);
            break;
        default:
            LogWarn("Could not resolve DataFormat");
            break;
    }

    portOutVolume.setData(result);
}

// void VolumeFromDataSet::updateChannelList() {
//     auto dataset = portInDataSet.getData();
//     LogWarn("Hej!");
//     std::string lastName = "";
//     if (channelName.size()) lastName = channelName.get();
//     LogWarn(lastName);
//     channelName.clearOptions();

//     if (!dataset) {
//         return;
//     }

//     auto dataSetNames = dataset->getChannelNames();
//     GridPrimitive filter = useVoxelData.get() ? GridPrimitive::Volume : GridPrimitive::Vertex;
//     for (auto& name : dataSetNames) {
//         if (!(name.second == filter)) continue;
//         auto chann = dataset->getChannel(name.first, filter);
//         if (chann->getNumComponents() != 1) continue;
//         channelName.addOption(name.first, name.first);
//     }

//     channelName.setSelectedIdentifier(lastName);
// }

}  // namespace discretedata
}  // namespace inviwo
