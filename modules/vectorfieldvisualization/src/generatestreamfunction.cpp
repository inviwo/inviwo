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

#include <modules/vectorfieldvisualization/generatestreamfunction.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <queue>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GenerateStreamFunction::processorInfo_{
    "org.inviwo.GenerateStreamFunction",      // Class identifier
    "Generate Stream Function",                // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo GenerateStreamFunction::getProcessorInfo() const { return processorInfo_; }

GenerateStreamFunction::GenerateStreamFunction()
    : Processor()
    , vectorFieldIn_("vectorField")
    , streamFunctionOut_("streamFunction")
    , seedPosition_("seedPosition", "Seed Position", {1,1},
                    {{0,0}, ConstraintBehavior::Immutable},
                    {{100,100}, ConstraintBehavior::Mutable}, {1,1} )
    , dataRange_("dataRange", "Data Range", vec2(0,0),
                 {{0,0}, ConstraintBehavior::Ignore},
                 {{1,1}, ConstraintBehavior::Ignore})
    , singleUpdateOnly_("singleUpdate", "Single Update Only?", true) {

    vectorFieldIn_.setOutportDeterminesSize(true);
    streamFunctionOut_.setHandleResizeEvents(false);
    dataRange_.setReadOnly(true);
    addPorts(vectorFieldIn_, streamFunctionOut_);
    vectorFieldIn_.onChange([this](){
        if (!vectorFieldIn_.hasData()) {
            seedPosition_.setMaxValue({1,1});
            return; 
        }
        seedPosition_.setMaxValue(vectorFieldIn_.getData()->getDimensions());
    });
    addProperties(seedPosition_, dataRange_, singleUpdateOnly_);
}

void GenerateStreamFunction::process() {
    streamFunctionOut_.clear();
    if (!vectorFieldIn_.hasData()) return;

    auto vectorField = vectorFieldIn_.getData();
    size2_t imageSize = vectorField->getDimensions();
    size_t numCells = imageSize.x * imageSize.y;
    const auto* ram = vectorField->getColorLayer()->getRepresentation<LayerRAM>();
    
    ram->dispatch<void, dispatching::filter::Vec2s>([&](const auto* typedRam){
        if (!typedRam) return;
        const auto* vectorData = typedRam->getDataTyped();
        size_t seedCellIdx = seedPosition_.get().x + seedPosition_.get().y * imageSize.x;
        if (vectorData[seedCellIdx].x == 0 && vectorData[seedCellIdx].y == 0 ) return;

        // Create output image.
        auto streamFunctionImage =
            std::make_shared<Image>(imageSize, DataFormat<float>::get());

        auto streamFunctionLayer = streamFunctionImage->getColorLayer();
        if (!streamFunctionLayer) return;

        auto streamFunctionRam =
            dynamic_cast<LayerRAMPrecision<float>*>(streamFunctionLayer->getEditableRepresentation<LayerRAM>());
        if (!streamFunctionRam) return;

        float* streamFunctionData = streamFunctionRam->getDataTyped();
        if (!streamFunctionData) return;

        char* sampleCount = new char[numCells];
        std::fill_n(streamFunctionData, numCells, 0);
        std::fill_n(sampleCount, numCells, 0);
        sampleCount[seedCellIdx] = 1; // Up this to one to circumvent some special treatment later on.

        // Create stream function.
        std::queue<size_t> todoCells;
        todoCells.push(seedCellIdx);
        
        while(!todoCells.empty()) {
            size_t cellIdx = todoCells.front();
            todoCells.pop();

            float scalar = streamFunctionData[cellIdx] /  sampleCount[cellIdx];
            vec2 gradient {vectorData[cellIdx].y, -vectorData[cellIdx].x};


            size2_t cellPos {cellIdx % imageSize.x, cellIdx / imageSize.x};

            for (int dir = 0; dir < 2; ++dir) {
                for (int sign = -1; sign <= 1; sign +=2) {
                    ivec2 neighborPos{cellPos.x, cellPos.y};
                    neighborPos[dir] += sign;
                    size_t neighborCell = neighborPos.x + neighborPos.y * imageSize.x;
                    if (0 > neighborPos.x || neighborPos.x >= int(imageSize.x) ||
                        0 > neighborPos.y || neighborPos.y >= int(imageSize.y) || 
                        (vectorData[neighborCell].x == 0 && vectorData[neighborCell].y ==0)
                        
                         //|| sampleCount[neighborCell] != 0
                        
                        ) continue;
                    if (singleUpdateOnly_.get() && sampleCount[neighborCell] != 0) continue;
                    // if (sampleCount[neighborCell] == 0) 
                    //float tmpCopy = streamFunctionData[neighborCell];
                    streamFunctionData[neighborCell] += scalar + (gradient[dir] * float(sign));

                    /*static size_t numUrghs = 0;
                    if (numUrghs < 100 && std::abs(streamFunctionData[neighborCell]) > 2*numCells*4) {
                        numUrghs++;
                        std::cout << fmt::format("Cell {}: \t{} = {} + {} + ({} * {})", neighborCell, streamFunctionData[neighborCell] , tmpCopy, scalar, gradient[dir], sign) << std::endl;
                    }*/
                    // Cell: 162042
                    if (sampleCount[neighborCell] == 0 && neighborCell != seedCellIdx) todoCells.push(neighborCell);
                    sampleCount[neighborCell]++; //+= (sampleCount[neighborCell] >= 0)? 1 : -1;
                }
            }
        }
        sampleCount[seedCellIdx]--;
        // std::cout << "Num good cells: " << numGoodCells << std::endl; 
        vec2 minMax {std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest()};
        for (size_t idx = 0; idx < numCells; ++idx) {
            if (std::isnan(streamFunctionData[idx]) || std::isinf(streamFunctionData[idx])) 
                streamFunctionData[idx] = 0;
            if (sampleCount[idx] != 0)
                streamFunctionData[idx] /= std::abs(sampleCount[idx]);
            minMax.x = std::min(minMax.x, streamFunctionData[idx]);
            minMax.y = std::max(minMax.y, streamFunctionData[idx]);
        }
        for (size_t idx = 0; idx < numCells; ++idx) {
            if (streamFunctionData[idx] == 0)
                streamFunctionData[idx] = minMax.x;
        }
        streamFunctionData[seedCellIdx] = minMax.y*10;

        dataRange_.setReadOnly(false);
        dataRange_.set(minMax);
        dataRange_.setReadOnly(true);

        delete[] sampleCount;
        streamFunctionOut_.setData(streamFunctionImage);
    });
}   

}  // namespace inviwo
