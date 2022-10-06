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

#include <modules/discretedata/processors/createindexchannel.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <type_traits>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CreateIndexChannel::processorInfo_{
    "org.inviwo.CreateIndexChannel",  // Class identifier
    "Create Index Channel",           // Display name
    "Data Set",                       // Category
    CodeState::Experimental,          // Code state
    Tags::None,                       // Tags
};

const ProcessorInfo CreateIndexChannel::getProcessorInfo() const { return processorInfo_; }

CreateIndexChannel::CreateIndexChannel()
    : Processor()
    , dataInport("InputData")
    , dataOutport("ExtendedData")
    , name_("name", "Name", "Indices")
    , primitive_("primitive", "Primitive")
    , normalize_("normalize", "Normalize", true) {

    addPort(dataInport);
    addPort(dataOutport);
    addProperties(name_, primitive_, normalize_);

    primitive_.addOption(primitiveName(GridPrimitive::Vertex), primitiveName(GridPrimitive::Vertex),
                         GridPrimitive::Vertex);
}

void CreateIndexChannel::process() {

    if (!dataInport.getData()) return;
    auto grid = dataInport.getData()->getGrid();
    GridPrimitive dimensionToProcess = primitive_.get();
    if (dimensionToProcess > grid->getDimension()) return;

    if (primitive_.size() != static_cast<size_t>(grid->getDimension()) + 1) {
        primitive_.clearOptions();
        for (ind dim = 0; dim <= static_cast<ind>(grid->getDimension()); ++dim)
            primitive_.addOption(primitiveName(static_cast<GridPrimitive>(dim)),
                                 primitiveName(static_cast<GridPrimitive>(dim)),
                                 static_cast<GridPrimitive>(dim));
    }
    dimensionToProcess = primitive_.get();
    ind numElements = grid->getNumElements(dimensionToProcess);

    Channel* channel;
    if (normalize_.get()) {
        channel = new AnalyticChannel<double, 1, double>(
            [size = numElements - 1](double& result, ind idx) { result = double(idx) / size; },
            numElements, name_.get(), dimensionToProcess);
    } else {
        channel = new AnalyticChannel<int, 1, int>([](int& result, ind idx) { result = idx; },
                                                   numElements, name_.get(), dimensionToProcess);
    }
    // channel->dispatch<void>([](auto* channel) {
    //     typename std::remove_pointer_t<decltype(channel)>::DefaultVec vec;

    //     for (ind i = 0; i < 1000; ++i) {
    //         channel->fill(vec, i);
    //         std::cout << i << ":  " << vec << std::endl;
    //     }
    // });

    // Generate output data
    auto outData = std::make_shared<DataSet>(*dataInport.getData());
    outData->addChannel(channel);
    dataOutport.setData(outData);
}

}  // namespace discretedata
}  // namespace inviwo
