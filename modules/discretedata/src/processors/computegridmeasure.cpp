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
    , propChannelCoordinates("ChannelCoordinates", "Coordinates", &dataInport,
                             [](const std::shared_ptr<const Channel> a) {
                                 return (a->getGridPrimitiveType() == GridPrimitive::Vertex &&
                                         a->getNumComponents() == 3);
                             })
    , dimensionToMeasure("dimensionToMeasure", "Grid Primitive to compute Measure on")
    , propMapToVertices("MapToVertices", "Map to Vertices", true) {
    addPort(dataInport);
    addPort(dataOutport);
    addProperty(propChannelCoordinates);
    addProperty(dimensionToMeasure);
    addProperty(propMapToVertices);

    dataInport.onConnect([this]() { this->updatePrimitiveOptions(); });
    dataInport.onChange([this]() { this->updatePrimitiveOptions(); });
}

void ComputeGridMeasure::updatePrimitiveOptions() {
    dimensionToMeasure.clearOptions();

    // Nothing connected, nothing to choose
    if (!dataInport.getData()) return;
    std::shared_ptr<const Connectivity> grid = dataInport.getData()->getGrid();
    if (!grid) return;

    for (int dim = 1; dim <= (ind)grid->getDimension(); dim++) {
        std::string dimString = "" + std::to_string(dim) + "D";
        std::string displayString = dimString;
        switch (dim) {
            case 1:
                displayString = "Length of Edges";
                break;
            case 2:
                displayString = "Area of Faces";
                break;
            case 3:
                displayString = "Volume of Voxels / Tetrahedra";
                break;
            default:
                break;
        }
        dimensionToMeasure.addOption(dimString, displayString, dim);
        dimensionToMeasure.set((int)grid->getDimension());
    }
}

void ComputeGridMeasure::process() {

    updatePrimitiveOptions();

    if (!dataInport.getData()) return;
    const auto& grid = dataInport.getData()->getGrid();
    GridPrimitive dimensionToProcess = (GridPrimitive)dimensionToMeasure.get();
    if ((ind)dimensionToProcess < 1 || dimensionToProcess > grid->getDimension()) return;

    auto VertexCoordinates = propChannelCoordinates.getCurrentChannel();
    if (!VertexCoordinates) return;

    // Generate output data
    auto outData = std::make_shared<DataSet>(*dataInport.getData());

    // Setup buffer to store data in
    const ind numElements = grid->getNumElements(dimensionToProcess);
    auto VolumeBuffer = std::make_shared<BufferChannel<double, 1>>(
        numElements, "Measure " + dimensionToMeasure.getSelectedDisplayName(), dimensionToProcess);

    // Compute measures per element
    for (auto element : grid->all(dimensionToProcess)) {
        (VolumeBuffer->get(element.getIndex())) =
            euclidean::getMeasure(*VertexCoordinates, element);
    }

    // Add to output
    outData->addChannel(VolumeBuffer);

    // Map back to vertices?
    if (propMapToVertices.get() && dimensionToProcess == GridPrimitive::Volume) {
        // Map volume from cell to vertices
        std::vector<ind> CellNeighs;
        auto VolumeDataVert = std::make_shared<BufferChannel<double>>(
            grid->getNumElements(GridPrimitive::Vertex), "Volume", GridPrimitive::Vertex);
        for (const auto& Vertex : grid->all(GridPrimitive::Vertex)) {
            // For all cells neighboring the vertex
            grid->getConnections(CellNeighs, Vertex.getIndex(), GridPrimitive::Vertex,
                                 GridPrimitive::Volume);

            // Each vertex gets an eigth of each neighboring cube
            double VertexVolume(0);
            for (const auto& Cell : CellNeighs) {
                VertexVolume += VolumeBuffer->get(Cell) / 8.0;
            }

            // Set it
            VolumeDataVert->get(Vertex.getIndex()) = VertexVolume;
        }

        outData->addChannel(VolumeDataVert);
    }

    dataOutport.setData(outData);
}

}  // namespace discretedata
}  // namespace inviwo
