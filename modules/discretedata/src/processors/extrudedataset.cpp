/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/discretedata/processors/extrudedataset.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <modules/discretedata/properties/datasamplerproperty.h>
#include <modules/discretedata/connectivity/extrudedgrid.h>
#include <modules/discretedata/sampling/extrudeddatasetsampler.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ExtrudeDataSet::processorInfo_{
    "org.inviwo.ExtrudeDataSet",  // Class identifier
    "Extrude Data Set",           // Display name
    "DiscreteData",               // Category
    CodeState::Experimental,      // Code state
    Tags::None,                   // Tags
};
const ProcessorInfo ExtrudeDataSet::getProcessorInfo() const { return processorInfo_; }

ExtrudeDataSet::ExtrudeDataSet()
    : Processor()
    , baseDataSetIn_("baseData")
    , extrudeDataSetIn_("extraData")
    , dataOut_("dataOut")
    , dataSetName_("dataSetName", "DataSet Name", "ExtrudedData")
    , numExtrudeElements_("numExtrudeElements", "# Elements to extrude", 10,
                          std::pair<size_t, ConstraintBehavior>{2, ConstraintBehavior::Immutable},
                          std::pair<size_t, ConstraintBehavior>{100, ConstraintBehavior::Ignore})
    , extrudeDataMembers_("extrudeData", "Data to Extrude", [this]() {
        std::vector<std::unique_ptr<Property>> v;
        v.emplace_back(std::make_unique<ExtendObjectProperty<DataChannelProperty>>(
            "dataChannel", "Data Channel", &baseDataSetIn_, &extrudeDataSetIn_));
        v.emplace_back(std::make_unique<ExtendObjectProperty<DataSamplerProperty>>(
            "dataSampler", "Data Sampler", &baseDataSetIn_, &extrudeDataSetIn_));
        return v;
    }()) {

    addPorts(baseDataSetIn_, extrudeDataSetIn_, dataOut_);
    addProperties(dataSetName_, numExtrudeElements_, extrudeDataMembers_);

    extrudeDataSetIn_.setOptional(true);
}

void ExtrudeDataSet::process() {
    std::shared_ptr<const DataSet> extrudeDataset;
    if (extrudeDataSetIn_.hasData()) extrudeDataset = extrudeDataSetIn_.getData();

    bool hasExtrudeData =
        extrudeDataset && (extrudeDataset->getGrid()->getDimension() == GridPrimitive::Edge ||
                           extrudeDataset->getGrid()->getDimension() == GridPrimitive::Vertex);
    numExtrudeElements_.setVisible(!hasExtrudeData || !extrudeDataMembers_.size());
    extrudeDataMembers_.setVisible(hasExtrudeData);

    if (!baseDataSetIn_.hasData()) return;
    auto dataset = baseDataSetIn_.getData();

    // No data to extend - make new grid only.
    if (!hasExtrudeData) {
        auto extendedGrid =
            std::make_shared<ExtrudedGrid>(dataset->getGrid(), numExtrudeElements_.get());
        dataOut_.setData(std::move(std::make_shared<DataSet>(dataSetName_.get(), extendedGrid)));
        return;
    }

    // Make a new grid first.
    auto newGrid = std::make_shared<ExtrudedGrid>(dataset->getGrid(),
                                                  extrudeDataset->getGrid()->getNumElements());
    auto newDataset = std::make_shared<DataSet>(dataSetName_.get(), newGrid);

    for (auto* channelProp :
         extrudeDataMembers_.getPropertiesByType<ExtendObjectProperty<DataChannelProperty>>()) {
        auto baseChannel = channelProp->dataObject_.getCurrentChannel();
        if (!baseChannel || baseChannel->getGridPrimitiveType() != GridPrimitive::Vertex) continue;

        auto extraChannel = channelProp->extrudingChannel_.getCurrentChannel();
        if (!extraChannel || baseChannel->getDataFormatId() != extraChannel->getDataFormatId())
            continue;

        auto combinedChannel = createExtendedChannel(
            baseChannel, extraChannel,
            fmt::format("{}_x_{}", baseChannel->getName(), extraChannel->getName()));
        if (!combinedChannel) continue;

        newDataset->addChannel(combinedChannel);
    }

    for (auto* samplerProp :
         extrudeDataMembers_.getPropertiesByType<ExtendObjectProperty<DataSamplerProperty>>()) {
        auto baseSampler = samplerProp->dataObject_.getCurrentSampler();
        if (!baseSampler) continue;

        auto extraChannel = samplerProp->extrudingChannel_.getCurrentChannelTyped<double, 1>();
        // std::dynamic_pointer_cast<const DataChannel<double, 1>>(
        // samplerProp->extrudingChannel_.getCurrentChannel());
        if (!extraChannel) continue;

        auto extrudedSampler = createExtrudedDataSetSampler(baseSampler, extraChannel, newGrid);
        if (!extrudedSampler) continue;
        newDataset->addChannel(extrudedSampler->coordinates_);
        newDataset->addSampler(extrudedSampler);
    }

    dataOut_.setData(std::move(newDataset));
}

}  // namespace discretedata
}  // namespace inviwo
