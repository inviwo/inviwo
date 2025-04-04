/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/processors/processor.h>                // for Processor
#include <inviwo/core/processors/processorwidget.h>          // for ProcessorWidget
#include <inviwo/core/util/glmvec.h>                         // for ivec2
#include <inviwo/core/util/raiiutils.h>                      // for KeepTrueWhileInScope
#include <modules/qtwidgets/inviwoqtutils.h>                 // for setFullScreenAndOnTop, toGLM
#include <modules/qtwidgets/processors/processorwidgetqt.h>  // for ProcessorWidgetQt, Processor...

#include <functional>   // for __base
#include <string_view>  // for string_view

#include <QMainWindow>              // for QMainWindow
#include <QMoveEvent>               // for QMoveEvent
#include <QPoint>                   // for QPoint
#include <QResizeEvent>             // for QResizeEvent
#include <QWidget>                  // for QWidget
#include <Qt>                       // for Tool, Window, WindowFlags
#include <glm/ext/vector_int2.hpp>  // for ivec2
#include <glm/vec2.hpp>             // for vec<>::(anonymous), operator!=

class QCloseEvent;
class QHideEvent;
class QMoveEvent;
class QResizeEvent;
class QShowEvent;

namespace inviwo {

ProcessorWidgetQt::ProcessorWidgetQt(Processor* p)
    : ProcessorWidget(p)
    , QMainWindow(utilqt::getApplicationMainWindow(),
                  ProcessorWidget::isOnTop() ? Qt::Tool : Qt::Window)
    , nameChange_{p->onDisplayNameChange([this](std::string_view newName, std::string_view) {
        setWindowTitle(utilqt::toQString(newName));
    })} {

    setWindowTitle(utilqt::toQString(p->getDisplayName()));
    setDockNestingEnabled(true);

    const ivec2 dim = ProcessorWidget::getDimensions();
    const ivec2 pos = ProcessorWidget::getPosition();

    if (auto mainWindow = utilqt::getApplicationMainWindow()) {
        // Move widget relative to main window to make sure that it is visible on screen.
        QPoint newPos =
            utilqt::movePointOntoDesktop(utilqt::toQPoint(pos), utilqt::toQSize(dim), true);

        if (!(newPos.x() == 0 && newPos.y() == 0)) {
            util::KeepTrueWhileInScope ignore(&ignoreEvents_);
            // prevent move events, since this will automatically save the "adjusted" position.
            // The processor widget already has its correct pos, i.e. the one de-serialized from
            // file.
            Super::move(newPos);
        } else {  // We guess that this is a new widget and give a new position
            newPos = mainWindow->pos();
            newPos += utilqt::offsetWidget();
            Super::move(newPos);
        }
    }

    {
        // ignore internal state updates, i.e. position, when showing the widget
        // On Windows, the widget hasn't got a decoration yet. So it will be positioned using the
        // decoration offset, i.e. the "adjusted" position.
        const util::KeepTrueWhileInScope ignore(&ignoreEvents_);
        Super::resize(dim.x, dim.y);
        Super::setVisible(ProcessorWidget::isVisible());
    }

    utilqt::setFullScreenAndOnTop(this, ProcessorWidget::isFullScreen(),
                                  ProcessorWidget::isOnTop());
}

void ProcessorWidgetQt::setPosition(glm::ivec2 pos) {
    if (pos != utilqt::toGLM(Super::pos())) {
        Super::move(pos.x, pos.y);  // This will trigger a move event.
    }
}

void ProcessorWidgetQt::setDimensions(ivec2 dimensions) {
    Super::resize(dimensions.x, dimensions.y);  // This will trigger a resize event.
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

    Super::resizeEvent(event);
    util::KeepTrueWhileInScope ignore(&resizeOngoing_);
    ProcessorWidget::setDimensions(utilqt::toGLM(event->size()));
}

void ProcessorWidgetQt::showEvent(QShowEvent* event) {
    if (ignoreEvents_) return;
    ProcessorWidget::setVisible(true);
    Super::showEvent(event);
}

void ProcessorWidgetQt::hideEvent(QHideEvent* event) {
    if (ignoreEvents_) return;
    ProcessorWidget::setVisible(false);
    Super::hideEvent(event);
}

void ProcessorWidgetQt::moveEvent(QMoveEvent* event) {
    if (ignoreEvents_) return;
    ProcessorWidget::setPosition(utilqt::toGLM(event->pos()));
    Super::moveEvent(event);
}

void ProcessorWidgetQt::closeEvent(QCloseEvent* event) {
    if (ignoreEvents_) return;
    ProcessorWidget::setVisible(false);
    Super::closeEvent(event);
}

void ProcessorWidgetQt::updateVisible(bool visible) {
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    Super::setVisible(visible);
}
void ProcessorWidgetQt::updateDimensions(ivec2 dim) {
    if (resizeOngoing_) return;
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    Super::move(dim.x, dim.y);
}
void ProcessorWidgetQt::updatePosition(ivec2 pos) {
    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    Super::move(pos.x, pos.y);
}

void ProcessorWidgetQt::updateFullScreen(bool fullScreen) {
    utilqt::setFullScreenAndOnTop(this, fullScreen, ProcessorWidget::isOnTop());
}

void ProcessorWidgetQt::updateOnTop(bool onTop) {
    utilqt::setFullScreenAndOnTop(this, ProcessorWidget::isFullScreen(), onTop);
}

}  // namespace inviwo
