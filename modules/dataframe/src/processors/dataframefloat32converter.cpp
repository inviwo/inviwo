/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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
#include <inviwo/core/util/glm.h>

#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags
#include <inviwo/core/util/formatdispatching.h>                         // for Floats, Precision...
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmutils.h>                                  // for same_extent
#include <inviwo/core/util/stdextensions.h>                             // for transform
#include <inviwo/dataframe/datastructures/dataframe.h>                  // for DataFrame, DataFr...

#include <memory>         // for shared_ptr, share...
#include <string>         // for string
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set
#include <utility>        // for move

namespace inviwo {
class Column;

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

    *dataframe->getIndexColumn() = *inport_.getData()->getIndexColumn();

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

                    dataframe->addColumn(srcCol->getHeader(), std::move(dst), srcCol->getUnit(),
                                         srcCol->getRange());
                });
        } else {
            dataframe->addColumn(std::shared_ptr<Column>(srcCol->clone()));
        }
    }

    outport_.setData(dataframe);
}

}  // namespace inviwo
