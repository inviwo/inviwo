/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <modules/plotting/processors/dataframecolumntocolorvector.h>

#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/tfprimitive.h>                     // for TFPrimitiveData
#include <inviwo/core/datastructures/transferfunction.h>                // for TransferFunction
#include <inviwo/core/ports/datainport.h>                               // for DataInport
#include <inviwo/core/ports/dataoutport.h>                              // for DataOutport
#include <inviwo/core/ports/outportiterable.h>                          // for OutportIterable
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags
#include <inviwo/core/properties/transferfunctionproperty.h>            // for TransferFunctionP...
#include <inviwo/core/util/formatdispatching.h>                         // for Scalars
#include <inviwo/core/util/glmvec.h>                                    // for vec4, uvec3
#include <inviwo/core/util/staticstring.h>                              // for operator+
#include <inviwo/dataframe/properties/columnoptionproperty.h>           // for ColumnOptionProperty

#include <algorithm>      // for minmax_element
#include <functional>     // for __base
#include <memory>         // for shared_ptr, make_...
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set

#include <fmt/core.h>  // for format, format_to

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameColumnToColorVector::processorInfo_{
    "org.inviwo.DataFrameColumnToColorVector",  // Class identifier
    "DataFrame Column To Color Vector",         // Display name
    "Plotting",                                 // Category
    CodeState::Stable,                          // Code state
    "CPU, Plotting, DataFrame"                  // Tags
};
const ProcessorInfo DataFrameColumnToColorVector::getProcessorInfo() const {
    return processorInfo_;
}

DataFrameColumnToColorVector::DataFrameColumnToColorVector()
    : Processor()
    , dataFrame_("dataFrame")
    , colors_("colors")
    , selectedColorAxis_("selectedColorAxis", "Selected Color Axis", dataFrame_)
    , tf_("tf", "Color Mapping",
          TransferFunction(
              {{0.0f, vec4(1, 0, 0, 1)}, {0.5f, vec4(1, 1, 0, 1)}, {1.0f, vec4(0, 1, 0, 1)}})) {

    addPort(dataFrame_);
    addPort(colors_);
    addProperty(selectedColorAxis_);
    addProperty(tf_);
}

void DataFrameColumnToColorVector::process() {
    auto buffer =
        dataFrame_.getData()->getColumn(selectedColorAxis_.getSelectedValue())->getBuffer();

    colors_.setData(
        buffer->getRepresentation<BufferRAM>()
            ->dispatch<std::shared_ptr<std::vector<vec4>>, dispatching::filter::Scalars>(
                [&](auto buf) {
                    auto colors = std::make_shared<std::vector<vec4>>();
                    auto& vec = buf->getDataContainer();
                    auto minMax = std::minmax_element(vec.begin(), vec.end());
                    double minV = static_cast<double>(*minMax.first);
                    double maxV = static_cast<double>(*minMax.second);
                    const double range = (maxV - minV);

                    for (const auto& v : vec) {
                        colors->push_back(tf_.get().sample((v - minV) / range));
                    }

                    return colors;
                }));
}

}  // namespace plot

}  // namespace inviwo
