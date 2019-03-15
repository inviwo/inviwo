/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
    auto dataFrame = dataFrame_.getData();

    colors_.setData(
        selectedColorAxis_.getBuffer()
            ->getRepresentation<BufferRAM>()
            ->dispatch<std::shared_ptr<std::vector<vec4>>, dispatching::filter::Scalars>(
                [&](auto buf) {
                    auto colors = std::make_shared<std::vector<vec4>>();
                    auto &vec = buf->getDataContainer();
                    auto minMax = std::minmax_element(vec.begin(), vec.end());
                    double minV = static_cast<double>(*minMax.first);
                    double maxV = static_cast<double>(*minMax.second);
                    const double range = (maxV - minV);

                    for (const auto &v : vec) {
                        colors->push_back(tf_.get().sample((v - minV) / range));
                    }

                    return colors;
                }));
}

}  // namespace plot

}  // namespace inviwo
