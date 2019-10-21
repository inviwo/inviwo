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

#include <modules/plottinggl/processors/scatterplotprocessor.h>
#include <inviwo/core/util/zip.h>
#include <modules/opengl/openglutils.h>
#include <inviwo/core/interaction/events/pickingevent.h>

#include <inviwo/dataframe/datastructures/dataframeutil.h>

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ScatterPlotProcessor::processorInfo_{
    "org.inviwo.ScatterPlotProcessor",  // Class identifier
    "Scatter Plot",                     // Display name
    "Plotting",                         // Category
    CodeState::Stable,                  // Code state
    "GL, Plotting",                     // Tags
};

const ProcessorInfo ScatterPlotProcessor::getProcessorInfo() const { return processorInfo_; }

ScatterPlotProcessor::ScatterPlotProcessor()
    : Processor()
    , dataFramePort_("dataFrame_")
    , brushingPort_("brushing")
    , backgroundPort_("background")
    , outport_("outport")
    , scatterPlot_(this)
    , xAxis_("xAxis", "X-axis", dataFramePort_, false, 0)
    , yAxis_("yAxis", "Y-axis", dataFramePort_, false, 2)
    , colorCol_("colorCol", "Color column", dataFramePort_, true, 3)
    , radiusCol_("radiusCol", "Radius column", dataFramePort_, true, 4) {

    addPort(dataFramePort_);
    addPort(brushingPort_);
    addPort(backgroundPort_);
    addPort(outport_);

    brushingPort_.setOptional(true);
    backgroundPort_.setOptional(true);

    tooltipCallBack_ = scatterPlot_.addToolTipCallback([this](PickingEvent* p, size_t rowId) {
        if (!p) return;
        if (auto dataframe = dataFramePort_.getData()) {
            p->setToolTip(dataframeutil::createToolTipForRow(*dataFramePort_.getData(), rowId));
        }
    });
    selectionChangedCallBack_ =
        scatterPlot_.addSelectionChangedCallback([this](const std::unordered_set<size_t>& indices) {
            brushingPort_.sendSelectionEvent(indices);
        });

    scatterPlot_.properties_.margins_.setLowerLeftMargin({50.0f, 40.0f});
    scatterPlot_.properties_.xAxis_.captionSettings_.setChecked(true);
    scatterPlot_.properties_.xAxis_.captionSettings_.offset_.set(20.0f);
    scatterPlot_.properties_.yAxis_.captionSettings_.setChecked(true);
    scatterPlot_.properties_.yAxis_.captionSettings_.offset_.set(30.0f);

    addProperty(scatterPlot_.properties_);
    addProperty(xAxis_);
    addProperty(yAxis_);
    addProperty(colorCol_);
    addProperty(radiusCol_);

    xAxis_.onChange([this]() { onXAxisChange(); });
    yAxis_.onChange([this]() { onYAxisChange(); });
    colorCol_.onChange([this]() { onColorChange(); });
    radiusCol_.onChange([this]() { onRadiusChange(); });

    dataFramePort_.onChange([this]() {
        onXAxisChange();
        onYAxisChange();
        onColorChange();
        onRadiusChange();

        if (dataFramePort_.hasData()) {
            scatterPlot_.setIndexColumn(dataFramePort_.getData()->getIndexColumn());
        }
    });
}

void ScatterPlotProcessor::process() {
    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    auto dataframe = dataFramePort_.getData();

    if (brushingPort_.isConnected()) {
        if (brushingPort_.isChanged()) {
            scatterPlot_.setSelectedIndices(brushingPort_.getSelectedIndices());
        }

        auto dfSize = dataframe->getNumberOfRows();

        auto iCol = dataframe->getIndexColumn();
        auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

        auto brushedIndicies = brushingPort_.getFilteredIndices();
        IndexBuffer indicies;
        auto& vec = indicies.getEditableRAMRepresentation()->getDataContainer();
        vec.reserve(dfSize - brushedIndicies.size());

        auto seq = util::sequence<uint32_t>(0, static_cast<uint32_t>(dfSize), 1);
        std::copy_if(seq.begin(), seq.end(), std::back_inserter(vec),
                     [&](const auto& id) { return !brushingPort_.isFiltered(indexCol[id]); });

        if (backgroundPort_.hasData()) {
            scatterPlot_.plot(*outport_.getEditableData(), *backgroundPort_.getData(), &indicies,
                              true);
        } else {
            scatterPlot_.plot(*outport_.getEditableData(), &indicies, true);
        }

    } else {
        if (backgroundPort_.hasData()) {
            scatterPlot_.plot(*outport_.getEditableData(), *backgroundPort_.getData(), nullptr,
                              true);
        } else {
            scatterPlot_.plot(*outport_.getEditableData(), nullptr, true);
        }
    }
}

void ScatterPlotProcessor::onXAxisChange() {
    if (!dataFramePort_.hasData()) return;
    auto data = dataFramePort_.getData();
    auto idx = xAxis_.get();
    scatterPlot_.setXAxis(data->getColumn(idx));
}

void ScatterPlotProcessor::onYAxisChange() {
    if (!dataFramePort_.hasData()) return;
    auto data = dataFramePort_.getData();
    auto idx = yAxis_.get();
    scatterPlot_.setYAxis(data->getColumn(idx));
}

void ScatterPlotProcessor::onColorChange() {
    if (!dataFramePort_.hasData()) return;
    auto data = dataFramePort_.getData();
    auto idx = colorCol_.get();
    if (idx == -1) {
        scatterPlot_.setColorData(nullptr);
    } else {
        auto buffer = data->getColumn(idx)->getBuffer();
        scatterPlot_.setColorData(buffer);
    }
}

void ScatterPlotProcessor::onRadiusChange() {
    if (!dataFramePort_.hasData()) return;
    auto data = dataFramePort_.getData();
    auto idx = radiusCol_.get();
    if (idx == -1) {
        scatterPlot_.setRadiusData(nullptr);
    } else {
        auto buffer = data->getColumn(idx)->getBuffer();
        scatterPlot_.setRadiusData(buffer);
    }
}

}  // namespace plot

}  // namespace inviwo
