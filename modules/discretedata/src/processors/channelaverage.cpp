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

#include <modules/discretedata/processors/channelaverage.h>

namespace inviwo {
namespace discretedata {

const std::string ChannelAverage::RANGE_ID = "range{}";
const std::string ChannelAverage::GRID_SIZE_ID = "gridSize{}";

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ChannelAverage::processorInfo_{
    "org.inviwo.ChannelAverage",  // Class identifier
    "Channel Average",            // Display name
    "Undefined",                  // Category
    CodeState::Experimental,      // Code state
    Tags::None,                   // Tags
};
const ProcessorInfo ChannelAverage::getProcessorInfo() const { return processorInfo_; }

ChannelAverage::ChannelAverage()
    : Processor()
    , dataIn_("dataIn")
    , dataOut_("dataWithAverage")
    , avgOut_("uniformAverage")

    , positions_("positions", "Positions", &dataIn_,
                 [&](auto channel) {
                     return dataIn_.hasData() &&
                            channel->getNumComponents() ==
                                (int)dataIn_.getData()->getGrid()->getDimension() &&
                            channel->getDataFormatId() == DataFormatId::Float32;
                 })
    , data_("data", "Data", &dataIn_)

    , averagingDim_("averagingDim", "Average Dimension", 0, {0, ConstraintBehavior::Immutable},
                    {0, ConstraintBehavior::Mutable})

    , dimRanges_("dimRanges", "Ranges")                  // One FloatMinMaxProperty per dimension.
    , avgGridSize_("avgGridSize", "Average Grid Sizes")  // One IntSizeTProperty per dimension.
    , relativeDiff_("relativeDiff", "Relative Difference", false) {

    addPorts(dataIn_, dataOut_, avgOut_);
    addProperties(positions_, data_, averagingDim_, dimRanges_, avgGridSize_, relativeDiff_);
}

void ChannelAverage::process() {
    if (!dataIn_.hasData()) {
        dataOut_.clear();
        avgOut_.clear();
        return;
    }
    auto data = dataIn_.getData();
    size_t numDims = (size_t)data->getGrid()->getDimension();
    averagingDim_.setMaxValue(numDims - 1);

    // For each dimension of the input data set, we should have
    // * a property to adjust the sample range and
    // * a property to set the size of the uniform average output.
    ivwAssert(dimRanges_.size() == avgGridSize_.size(),
              "Expected both composite properties to hold the same number of dims.");
    bool numDimsChanged = (dimRanges_.size() != numDims);
    size_t numDimsUnchanged = std::min(numDims, dimRanges_.size());
    if (numDimsChanged) {

        while (dimRanges_.size() > numDims) {
            dimRanges_.removeProperty(numDims);
            avgGridSize_.removeProperty(numDims);
        }

        while (dimRanges_.size() < numDims) {
            dimRanges_.addProperty(
                new FloatMinMaxProperty(fmt::format(RANGE_ID, dimRanges_.size()),
                                        fmt::format("Range {}", dimRanges_.size()), 0, 1));
            avgGridSize_.addProperty(new IntSizeTProperty(
                fmt::format(GRID_SIZE_ID, avgGridSize_.size()),
                fmt::format("Grid Size {}", avgGridSize_.size()), 32,
                {1, ConstraintBehavior::Immutable}, {256, ConstraintBehavior::Ignore}));
        }
    }
    if (!positions_.getCurrentChannel() || !data_.getCurrentChannel()) {
        dataOut_.clear();
        avgOut_.clear();
        return;
    }

    auto datasetOut = std::make_shared<DataSet>(*data);

    auto avgUniformData = channeldispatching::dispatchNumber<std::shared_ptr<DataSet>, 2,
                                                             DISCRETEDATA_MAX_NUM_DIMENSIONS>(
        positions_.getCurrentChannel()->getNumComponents(), AverageSpatialDispatcher(),
        datasetOut.get(), this, numDimsUnchanged);

    if (avgUniformData) {
        avgOut_.setData(avgUniformData);
        dataOut_.setData(datasetOut);
    } else {
        avgOut_.detachData();
        dataOut_.detachData();
    }
}

}  // namespace discretedata
}  // namespace inviwo
