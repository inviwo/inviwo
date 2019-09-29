/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/dataframeqt/processors/dataframeview.h>

#include <inviwo/dataframeqt/dataframeviewprocessorwidget.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameView::processorInfo_{
    "org.inviwo.DataFrameView",  // Class identifier
    "DataFrame View",            // Display name
    "Data Output",               // Category
    CodeState::Stable,           // Code state
    "CPU, DataFrame",            // Tags
};
const ProcessorInfo DataFrameView::getProcessorInfo() const { return processorInfo_; }

DataFrameView::DataFrameView()
    : Processor()
    , inport_("inport")
    , brushLinkPort_("brushingAndLinking")

    , dimensions_("dimensions", "Canvas Size", size2_t(512, 300), size2_t(1, 1),
                  size2_t(10000, 10000), size2_t(1, 1), InvalidationLevel::Valid)
    , position_("position", "Canvas Position", ivec2(128, 128),
                ivec2(std::numeric_limits<int>::lowest()), ivec2(std::numeric_limits<int>::max()),
                ivec2(1, 1), InvalidationLevel::Valid, PropertySemantics::Text)
    , showIndexColumn_("showIndexColumn", "Show Index Column", false, InvalidationLevel::Valid)
    , vectorCompAsColumn_("vectorCompAsColumn", "Vector Components as Columns", true)
    , widgetMetaData_{
          createMetaData<ProcessorWidgetMetaData>(ProcessorWidgetMetaData::CLASS_IDENTIFIER)} {
    widgetMetaData_->addObserver(this);

    addPort(inport_);
    addPort(brushLinkPort_);
    addProperties(dimensions_, position_, showIndexColumn_, vectorCompAsColumn_);

    dimensions_.setSerializationMode(PropertySerializationMode::None);
    dimensions_.onChange([this]() { widgetMetaData_->setDimensions(dimensions_.get()); });
    position_.onChange([this]() { widgetMetaData_->setPosition(position_.get()); });

    showIndexColumn_.onChange([this]() {
        if (auto w = getWidget()) {
            w->setIndexColumnVisible(showIndexColumn_);
        }
    });
}

DataFrameView::~DataFrameView() {
    if (processorWidget_) {
        processorWidget_->hide();
    }
}

void DataFrameView::process() {
    if (auto w = getWidget()) {
        if (inport_.isChanged() || vectorCompAsColumn_.isModified()) {
            w->setDataFrame(inport_.getData(), vectorCompAsColumn_);
        }
        if (brushLinkPort_.isChanged()) {
            w->updateSelection();
        }
    }
}

void DataFrameView::selectColumns(const std::unordered_set<size_t>& columns) {
    brushLinkPort_.sendColumnSelectionEvent(columns);
}

const std::unordered_set<size_t>& DataFrameView::getSelectedColumns() const {
    return brushLinkPort_.getSelectedColumns();
}

void DataFrameView::selectRows(const std::unordered_set<size_t>& rows) {
    brushLinkPort_.sendSelectionEvent(rows);
}

const std::unordered_set<size_t>& DataFrameView::getSelectedRows() const {
    return brushLinkPort_.getSelectedIndices();
}

void DataFrameView::setProcessorWidget(std::unique_ptr<ProcessorWidget> processorWidget) {
    if (processorWidget && !dynamic_cast<DataFrameViewProcessorWidget*>(processorWidget.get())) {
        throw Exception(
            "Expected DataFrameViewProcessorWidget in DataFrameView::setProcessorWidget");
    }
    Processor::setProcessorWidget(std::move(processorWidget));
    isSink_.update();
    isReady_.update();
}

void DataFrameView::onProcessorWidgetPositionChange(ProcessorWidgetMetaData*) {
    if (widgetMetaData_->getPosition() != position_.get()) {
        Property::OnChangeBlocker blocker{position_};
        position_.set(widgetMetaData_->getPosition());
    }
}

void DataFrameView::onProcessorWidgetDimensionChange(ProcessorWidgetMetaData*) {
    if (widgetMetaData_->getDimensions() != dimensions_.get()) {
        Property::OnChangeBlocker blocker{dimensions_};
        dimensions_.set(widgetMetaData_->getDimensions());
    }
}

void DataFrameView::onProcessorWidgetVisibilityChange(ProcessorWidgetMetaData*) {
    isSink_.update();
    isReady_.update();
    invalidate(InvalidationLevel::InvalidOutput);
}

DataFrameViewProcessorWidget* DataFrameView::getWidget() const {
    if (auto widget = static_cast<DataFrameViewProcessorWidget*>(processorWidget_.get())) {
        return widget;
    } else {
        return nullptr;
    }
}

void DataFrameView::setWidgetSize(size2_t dim) {
    NetworkLock lock(this);
    dimensions_.set(dim);
}

size2_t DataFrameView::getWidgetSize() const { return dimensions_; }

}  // namespace inviwo
