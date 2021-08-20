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

#include <modules/discretedata/processors/datasetroi.h>
#include <modules/discretedata/connectivity/pointcloud.h>
#include <modules/discretedata/channels/roichannel.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataSetROI::processorInfo_{
    "org.inviwo.DataSetROI",  // Class identifier
    "Data Set ROI",           // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo DataSetROI::getProcessorInfo() const { return processorInfo_; }

DataSetROI::DataSetROI()
    : Processor()
    , inData_("datasetIn")
    , outData_("datasetOut")
    , dataRange_("dataRange", "Range", {0, 10}, {0, 1}, {99, 100})
    , dataSize_("dataSize", "Num Elements", 10, 1, 100)
    , dataSetName_("dataSetName", "DataSet Name", "ROI") {

    addPort(inData_);
    addPort(outData_);
    addProperties(dataRange_, dataSize_);

    dataRange_.onChange([this]() {
        // Prevent endless loop.
        if (updatingProperties_) {
            updatingProperties_ = false;
            return;
        }
        updatingProperties_ = true;

        size_t offset = dataRange_.get()[0];
        size_t size = dataRange_.get()[1] - offset;
        dataRange_.setMaxValue({maxNumElements_ - size, maxNumElements_});
        dataSize_.setMaxValue(maxNumElements_ - offset);
        dataSize_.set(size);
    });

    dataSize_.onChange([this]() {
        // Prevent endless loop.
        if (updatingProperties_) {
            updatingProperties_ = false;
            return;
        }
        updatingProperties_ = true;

        size_t offset = dataRange_.get()[0];
        size_t size = dataSize_.get();
        dataRange_.set({offset, offset + size});
    });
}

void DataSetROI::process() {
    LogWarn("Processing ROI");
    if (!inData_.hasData()) {
        outData_.clear();
        return;
    }

    LogWarn("= Really processing!");

    maxNumElements_ = inData_.getData()->getGrid()->getNumElements(GridPrimitive::Vertex);

    size_t min = dataRange_.get()[0];
    size_t max = dataRange_.get()[1];
    if (min + 1 >= maxNumElements_ || max >= maxNumElements_) {
        LogWarn("= Do we update the prop?");
        dataRange_.set({std::max(min, maxNumElements_ - 1), maxNumElements_ - 1});
    } else {
        dataRange_.propertyModified();
    }
    LogWarn("= Updated props");

    auto pointCloud = std::make_shared<PointCloud>(max - min);
    auto roiData = std::make_shared<DataSet>(dataSetName_.get(), pointCloud);

    for (auto channel : inData_.getData()->getChannels()) {
        if (channel.second->getGridPrimitiveType() != GridPrimitive::Vertex) continue;

        // channel.second->dispatch(
        //     [](auto dataChannel) { auto channel = new ROIChannel<template>(); })
        auto roi = createROIChannel(channel.second, min, max - min);
        if (!roi) {
            LogWarn("That didn't work! " << channel.second->getName());
            continue;
        }
        roiData->addChannel(roi);
    }

    outData_.setData(roiData);
}

void DataSetROI::deserialize(Deserializer& d) {
    d.deserialize("maxNumElements", maxNumElements_);
    Processor::deserialize(d);
}

void DataSetROI::serialize(Serializer& s) const {
    s.serialize("maxNumElements", maxNumElements_);
    Processor::serialize(s);
}

}  // namespace discretedata
}  // namespace inviwo
