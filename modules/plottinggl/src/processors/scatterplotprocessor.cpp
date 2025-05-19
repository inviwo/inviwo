/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/bitset.h>                         // for BitSet, BitSet::Bi...
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>      // for BufferRAMPrecision
#include <inviwo/core/interaction/events/pickingevent.h>               // for PickingEvent
#include <inviwo/core/ports/imageport.h>                               // for BaseImageInport
#include <inviwo/core/processors/processor.h>                          // for Processor
#include <inviwo/core/processors/processorinfo.h>                      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                     // for CodeState, CodeSta...
#include <inviwo/core/processors/processortags.h>                      // for Tags
#include <inviwo/core/properties/optionproperty.h>                     // for OptionPropertyOption
#include <inviwo/core/properties/ordinalproperty.h>                    // for FloatProperty
#include <inviwo/core/properties/marginproperty.h>                     // for MarginProperty
#include <inviwo/core/util/staticstring.h>                             // for operator+
#include <inviwo/core/util/zip.h>                                      // for enumerate, zipIter...
#include <inviwo/dataframe/datastructures/dataframe.h>                 // for DataFrameInport
#include <inviwo/dataframe/properties/columnoptionproperty.h>          // for ColumnOptionProperty
#include <inviwo/dataframe/util/dataframeutil.h>                       // for createToolTipForRow
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>  // for BrushingAndLinking...
#include <modules/opengl/inviwoopengl.h>                               // for GL_ONE, GL_ONE_MIN...
#include <modules/opengl/openglutils.h>                                // for BlendModeState
#include <modules/opengl/texture/textureutils.h>                       // for ImageInport
#include <modules/plotting/properties/axisproperty.h>                  // for AxisProperty
#include <modules/plotting/properties/plottextproperty.h>              // for PlotTextProperty
#include <modules/plottinggl/plotters/scatterplotgl.h>                 // for ScatterPlotGL, Sca...

#include <type_traits>  // for remove_extent_t
#include <utility>      // for pair

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ScatterPlotProcessor::processorInfo_{
    "org.inviwo.ScatterPlotProcessor",  // Class identifier
    "Scatter Plot",                     // Display name
    "Plotting",                         // Category
    CodeState::Stable,                  // Code state
    "GL, Plotting",                     // Tags
    "Renders a scatter plot for a given DataFrame."_help,
};

const ProcessorInfo& ScatterPlotProcessor::getProcessorInfo() const { return processorInfo_; }

ScatterPlotProcessor::ScatterPlotProcessor()
    : Processor()
    , dataFramePort_("dataFrame_", "data input for plotting"_help)
    , brushingPort_("brushing", "inport for brushing & linking interactions"_help)
    , backgroundPort_("background")
    , outport_("outport", "rendered image of the scatter plot"_help)
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
            if (auto data = dataFramePort_.getData()) {
                BitSet indices;
                auto iCol = data->getIndexColumn();
                auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();
                for (auto idx : highlighted) {
                    indices.add(indexCol[idx]);
                }
                brushingPort_.highlight(indices);
            }
        });
    selectionChangedCallBack_ =
        scatterPlot_.addSelectionChangedCallback([this](const BitSet& selected) {
            if (auto data = dataFramePort_.getData()) {
                BitSet indices;
                auto iCol = data->getIndexColumn();
                auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();
                for (auto idx : selected) {
                    indices.add(indexCol[idx]);
                }
                brushingPort_.select(indices);
            }
        });
    filteringChangedCallBack_ =
        scatterPlot_.addFilteringChangedCallback([this](const BitSet& filtered) {
            if (auto data = dataFramePort_.getData()) {
                BitSet indices;
                auto iCol = data->getIndexColumn();
                auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();
                for (auto idx : filtered) {
                    indices.add(indexCol[idx]);
                }
                brushingPort_.filter("scatterplot", indices);
            }
        });
    addInteractionHandler(&scatterPlot_);
    scatterPlot_.properties_.margins_.setLowerLeftMargin({65.0f, 60.0f});
    scatterPlot_.properties_.xAxis_.captionSettings_.setChecked(true);
    scatterPlot_.properties_.yAxis_.captionSettings_.setChecked(true);
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

    if (dataFramePort_.isChanged()) {
        indexToRowMap_ = [&]() {
            auto iCol = dataFramePort_.getData()->getIndexColumn();
            auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();
            std::unordered_map<uint32_t, uint32_t> indexToRow;
            indexToRow.reserve(indexCol.size());
            for (auto&& [row, index] : util::enumerate(indexCol)) {
                indexToRow.try_emplace(index, static_cast<uint32_t>(row));
            }
            return indexToRow;
        }();
    }

    auto transformIdsToRows = [&](const BitSet& b) {
        BitSet rows;
        for (auto id : b) {
            auto it = indexToRowMap_.find(id);
            if (it != indexToRowMap_.end()) {
                rows.add(it->second);
            }
        }
        return rows;
    };

    if (brushingPort_.isSelectionModified() || dataFramePort_.isChanged()) {
        scatterPlot_.setSelectedIndices(transformIdsToRows(brushingPort_.getSelectedIndices()));
    }
    if (brushingPort_.isHighlightModified() || dataFramePort_.isChanged()) {
        scatterPlot_.setHighlightedIndices(
            transformIdsToRows(brushingPort_.getHighlightedIndices()));
    }
    if (brushingPort_.isFilteringModified() || dataFramePort_.isChanged()) {
        scatterPlot_.setFilteredIndices(transformIdsToRows(brushingPort_.getFilteredIndices()));
    }

    if (backgroundPort_.isReady()) {
        scatterPlot_.plot(outport_, backgroundPort_, true);
    } else {
        scatterPlot_.plot(outport_, true);
    }
}

}  // namespace plot

}  // namespace inviwo
