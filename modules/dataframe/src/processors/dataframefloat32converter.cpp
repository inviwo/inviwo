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

#include <inviwo/dataframe/processors/dataframefloat32converter.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameFloat32Converter::processorInfo_{
    "org.inviwo.DataFrameFloat32Converter",  // Class identifier
    "DataFrame Float32 Converter",           // Display name
    "DataFrame",                             // Category
    CodeState::Stable,                       // Code state
    "CPU, Plotting, DataFrame",              // Tags
};
const ProcessorInfo DataFrameFloat32Converter::getProcessorInfo() const { return processorInfo_; }

DataFrameFloat32Converter::DataFrameFloat32Converter()
    : Processor(), inport_("inport"), outport_("outport") {

    addPort(inport_);
    addPort(outport_);
}

void DataFrameFloat32Converter::process() {
    auto dataframe = std::make_shared<DataFrame>();
    for (auto srcCol : *inport_.getData()) {
        if (srcCol == inport_.getData()->getIndexColumn()) continue;
        const auto df = srcCol->getBuffer()->getDataFormat();

        if ((df->getPrecision() == 64) && (df->getNumericType() == NumericType::Float)) {
            srcCol->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Floats>([dataframe, srcCol](auto typedBuf) {
                    using ValueType = util::PrecisionValueType<decltype(typedBuf)>;
                    using T = typename util::same_extent<ValueType, float>::type;

                    auto dst = util::transform(typedBuf->getDataContainer(),
                                               [](const auto& v) { return static_cast<T>(v); });
                    dataframe->addColumn(srcCol->getHeader(), std::move(dst));
                });
        } else {
            dataframe->addColumn(std::shared_ptr<Column>(srcCol->clone()));
        }
    }
    dataframe->updateIndexBuffer();

    outport_.setData(dataframe);
}

}  // namespace inviwo
