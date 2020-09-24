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

#include <modules/discretedata/processors/createuniformgrid.h>
#include <modules/discretedata/channels/channeldispatching.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CreateUniformGrid::processorInfo_{
    "org.inviwo.CreateUniformGrid",  // Class identifier
    "Create Uniform Grid",           // Display name
    "Data Set",                      // Category
    CodeState::Experimental,         // Code state
    Tags::None,                      // Tags
};
const ProcessorInfo CreateUniformGrid::getProcessorInfo() const { return processorInfo_; }

CreateUniformGrid::CreateUniformGrid()
    : Processor()
    , dataSetOutport_("DataSetOutput")
    , dimensions_("dimensions", "Grid dimensions",
                  std::make_unique<DimensionProperty>("dimension", "Dimension"),
                  DISCRETEDATA_MAX_NUM_DIMENSIONS)
    , name_("positionName", "Position Channel Name", "Position") {

    addPort(dataSetOutport_);
    addProperties(dimensions_, name_);
    dimensions_.constructProperty(0);
}

void CreateUniformGrid::process() {
    detail::CreateUniformGridDispatcher dispatcher;

    size_t numDims = dimensions_.size();
    if (numDims < 1) {
        dimensions_.constructProperty(0);
    }

    std::vector<ind> numCells;
    std::vector<double> sizeCells;
    std::vector<double> offsetCells;
    numCells.reserve(numDims);
    sizeCells.reserve(numDims);
    offsetCells.reserve(numDims);
    for (size_t dim = 0; dim < numDims; ++dim) {
        auto dimProp = dynamic_cast<DimensionProperty*>(dimensions_[dim]);
        numCells.push_back(dimProp->numCells_.get());
        sizeCells.push_back(dimProp->cellSize_.get());
        offsetCells.push_back(dimProp->gridOffset_.get());
    }
    auto dataSet = channeldispatching::dispatchNumber<std::shared_ptr<DataSet>, 1, 4>(
        //   DISCRETEDATA_MAX_NUM_DIMENSIONS>(
        numDims, dispatcher, numCells, sizeCells, offsetCells, name_.get());

    dataSetOutport_.setData(dataSet);
}

DimensionProperty::DimensionProperty(std::string identifier, std::string displayName)
    : CompositeProperty(identifier, displayName)
    , numCells_("numCells", "Num Cells", 128, {1, ConstraintBehavior::Immutable},
                {1024, ConstraintBehavior::Ignore}, 1, InvalidationLevel::InvalidOutput,
                PropertySemantics::Text)
    , cellSize_("cellSize", "Cell Size", 1.0, {0.0, ConstraintBehavior::Immutable},
                {10.0, ConstraintBehavior::Ignore}, 0.1, InvalidationLevel::InvalidOutput,
                PropertySemantics::Text)
    , gridOffset_("gridOffset", "Grid Offset", 0, {-10.0, ConstraintBehavior::Ignore},
                  {10.0, ConstraintBehavior::Ignore}, 0.1, InvalidationLevel::InvalidOutput,
                  PropertySemantics::Text) {
    addProperties(numCells_, cellSize_, gridOffset_);
}

DimensionProperty::DimensionProperty(const DimensionProperty& rhs)
    : CompositeProperty{rhs}
    , numCells_{rhs.numCells_}
    , cellSize_{rhs.cellSize_}
    , gridOffset_{rhs.gridOffset_} {
    addProperties(numCells_, cellSize_, gridOffset_);
}
DimensionProperty* DimensionProperty::clone() const { return new DimensionProperty(*this); }

const std::string DimensionProperty::classIdentifier = "org.inviwo.GridDimensionProperty";

std::string DimensionProperty::getClassIdentifier() const { return classIdentifier; }

}  // namespace discretedata
}  // namespace inviwo
