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

#include <inviwo/dataframeqt/dataframeviewprocessorwidget.h>
#include <inviwo/dataframeqt/processors/dataframeview.h>
#include <inviwo/dataframeqt/dataframetableview.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/raiiutils.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <warn/pop>

namespace inviwo {

DataFrameViewProcessorWidget::DataFrameViewProcessorWidget(Processor* p)
    : QWidget(utilqt::getApplicationMainWindow()), ProcessorWidget(p), tableview_(nullptr) {

    setMinimumSize(32, 32);
    QObject::setProperty("bgType", "window");
    setWindowFlags(Qt::Window);

    ivec2 dim = getDimensions();
    ivec2 pos = getPosition();

    setWindowTitle(QString::fromStdString(processor_->getDisplayName()));

    tableview_ = tableview_ptr(new DataFrameTableView(this), [&](DataFrameTableView* c) {
        layout()->removeWidget(c);
        delete c;
    });

    tableview_->setMouseTracking(true);
    tableview_->setAttribute(Qt::WA_OpaquePaintEvent);

    setFocusProxy(tableview_.get());

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(tableview_.get());

    setDimensions(dim);

    if (auto mainWindow = utilqt::getApplicationMainWindow()) {
        // Move widget relative to main window to make sure that it is visible on screen.
        QPoint newPos =
            utilqt::movePointOntoDesktop(QPoint(pos.x, pos.y), QSize(dim.x, dim.y), true);

        if (!(newPos.x() == 0 && newPos.y() == 0)) {
            util::KeepTrueWhileInScope ignore(&ignoreEvents_);
            // prevent move events, since this will automatically save the "adjusted" position.
            // The processor widget already has its correct pos, i.e. the one de-serialized from
            // file.
            QWidget::move(newPos);
        } else {  // We guess that this is a new widget and give a new position
            newPos = mainWindow->pos();
            newPos += utilqt::offsetWidget();
            QWidget::move(newPos);
        }
    }

    {
        // Trigger both resize event and move event by showing and hiding the widget
        // in order to set the correct, i.e. the de-serialized, size and position.
        //
        // Otherwise, a spontaneous event will be triggered which will set the widget
        // to its "initial" size of 160 by 160 at (0, 0) thereby overwriting our values.
        util::KeepTrueWhileInScope ignore(&ignoreEvents_);
        QWidget::setVisible(true);
        QWidget::resize(QSize(dim.x, dim.y));
        QWidget::setVisible(false);
    }

    processor_->ProcessorObservable::addObserver(this);
    {
        // ignore internal state updates, i.e. position, when showing the widget
        // On Windows, the widget hasn't got a decoration yet. So it will be positioned using the
        // decoration offset, i.e. the "adjusted" position.
        util::KeepTrueWhileInScope ignore(&ignoreEvents_);
        QWidget::setVisible(ProcessorWidget::isVisible());
    }
}

void DataFrameViewProcessorWidget::setVisible(bool visible) {
    if (visible) {
        tableview_->show();
    } else {
        tableview_->hide();
    }
    QWidget::setVisible(visible);  // This will trigger show/hide events.
}

void DataFrameViewProcessorWidget::show() { setVisible(true); }

void DataFrameViewProcessorWidget::hide() { setVisible(false); }

void DataFrameViewProcessorWidget::setPosition(ivec2 pos) {
    if (pos != utilqt::toGLM(QWidget::pos())) {
        QWidget::move(pos.x, pos.y);  // This will trigger a move event.
    }
}

void DataFrameViewProcessorWidget::setDimensions(ivec2 dimensions) {
    if (dimensions != utilqt::toGLM(QWidget::size())) {
        QWidget::resize(dimensions.x, dimensions.y);  // This will trigger a resize event.
    }
}

void DataFrameViewProcessorWidget::setDataFrame(std::shared_ptr<const DataFrame> dataframe,
                                                bool vectorsIntoColumns) {
    tableview_->setDataFrame(dataframe, vectorsIntoColumns);
}

void DataFrameViewProcessorWidget::setIndexColumnVisible(bool visible) {
    tableview_->setIndexColumnVisible(visible);
}

DataFrameTableView* DataFrameViewProcessorWidget::getTableView() const { return tableview_.get(); }

void DataFrameViewProcessorWidget::resizeEvent(QResizeEvent* event) {
    if (ignoreEvents_) return;
    util::KeepTrueWhileInScope ignore(&ignoreUpdate_);

    setUpdatesEnabled(false);
    util::OnScopeExit enable([&]() { setUpdatesEnabled(true); });

    ProcessorWidget::setDimensions(utilqt::toGLM(event->size()));

    if (!event->spontaneous()) {
        QWidget::resizeEvent(event);
    }
}

void DataFrameViewProcessorWidget::closeEvent(QCloseEvent* event) {
    if (ignoreEvents_) return;
    util::KeepTrueWhileInScope ignore(&ignoreUpdate_);

    ProcessorWidget::setVisible(false);
    QWidget::closeEvent(event);
}

void DataFrameViewProcessorWidget::showEvent(QShowEvent* event) {
    if (ignoreEvents_) return;
    util::KeepTrueWhileInScope ignore(&ignoreUpdate_);

    ProcessorWidget::setVisible(true);
    QWidget::setVisible(true);
    QWidget::showEvent(event);
}

void DataFrameViewProcessorWidget::hideEvent(QHideEvent* event) {
    if (ignoreEvents_) return;
    util::KeepTrueWhileInScope ignore(&ignoreUpdate_);

    ProcessorWidget::setVisible(false);
    QWidget::hideEvent(event);
}

void DataFrameViewProcessorWidget::moveEvent(QMoveEvent* event) {
    if (ignoreEvents_) return;
    util::KeepTrueWhileInScope ignore(&ignoreUpdate_);

    ProcessorWidget::setPosition(utilqt::toGLM(event->pos()));
    QWidget::moveEvent(event);
}

void DataFrameViewProcessorWidget::onProcessorDisplayNameChanged(Processor*, const std::string&) {
    setWindowTitle(QString::fromStdString(processor_->getDisplayName()));
}

void DataFrameViewProcessorWidget::updateVisible(bool visible) {
    if (ignoreUpdate_) return;
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    setVisible(visible);
}
void DataFrameViewProcessorWidget::updateDimensions(ivec2 dim) {
    if (ignoreUpdate_) return;
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    setDimensions(dim);
}
void DataFrameViewProcessorWidget::updatePosition(ivec2 pos) {
    if (ignoreUpdate_) return;
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    setPosition(pos);
}

}  // namespace inviwo
