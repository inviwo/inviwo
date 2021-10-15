/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#include <inviwo/dataframeqt/dataframetableprocessorwidget.h>
#include <inviwo/dataframeqt/dataframetableview.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/processors/processor.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <warn/pop>

namespace inviwo {

DataFrameTableProcessorWidget::DataFrameTableProcessorWidget(Processor* p)
    : ProcessorWidgetQt(p), tableview_(nullptr) {

    this->setParent(utilqt::getApplicationMainWindow());
    setMinimumSize(32, 32);
    QObject::setProperty("bgType", "window");

    tableview_ = tableview_ptr(new DataFrameTableView(this), [&](DataFrameTableView* c) {
        layout()->removeWidget(c);
        delete c;
    });

    tableview_->setMouseTracking(true);
    tableview_->setAttribute(Qt::WA_OpaquePaintEvent);

    QObject::connect(tableview_.get(), &DataFrameTableView::columnSelectionChanged, this,
                     [this](const BitSet& columns) { columnSelectionChanged_.invoke(columns); });
    QObject::connect(tableview_.get(), &DataFrameTableView::rowSelectionChanged, this,
                     [this](const BitSet& rows) { rowSelectionChanged_.invoke(rows); });
    QObject::connect(tableview_.get(), &DataFrameTableView::rowHighlightChanged, this,
                     [this](const BitSet& rows) { rowHighlightChanged_.invoke(rows); });

    setFocusProxy(tableview_.get());

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(tableview_.get());
}

void DataFrameTableProcessorWidget::setDataFrame(std::shared_ptr<const DataFrame> dataframe,
                                                 bool vectorsIntoColumns, bool categoryIndices) {
    tableview_->setDataFrame(dataframe, vectorsIntoColumns, categoryIndices);
}

void DataFrameTableProcessorWidget::setIndexColumnVisible(bool visible) {
    tableview_->setIndexColumnVisible(visible);
}

void DataFrameTableProcessorWidget::updateSelection(const BitSet& rows) {
    tableview_->selectRows(rows);
}

void DataFrameTableProcessorWidget::updateColumnSelection(const BitSet& columns) {
    tableview_->selectColumns(columns);
}

void DataFrameTableProcessorWidget::updateHighlight(const BitSet& highlightedRows) {
    tableview_->highlightRows(highlightedRows);
}

auto DataFrameTableProcessorWidget::setColumnSelectionChangedCallback(
    std::function<SelectionChangedFunc> callback) -> CallbackHandle {
    return columnSelectionChanged_.add(callback);
}

auto DataFrameTableProcessorWidget::setRowSelectionChangedCallback(
    std::function<SelectionChangedFunc> callback) -> CallbackHandle {
    return rowSelectionChanged_.add(callback);
}

auto DataFrameTableProcessorWidget::setRowHighlightChangedCallback(
    std::function<SelectionChangedFunc> callback) -> CallbackHandle {
    return rowHighlightChanged_.add(callback);
}

}  // namespace inviwo
