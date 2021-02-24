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

#include <modules/discretedata/processors/createconstantchannel.h>

namespace inviwo {
namespace discretedata {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CreateConstantChannel::processorInfo_{
    "org.inviwo.CreateConstantChannel",  // Class identifier
    "Create Constant Channel",           // Display name
    "Data Set",                          // Category
    CodeState::Experimental,             // Code state
    Tags::None,                          // Tags
};

const ProcessorInfo CreateConstantChannel::getProcessorInfo() const { return processorInfo_; }

CreateConstantChannel::CreateConstantChannel()
    : Processor()
    , dataInport("InputData")
    , dataOutport("ExtendedData")
    , name_("name", "Name", "Constant")
    , format_("format", "Format")
    , primitive_("primitive", "Primitive")
    , numComponents_("numComponents", "Number of Components", 1, 1, 7)
    , value_("value", "Value", 1.0, 0.0, 10.0) {

    for (int format = static_cast<int>(DataFormatId::Float16);
         format <= static_cast<int>(DataFormatId::UInt64); ++format) {
        std::string name =
            std::string(DataFormatBase::get(static_cast<DataFormatId>(format))->getString());
        format_.addOption(name, name, format);
    }

    addPort(dataInport);
    addPort(dataOutport);
    addProperty(name_);
    addProperty(primitive_);
    addProperty(numComponents_);
    addProperty(format_);
    addProperty(value_);

    primitive_.addOption(primitiveName(GridPrimitive::Vertex), primitiveName(GridPrimitive::Vertex),
                         GridPrimitive::Vertex);
}

void CreateConstantChannel::process() {

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

    // Dispatch to create channel
    CreateChannelDispatcher dispatcher;
    Channel* channel = channeldispatching::dispatch<Channel*, dispatching::filter::Scalars, 1, 7>(
        static_cast<DataFormatId>(format_.get()), numComponents_.get(), dispatcher, value_.get(),
        name_.get(), dimensionToProcess, grid->getNumElements(dimensionToProcess));

    // Generate output data
    auto outData = std::make_shared<DataSet>(*dataInport.getData());
    outData->addChannel(channel);
    dataOutport.setData(outData);
}

}  // namespace discretedata
}  // namespace inviwo
