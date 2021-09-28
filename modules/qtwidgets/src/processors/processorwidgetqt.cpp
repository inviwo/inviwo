/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <inviwo/core/util/raiiutils.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/processors/processorwidgetqt.h>
#include <inviwo/core/processors/processorwidgetfactory.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QLayout>
#include <warn/pop>

namespace inviwo {

ProcessorWidgetQt::ProcessorWidgetQt(Processor* p)
    : ProcessorWidget(p)
    , QWidget(utilqt::getApplicationMainWindow(),
              ProcessorWidget::isOnTop() ? Qt::Tool : Qt::Window)
    , nameChange_{p->onDisplayNameChange([this](std::string_view newName, std::string_view) {
        setWindowTitle(utilqt::toQString(newName));
    })} {

    setWindowTitle(utilqt::toQString(p->getDisplayName()));

    const ivec2 pysicalDim = ProcessorWidget::getDimensions();
    const ivec2 pos = ProcessorWidget::getPosition();

    const auto dpr = window()->devicePixelRatio();
    const ivec2 logicalDim = pysicalDim / dpr;

    if (auto mainWindow = utilqt::getApplicationMainWindow()) {
        // Move widget relative to main window to make sure that it is visible on screen.
        QPoint newPos =
            utilqt::movePointOntoDesktop(utilqt::toQPoint(pos), utilqt::toQSize(logicalDim), true);

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
        QWidget::resize(QSize(logicalDim.x, logicalDim.y));
        QWidget::setVisible(false);
    }
    {
        // ignore internal state updates, i.e. position, when showing the widget
        // On Windows, the widget hasn't got a decoration yet. So it will be positioned using the
        // decoration offset, i.e. the "adjusted" position.
        util::KeepTrueWhileInScope ignore(&ignoreEvents_);
        QWidget::setVisible(ProcessorWidget::isVisible());
    }
}

void ProcessorWidgetQt::setPosition(glm::ivec2 pos) {
    if (pos != utilqt::toGLM(QWidget::pos())) {
        QWidget::move(pos.x, pos.y);  // This will trigger a move event.
    }
}

void ProcessorWidgetQt::setDimensions(ivec2 dimensions) {
    if (dimensions != utilqt::toGLM(QWidget::size())) {
        const auto dpr = window()->devicePixelRatio();
        const ivec2 logicalDim = dimensions / dpr;
        QWidget::resize(logicalDim.x, logicalDim.y);  // This will trigger a resize event.
    }
}

void ProcessorWidgetQt::setFullScreen(bool fullScreen) {
    utilqt::setFullScreenAndOnTop(this, fullScreen, ProcessorWidget::isOnTop());
    ProcessorWidget::setFullScreen(fullScreen);
}

void ProcessorWidgetQt::setOnTop(bool onTop) {
    utilqt::setFullScreenAndOnTop(this, ProcessorWidget::isFullScreen(), onTop);
    ProcessorWidget::setOnTop(onTop);
}

void ProcessorWidgetQt::resizeEvent(QResizeEvent* event) {
    if (ignoreEvents_) return;

    QWidget::resizeEvent(event);
    util::KeepTrueWhileInScope ignore(&resizeOngoing_);
    const auto dpr = window()->devicePixelRatio();
    const ivec2 dims = dpr * utilqt::toGLM(event->size());
    ProcessorWidget::setDimensions(dims);
}

void ProcessorWidgetQt::showEvent(QShowEvent* event) {
    if (ignoreEvents_) return;
    ProcessorWidget::setVisible(true);
    QWidget::showEvent(event);
}

void ProcessorWidgetQt::closeEvent(QCloseEvent* event) {
    if (ignoreEvents_) return;
    ProcessorWidget::setVisible(false);
    QWidget::closeEvent(event);
}

void ProcessorWidgetQt::hideEvent(QHideEvent* event) {
    if (ignoreEvents_) return;
    ProcessorWidget::setVisible(false);
    QWidget::hideEvent(event);
}

void ProcessorWidgetQt::moveEvent(QMoveEvent* event) {
    if (ignoreEvents_) return;
    ProcessorWidget::setPosition(utilqt::toGLM(event->pos()));
    QWidget::moveEvent(event);
}

void ProcessorWidgetQt::updateVisible(bool visible) {
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    QWidget::setVisible(visible);
}
void ProcessorWidgetQt::updateDimensions(ivec2 dim) {
    if (resizeOngoing_) return;
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    QWidget::move(dim.x, dim.y);
}
void ProcessorWidgetQt::updatePosition(ivec2 pos) {
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    QWidget::move(pos.x, pos.y);
}

void ProcessorWidgetQt::updateFullScreen(bool fullScreen) {
    utilqt::setFullScreenAndOnTop(this, fullScreen, ProcessorWidget::isOnTop());
}

void ProcessorWidgetQt::updateOnTop(bool onTop) {
    utilqt::setFullScreenAndOnTop(this, ProcessorWidget::isFullScreen(), onTop);
}

}  // namespace inviwo
