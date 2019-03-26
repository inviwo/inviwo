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

#include <modules/discretedata/processors/computegridmeasure.h>
#include <modules/discretedata/connectivity/euclideanmeasure.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ComputeGridMeasure::processorInfo_{
    "org.inviwo.ComputeGridMeasure",  // Class identifier
    "Compute Grid Measure",           // Display name
    "Data Set",                       // Category
    CodeState::Experimental,          // Code state
    Tags::None,                       // Tags
};

const ProcessorInfo ComputeGridMeasure::getProcessorInfo() const { return processorInfo_; }

ComputeGridMeasure::ComputeGridMeasure()
    : Processor()
    , dataInport("InputData")
    , dataOutport("ExtendedData")
    , dimensionToMeasure("dimensionToMeasure", "Grid Primitive to compute Measure on") {
    addPort(dataInport);
    addPort(dataOutport);
    addProperty(dimensionToMeasure);

    dataInport.onConnect([this]() { this->updatePrimitiveOptions(); });
    dataInport.onChange([this]() { this->updatePrimitiveOptions(); });
}

void ComputeGridMeasure::updatePrimitiveOptions() {
    std::shared_ptr<const Connectivity> grid;
    dimensionToMeasure.clearOptions();

    // Nothing connected, nothing to choose
    if (!dataInport.getData() || (grid = dataInport.getData()->grid)) return;

    for (int dim = 1; dim < (ind)grid->getDimension(); ++dim) {
        std::string dimString = "" + std::to_string(dim) + "D";
        std::string addString = "";
        switch (dim) {
            case 1:
                addString = " (Edges)";
                break;
            case 2:
                addString = " (Faces)";
                break;
            case 3:
                addString = " (Volumes)";
                break;
            default:
                break;
        }
        dimensionToMeasure.addOption(dimString, dimString + addString, dim);
        dimensionToMeasure.set((int)grid->getDimension());
    }
}

void ComputeGridMeasure::process() {
    if (!dataInport.getData()) return;
    const auto& grid = dataInport.getData()->grid;
    GridPrimitive dimensionToProcess = (GridPrimitive)dimensionToMeasure.get();

    if ((ind)dimensionToProcess < 1 || dimensionToProcess > grid->getDimension()) return;

    // TODO: Dropdown.
    auto vertices = dataInport.getData()->getChannel("Position", dimensionToProcess);
    if (!vertices) return;

    // Setup buffer to store data in
    ind numElements = grid->getNumElements(dimensionToProcess);
    auto volumeBuffer = std::make_shared<BufferChannel<double, 1>>(
        numElements, "Measure" + dimensionToMeasure.getDisplayName(), dimensionToProcess);

    // Compute measures per element
    for (auto element : grid->all(dimensionToProcess))
        (volumeBuffer->get(element.getIndex())) = euclidean::getMeasure(*vertices, element);
    // grid->getPrimitiveMeasure(element);

    auto outData = std::make_shared<DataSet>(*dataInport.getData());

    outData->addChannel(volumeBuffer);
    dataOutport.setData(outData);
}

}  // namespace discretedata
}  // namespace inviwo
