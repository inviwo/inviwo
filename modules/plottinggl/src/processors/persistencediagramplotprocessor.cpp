/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <modules/plottinggl/processors/persistencediagramplotprocessor.h>

#include <inviwo/core/datastructures/bitset.h>                         // for BitSet
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>      // for BufferRAMPrecision
#include <inviwo/core/interaction/events/pickingevent.h>               // for PickingEvent
#include <inviwo/core/ports/imageport.h>                               // for BaseImageInport
#include <inviwo/core/processors/processor.h>                          // for Processor
#include <inviwo/core/processors/processorinfo.h>                      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                     // for CodeState, CodeSta...
#include <inviwo/core/processors/processortags.h>                      // for Tags
#include <inviwo/core/util/zip.h>                                      // for sequence
#include <inviwo/dataframe/datastructures/dataframe.h>                 // for DataFrameInport
#include <inviwo/dataframe/properties/columnoptionproperty.h>          // for ColumnOptionProperty
#include <inviwo/dataframe/util/dataframeutil.h>                       // for createToolTipForRow
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>  // for BrushingAndLinking...
#include <modules/opengl/texture/textureutils.h>                       // for ImageInport
#include <modules/plottinggl/plotters/persistencediagramplotgl.h>      // for PersistenceDiagram...

#include <algorithm>    // for copy_if
#include <cstdint>      // for uint32_t
#include <iterator>     // for back_insert_iterator
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <vector>       // for vector

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PersistenceDiagramPlotProcessor::processorInfo_{
    "org.inviwo.PersistenceDiagramPlotProcessor",  // Class identifier
    "Persistence Diagram Plot",                    // Display name
    "Plotting",                                    // Category
    CodeState::Stable,                             // Code state
    "GL, Plotting, Topology",                      // Tags
};
const ProcessorInfo PersistenceDiagramPlotProcessor::getProcessorInfo() const {
    return processorInfo_;
}

PersistenceDiagramPlotProcessor::PersistenceDiagramPlotProcessor()
    : Processor()
    , dataFrame_("dataFrame")
    , brushingPort_("brushing")
    , backgroundPort_("background")
    , outport_("outport")
    , persistenceDiagramPlot_(this)
    , birth_("xAxis", "Birth", dataFrame_, ColumnOptionProperty::AddNoneOption::No, 1)
    , death_("yAxis", "Death", dataFrame_, ColumnOptionProperty::AddNoneOption::Yes, 2)
    , persistence_("persistence", "Persistence", dataFrame_,
                   ColumnOptionProperty::AddNoneOption::Yes, -1)
    , colorCol_("colorCol", "Color column", dataFrame_, ColumnOptionProperty::AddNoneOption::Yes,
                3) {

    addPort(dataFrame_);
    addPort(brushingPort_);
    addPort(backgroundPort_);
    addPort(outport_);

    brushingPort_.setOptional(true);
    backgroundPort_.setOptional(true);

    tooltipCallBack_ =
        persistenceDiagramPlot_.addToolTipCallback([this](PickingEvent* p, size_t rowId) {
            if (!p) return;
            if (auto dataframe = dataFrame_.getData()) {
                p->setToolTip(dataframe::createToolTipForRow(*dataFrame_.getData(), rowId));
            }
        });
    selectionChangedCallBack_ = persistenceDiagramPlot_.addSelectionChangedCallback(
        [this](const BitSet& indices) { brushingPort_.select(indices); });

    addProperty(persistenceDiagramPlot_.properties_);
    addProperty(birth_);
    addProperty(death_);
    addProperty(persistence_);
    addProperty(colorCol_);

    birth_.onChange([this]() { onXAxisChange(); });
    death_.onChange([this]() { onYAxisChange(); });
    persistence_.onChange([this]() { onYAxisChange(); });
    colorCol_.onChange([this]() { onColorChange(); });

    dataFrame_.onChange([this]() {
        onXAxisChange();
        onYAxisChange();
        onColorChange();

        if (dataFrame_.hasData()) {
            persistenceDiagramPlot_.setIndexColumn(dataFrame_.getData()->getIndexColumn());
        }
    });
}

void PersistenceDiagramPlotProcessor::process() {
    if (brushingPort_.isConnected()) {
        if (brushingPort_.isChanged()) {
            persistenceDiagramPlot_.setSelectedIndices(brushingPort_.getSelectedIndices());
        }

        auto dataframe = dataFrame_.getData();
        auto dfSize = dataframe->getNumberOfRows();

        auto iCol = dataframe->getIndexColumn();
        auto& indexCol = iCol->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

        auto filteredIndicies = brushingPort_.getFilteredIndices();
        IndexBuffer indicies;
        auto& vec = indicies.getEditableRAMRepresentation()->getDataContainer();
        vec.reserve(dfSize - filteredIndicies.size());

        auto seq = util::sequence<uint32_t>(0, static_cast<uint32_t>(dfSize), 1);
        std::copy_if(seq.begin(), seq.end(), std::back_inserter(vec),
                     [&](const auto& id) { return !brushingPort_.isFiltered(indexCol[id]); });

        if (backgroundPort_.hasData()) {
            persistenceDiagramPlot_.plot(*outport_.getEditableData(), *backgroundPort_.getData(),
                                         &indicies, true);
        } else {
            persistenceDiagramPlot_.plot(outport_, &indicies, true);
        }

    } else {
        if (backgroundPort_.hasData()) {
            persistenceDiagramPlot_.plot(*outport_.getEditableData(), *backgroundPort_.getData(),
                                         nullptr, true);
        } else {
            persistenceDiagramPlot_.plot(outport_, nullptr, true);
        }
    }
}

void PersistenceDiagramPlotProcessor::onXAxisChange() {
    if (!dataFrame_.hasData()) return;
    auto data = dataFrame_.getData();
    auto idx = birth_.get();
    persistenceDiagramPlot_.setXAxis(data->getColumn(idx));
}

void PersistenceDiagramPlotProcessor::onYAxisChange() {
    if (!dataFrame_.hasData()) return;
    auto data = dataFrame_.getData();

    if (death_.size() > 0 && death_.get() > 0) {
        persistenceDiagramPlot_.setYAxis(data->getColumn(death_.get()),
                                         PersistenceDiagramPlotGL::Type::Death);
    } else if (persistence_.size() > 0 && persistence_.get() > 0) {
        persistenceDiagramPlot_.setYAxis(data->getColumn(persistence_.get()),
                                         PersistenceDiagramPlotGL::Type::Persistence);
    } else {
        persistenceDiagramPlot_.setYAxis(nullptr, PersistenceDiagramPlotGL::Type::Death);
    }
}

void PersistenceDiagramPlotProcessor::onColorChange() {
    if (!dataFrame_.hasData()) return;
    auto data = dataFrame_.getData();
    auto idx = colorCol_.get();
    if (idx == -1) {
        persistenceDiagramPlot_.setColorData(nullptr);
    } else {
        auto buffer = data->getColumn(idx)->getBuffer();
        persistenceDiagramPlot_.setColorData(buffer);
    }
}

}  // namespace plot

}  // namespace inviwo
