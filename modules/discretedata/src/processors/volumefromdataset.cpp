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
    "Volume From DataSet",           // Display name
    "DataSet",                       // Category
    CodeState::Experimental,         // Code state
    Tags::None,                      // Tags
};

const ProcessorInfo VolumeFromDataSet::getProcessorInfo() const { return processorInfo_; }

VolumeFromDataSet::VolumeFromDataSet()
    : portInDataSet_("InDataSet")
    , portOutVolume_("OutVolume")
    , conversionMethod_("conversionMethod", "Conversion method",
                        std::vector<OptionPropertyOption<ConversionMethod>>{
                            {"direct", "UniformGrid", ConversionMethod::UniformGrid},
                            {"sampling", "Sampling", ConversionMethod::Sample}})
    , channelToConvert_("channelName", "Data Channel", &portInDataSet_)
    , sampler_("sampler", "Sampler", &portInDataSet_)
    , volumeSize_("volumeSize", "Volume size", {128, 128, 128},
                  {{32, 32, 32}, ConstraintBehavior::Immutable},
                  {{512, 512, 512}, ConstraintBehavior::Immutable})
    , invalidValue_("invalidValue", "Invalid value", 0, {-1, ConstraintBehavior::Ignore},
                    {1, ConstraintBehavior::Ignore}, 0.1, InvalidationLevel::InvalidOutput,
                    PropertySemantics::Text)
    , floatVolumeOutput_("floatVolumeOutput", "Convert Data to Float 32") {
    addPorts(portInDataSet_, portOutVolume_);
    addProperties(conversionMethod_, channelToConvert_, sampler_, volumeSize_, invalidValue_,
                  floatVolumeOutput_);

    sampler_.visibilityDependsOn(conversionMethod_,
                                 [](auto& prop) { return prop.get() == ConversionMethod::Sample; });
    volumeSize_.visibilityDependsOn(
        conversionMethod_, [](auto& prop) { return prop.get() != ConversionMethod::UniformGrid; });

    floatVolumeOutput_.visibilityDependsOn(
        conversionMethod_, [](auto& prop) { return prop.get() == ConversionMethod::UniformGrid; });
}

void VolumeFromDataSet::process() {

    // Get data
    auto pInDataSet = portInDataSet_.getData();
    if (!pInDataSet) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    auto pGrid = pInDataSet->getGrid();
    if (conversionMethod_.get() == ConversionMethod::UniformGrid && !pGrid) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    auto channel = channelToConvert_.getCurrentChannel();
    if (!channel) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    auto sampler = sampler_.getCurrentSampler();
    if (conversionMethod_.get() == ConversionMethod::Sample && !sampler) {
        invalidate(InvalidationLevel::InvalidOutput);
        return;
    }

    // inviwo::Volume* result = nullptr;
    detail::VolumeDispatcher dispatcher;
    inviwo::Volume* result = Channel::dispatchSharedPointer<Volume*>(
        channel, dispatcher,  //*this);
        sampler, pGrid.get(), volumeSize_.get(), invalidValue_.get(), conversionMethod_.get(),
        floatVolumeOutput_.get());

    portOutVolume_.setData(result);
}

// void VolumeFromDataSet::updateChannelList() {
//     auto dataset = portInDataSet_.getData();
//     LogWarn("Hej!");
//     std::string lastName = "";
//     if (channelToConvert_.size()) lastName = channelToConvert_.get();
//     LogWarn(lastName);
//     channelToConvert_.clearOptions();

//     if (!dataset) {
//         return;
//     }

//     auto dataSetNames = dataset->getChannelToConvert_s();
//     GridPrimitive filter = useVoxelData.get() ? GridPrimitive::Volume : GridPrimitive::Vertex;
//     for (auto& name : dataSetNames) {
//         if (!(name.second == filter)) continue;
//         auto chann = dataset->getChannel(name.first, filter);
//         if (chann->getNumComponents() != 1) continue;
//         channelToConvert_.addOption(name.first, name.first);
//     }

//     channelToConvert_.setSelectedIdentifier(lastName);
// }

}  // namespace discretedata
}  // namespace inviwo
