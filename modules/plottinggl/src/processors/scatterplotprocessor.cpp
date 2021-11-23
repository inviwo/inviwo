/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <modules/opengl/openglutils.h>

#include <inviwo/dataframe/util/dataframeutil.h>

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
    , xAxis_("xAxis", "X-axis", dataFramePort_, ColumnOptionProperty::AddNoneOption::No, 0)
    , yAxis_("yAxis", "Y-axis", dataFramePort_, ColumnOptionProperty::AddNoneOption::No, 2)
    , colorCol_("colorCol", "Color column", dataFramePort_,
                ColumnOptionProperty::AddNoneOption::Yes, 3)
    , radiusCol_("radiusCol", "Radius column", dataFramePort_,
                 ColumnOptionProperty::AddNoneOption::Yes, 4)
    , sortCol_("sortCol", "Sorting column", dataFramePort_,
               ColumnOptionProperty::AddNoneOption::Yes, 0)
    , sortOrder_("sortOrder", "Sorting Order",
                 {{"ascending", "Ascending", ScatterPlotGL::SortingOrder::Ascending},
                  {"descending", "Descending", ScatterPlotGL::SortingOrder::Descending}}) {

    addPort(dataFramePort_);
    addPort(brushingPort_).setOptional(true);
    addPort(backgroundPort_).setOptional(true);
    addPort(outport_);

    tooltipCallBack_ = scatterPlot_.addToolTipCallback([this](PickingEvent* p, size_t rowId) {
        if (!p) return;
        if (auto dataframe = dataFramePort_.getData()) {
            p->setToolTip(dataframe::createToolTipForRow(*dataFramePort_.getData(), rowId));
        }
    });
    highlightChangedCallBack_ =
        scatterPlot_.addHighlightChangedCallback([this](const BitSet& highlighted) {
            if (brushingPort_.isConnected()) {
                BitSet indices;
                auto iCol = dataFramePort_.getData()->getIndexColumn();
                auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();
                for (auto idx : highlighted) {
                    indices.add(indexCol[idx]);
                }
                brushingPort_.highlight(indices);
            } else {
                invalidate(InvalidationLevel::InvalidOutput);
            }
        });
    selectionChangedCallBack_ =
        scatterPlot_.addSelectionChangedCallback([this](const std::vector<bool>& selected) {
            if (brushingPort_.isConnected()) {
                BitSet selectedIndices;
                auto iCol = dataFramePort_.getData()->getIndexColumn();
                auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();
                for (size_t i = 0; i < selected.size(); ++i) {
                    if (selected[i]) selectedIndices.add(indexCol[i]);
                }
                brushingPort_.select(selectedIndices);
            } else {
                invalidate(InvalidationLevel::InvalidOutput);
            }
        });
    filteringChangedCallBack_ =
        scatterPlot_.addFilteringChangedCallback([this](const std::vector<bool>& filtered) {
            if (brushingPort_.isConnected()) {
                BitSet filteredIndices;
                auto iCol = dataFramePort_.getData()->getIndexColumn();
                auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();
                for (size_t i = 0; i < filtered.size(); ++i) {
                    if (filtered[i]) filteredIndices.add(indexCol[i]);
                }
                brushingPort_.filter("scatterplot", filteredIndices);
            } else {
                invalidate(InvalidationLevel::InvalidOutput);
            }
        });
    addInteractionHandler(&scatterPlot_);
    scatterPlot_.properties_.margins_.setLowerLeftMargin({50.0f, 40.0f});
    scatterPlot_.properties_.xAxis_.captionSettings_.setChecked(true);
    scatterPlot_.properties_.xAxis_.captionSettings_.offset_.set(20.0f);
    scatterPlot_.properties_.yAxis_.captionSettings_.setChecked(true);
    scatterPlot_.properties_.yAxis_.captionSettings_.offset_.set(30.0f);
    scatterPlot_.properties_.setCurrentStateAsDefault();

    addProperties(scatterPlot_.properties_, xAxis_, yAxis_, colorCol_, radiusCol_, sortCol_,
                  sortOrder_);

    auto setColumn = [this](const ColumnOptionProperty& property, auto memberfunc) {
        if (!dataFramePort_.hasData()) return;
        if (auto idx = property.get(); idx == -1) {
            std::invoke(memberfunc, scatterPlot_, nullptr);
        } else {
            std::invoke(memberfunc, scatterPlot_, dataFramePort_.getData()->getColumn(idx).get());
        }
    };

    xAxis_.onChange([this, setColumn]() { setColumn(xAxis_, &ScatterPlotGL::setXAxis); });
    yAxis_.onChange([this, setColumn]() { setColumn(yAxis_, &ScatterPlotGL::setYAxis); });
    colorCol_.onChange([this, setColumn]() { setColumn(colorCol_, &ScatterPlotGL::setColorData); });
    radiusCol_.onChange(
        [this, setColumn]() { setColumn(radiusCol_, &ScatterPlotGL::setRadiusData); });
    sortCol_.onChange([this, setColumn]() { setColumn(sortCol_, &ScatterPlotGL::setSortingData); });
    sortOrder_.onChange([this]() { scatterPlot_.setSortingOrder(sortOrder_); });

    dataFramePort_.onChange([this, setColumn]() {
        setColumn(xAxis_, &ScatterPlotGL::setXAxis);
        setColumn(yAxis_, &ScatterPlotGL::setYAxis);
        setColumn(colorCol_, &ScatterPlotGL::setColorData);
        setColumn(radiusCol_, &ScatterPlotGL::setRadiusData);
        setColumn(sortCol_, &ScatterPlotGL::setSortingData);

        if (dataFramePort_.hasData()) {
            scatterPlot_.setIndexColumn(dataFramePort_.getData()->getIndexColumn());
        }
    });
}

void ScatterPlotProcessor::process() {
    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    auto dataframe = dataFramePort_.getData();

    auto indexToRowMap = [&]() {
        auto iCol = dataframe->getIndexColumn();
        auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();
        std::unordered_map<uint32_t, uint32_t> indexToRow;
        for (auto&& [row, index] : util::enumerate(indexCol)) {
            indexToRow.try_emplace(index, static_cast<uint32_t>(row));
        }
        return indexToRow;
    }();

    auto transformIdsToRows = [&](const BitSet& b) {
        BitSet rows;
        for (const auto& id : b) {
            auto it = indexToRowMap.find(id);
            if (it != indexToRowMap.end()) {
                rows.add(it->second);
            }
        }
        return rows;
    };

    if (brushingPort_.isSelectionModified()) {
        scatterPlot_.setSelectedIndices(transformIdsToRows(brushingPort_.getSelectedIndices()));
    }
    if (brushingPort_.isHighlightModified()) {
        scatterPlot_.setHighlightedIndices(
            transformIdsToRows(brushingPort_.getHighlightedIndices()));
    }

    BitSet b(brushingPort_.getFilteredIndices());
    // invert the bitset to obtain the ids remaining after the filtering
    b.flipRange(0, static_cast<uint32_t>(dataframe->getNumberOfRows()));
    b = transformIdsToRows(b);

    IndexBuffer indicies;
    auto& vec = indicies.getEditableRAMRepresentation()->getDataContainer();
    vec = b.toVector();

    if (backgroundPort_.isReady()) {
        scatterPlot_.plot(outport_, backgroundPort_, &indicies, true);
    } else {
        scatterPlot_.plot(outport_, &indicies, true);
    }
}

}  // namespace plot

}  // namespace inviwo
