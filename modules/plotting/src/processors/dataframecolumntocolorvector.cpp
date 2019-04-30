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

namespace {
    // Copied from dendogramplotter.cpp
vec3 rgb(unsigned char r, unsigned char g, unsigned char b) {
    return {r / 255.0f, g / 255.0f, b / 255.0f};
}

// Generated at http://phrogz.net/css/distinct-colors.html
const static std::vector<vec3> semirandomcolors = {
    rgb(255, 0, 0),     rgb(0, 238, 255),   rgb(122, 0, 230),
    rgb(166, 141, 41),  rgb(32, 96, 128),   rgb(109, 86, 115),
    rgb(64, 0, 0),      rgb(115, 113, 86),  rgb(182, 222, 242),
    rgb(210, 108, 217), rgb(255, 191, 191), rgb(214, 230, 0),
    rgb(0, 88, 166),    rgb(255, 0, 238),   rgb(255, 145, 128),
    rgb(82, 102, 0),    rgb(0, 34, 64),     rgb(102, 26, 97),
    rgb(153, 41, 0),    rgb(222, 242, 182), rgb(128, 196, 255),
    rgb(204, 153, 187), rgb(242, 97, 0),    rgb(86, 191, 48),
    rgb(0, 102, 255),   rgb(242, 0, 129),   rgb(204, 143, 102),
    rgb(19, 77, 27),    rgb(128, 162, 255), rgb(51, 26, 32),
    rgb(51, 36, 26),    rgb(89, 179, 125),  rgb(67, 73, 89),
    rgb(178, 45, 62),   rgb(242, 157, 61),  rgb(0, 255, 170),
    rgb(0, 0, 140),     rgb(127, 64, 72),   rgb(115, 75, 29),
    rgb(32, 128, 121),  rgb(0, 0, 128),     rgb(230, 210, 172),
    rgb(38, 77, 74),    rgb(0, 0, 102)

};
}  // namespace

namespace util {
void makeDiscreteTF(TransferFunction &tf, const std::vector<vec3> &colors, size_t N = 0,
                    bool nodeCentered = true) {
    tf.clear();
    if (N == 0) N = colors.size();
    double dt = nodeCentered ? 1.0 / (N - 1.0) : 1.0 / N;
    for (size_t i = 0; i < N; i++) {
        const auto &c = colors[i % colors.size()];
        double x1;
        double x2;
        if (nodeCentered) {
            x1 = std::max(0., dt * i - dt / 2);
            x2 = std::min(1., dt * i + dt / 2) - std::numeric_limits<double>::epsilon();
        } else {
            x1 = i * dt;
            x2 = (i + 1) * dt - std::numeric_limits<double>::epsilon();
        }

        tf.add(x1, vec4(c, 1));
        tf.add(x2, vec4(c, 1));
    }
}
}  // namespace util

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
    , brushing_("brushing")
    , colors_("colors")
    , selectedColorAxis_("selectedColorAxis", "Selected Color Axis", dataFrame_)
    , tf_("tf", "Color Mapping",
          TransferFunction(
              {{0.0f, vec4(1, 0, 0, 1)}, {0.5f, vec4(1, 1, 0, 1)}, {1.0f, vec4(0, 1, 0, 1)}}))
    , useSelectedColumnFromBrushing_("useSelectedColumnFromBrushing",
                                     "Use Selected Column From Brushing port", true)
    , autoSetTF_("autoSetTF", "Auto update tf (for int buffers)", true) {

    addPort(dataFrame_);
    addPort(brushing_);
    addPort(colors_);
    addProperties(selectedColorAxis_, tf_, useSelectedColumnFromBrushing_, autoSetTF_);
}

void DataFrameColumnToColorVector::process() {
    auto dataFrame = dataFrame_.getData();

    bool updateTF = false;
    

    if (useSelectedColumnFromBrushing_.get() && brushing_.isConnected()) {
        auto selectedCols = brushing_.getSelectedColumns();
        if (selectedCols.size() > 2) {
            LogWarn("Multiple Columns selected, using only first");
        }
        if (!selectedCols.empty()) {
            auto cur = selectedColorAxis_.getSelectedIndex();
            auto newID = *selectedCols.begin();
            if(cur != newID){
                selectedColorAxis_.setSelectedIndex(newID);
                updateTF = true;
            }
            
        }
    }

    updateTF |= dataFrame_.isChanged();
    updateTF |= selectedColorAxis_.isModified();

    colors_.setData(
        selectedColorAxis_.getBuffer()
            ->getRepresentation<BufferRAM>()
            ->dispatch<std::shared_ptr<std::vector<vec4>>, dispatching::filter::Scalars>(
                [&](auto buf) {
                    using T = util::PrecisionValueType<decltype(buf)>;
                    auto &vec = buf->getDataContainer();
                    auto minMax = std::minmax_element(vec.begin(), vec.end());
                    double minV = static_cast<double>(*minMax.first);
                    double maxV = static_cast<double>(*minMax.second);

                    if((DataFormat<T>::get()->getNumericType() == NumericType::UnsignedInteger || 
                        DataFormat<T>::get()->getNumericType() == NumericType::SignedInteger ) &&
                        updateTF && autoSetTF_.get()){
                        size_t N = (*minMax.second) - (*minMax.first); 
                        if(N>100){
                            LogWarn("N is larger than a 100: " << N);
                            N = 100;
                        }
                        util::makeDiscreteTF(tf_.get() , semirandomcolors , N+1 , true); // plus 1 to include both min and max 
                        tf_.setModified();
                        LogWarn("How often is this called?")
                    }
                    auto colors = std::make_shared<std::vector<vec4>>();
                    const double range = (maxV - minV);
                    for (const auto &v : vec) {
                        colors->push_back(tf_.get().sample((v - minV) / range));
                    }

                    return colors;
                }));
}

}  // namespace plot

}  // namespace inviwo
