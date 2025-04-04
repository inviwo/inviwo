/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/dataframeqt/dataframetableview.h>           // for DataFrameTableView
#include <modules/qtwidgets/inviwoqtutils.h>                 // for getApplicationMainWindow
#include <modules/qtwidgets/processors/processorwidgetqt.h>  // for ProcessorWidgetQt

#include <warn/push>
#include <warn/ignore/all>
#include <QLayout>      // for QLayout
#include <QMainWindow>  // for QMainWindow
#include <QVBoxLayout>  // for QVBoxLayout
#include <QWidget>      // for QWidget
#include <Qt>           // for WA_OpaquePaintEvent

#include <warn/pop>

namespace inviwo {

DataFrameTableProcessorWidget::DataFrameTableProcessorWidget(Processor* p)
    : ProcessorWidgetQt(p), tableview_(nullptr) {

    setMinimumSize(32, 32);
    QObject::setProperty("bgType", "window");

    tableview_ = new DataFrameTableView(this);
    tableview_->setMouseTracking(true);
    tableview_->setAttribute(Qt::WA_OpaquePaintEvent);

    setFocusProxy(tableview_);
    setCentralWidget(tableview_);
}

void DataFrameTableProcessorWidget::setVisible(bool visible) {
    if (QWidget::isVisible() != visible) {
        QWidget::setVisible(visible);  // This will trigger show/hide events.
    }
}

void DataFrameTableProcessorWidget::setManager(BrushingAndLinkingManager& manager) {
    tableview_->setManager(manager);
}

void DataFrameTableProcessorWidget::setDataFrame(std::shared_ptr<const DataFrame> dataframe,
                                                 bool categoryIndices) {
    tableview_->setDataFrame(dataframe, categoryIndices);
}

void DataFrameTableProcessorWidget::setIndexColumnVisible(bool visible) {
    tableview_->setIndexColumnVisible(visible);
}

void DataFrameTableProcessorWidget::setFilteredRowsVisible(bool visible) {
    tableview_->setFilteredRowsVisible(visible);
}

void DataFrameTableProcessorWidget::brushingUpdate() { tableview_->brushingUpdate(); }

}  // namespace inviwo
