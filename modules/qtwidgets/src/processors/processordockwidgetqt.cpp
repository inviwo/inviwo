/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/qtwidgets/processors/processordockwidgetqt.h>

#include <inviwo/core/processors/processor.h>        // for Processor, Processor::NameDispatcher...
#include <inviwo/core/processors/processorwidget.h>  // for ProcessorWidget
#include <inviwo/core/util/glmvec.h>                 // for ivec2
#include <modules/qtwidgets/inviwodockwidget.h>      // for InviwoDockWidget
#include <modules/qtwidgets/inviwoqtutils.h>         // for toQString, getApplicationMainWindow

#include <functional>   // for __base
#include <string_view>  // for string_view

#include <QMainWindow>              // for QMainWindow
#include <QMoveEvent>               // for QMoveEvent
#include <QPoint>                   // for QPoint
#include <QResizeEvent>             // for QResizeEvent
#include <QSize>                    // for QSize
#include <Qt>                       // for RightDockWidgetArea
#include <glm/ext/vector_int2.hpp>  // for ivec2
#include <glm/vec2.hpp>             // for vec<>::(anonymous)

class QHideEvent;
class QMoveEvent;
class QResizeEvent;
class QShowEvent;
class QWidget;

namespace inviwo {

ProcessorDockWidgetQt::ProcessorDockWidgetQt(Processor* p, const QString& title, QWidget* parent)
    : InviwoDockWidget(title, parent, "ProcessorDockWidgetQt")
    , ProcessorWidget(p)
    , idChange_{p->onDisplayNameChange([this](std::string_view newName, std::string_view) {
        setWindowTitle(utilqt::toQString(newName));
    })} {

    const ivec2 dim = ProcessorWidget::getDimensions();
    const ivec2 pos = ProcessorWidget::getPosition();

    setWindowTitle(utilqt::toQString(p->getDisplayName()));
    // make the widget to float and not sticky by default
    this->setFloating(true);
    this->setSticky(false);

    if (auto mainWindow = utilqt::getApplicationMainWindow()) {
        // Move widget relative to main window to make sure that it is visible on screen.
        QPoint newPos =
            utilqt::movePointOntoDesktop(utilqt::toQPoint(pos), utilqt::toQSize(dim), true);

        if (!(newPos.x() == 0 && newPos.y() == 0)) {
            const util::KeepTrueWhileInScope ignore(&ignoreEvents_);
            // prevent move events, since this will automatically save the "adjusted" position.
            // The processor widget already has its correct pos, i.e. the one de-serialized from
            // file.
            InviwoDockWidget::move(newPos);
        } else {  // We guess that this is a new widget and give a new position
            newPos = mainWindow->pos();
            newPos += utilqt::offsetWidget();
            InviwoDockWidget::move(newPos);
        }
    }

    {
        // Trigger both resize event and move event by showing and hiding the widget
        // in order to set the correct, i.e. the de-serialized, size and position.
        //
        // Otherwise, a spontaneous event will be triggered which will set the widget
        // to its "initial" size of 160 by 160 at (0, 0) thereby overwriting our values.
        const util::KeepTrueWhileInScope ignore(&ignoreEvents_);
        InviwoDockWidget::setVisible(true);
        InviwoDockWidget::resize(dim.x, dim.y);
        InviwoDockWidget::setVisible(false);
    }
    {
        // ignore internal state updates, i.e. position, when showing the widget
        // On Windows, the widget hasn't got a decoration yet. So it will be positioned using the
        // decoration offset, i.e. the "adjusted" position.
        const util::KeepTrueWhileInScope ignore(&ignoreEvents_);
        InviwoDockWidget::setVisible(ProcessorWidget::isVisible());
    }

    utilqt::setFullScreenAndOnTop(this, ProcessorWidget::isFullScreen(),
                                  ProcessorWidget::isOnTop());
}

void ProcessorDockWidgetQt::setPosition(glm::ivec2 pos) {
    if (pos != utilqt::toGLM(QWidget::pos())) {
        // This will trigger a move event.
        InviwoDockWidget::move(pos.x, pos.y);
    }
}

void ProcessorDockWidgetQt::setDimensions(ivec2 dimensions) {
    // This will trigger a resize event.
    InviwoDockWidget::resize(dimensions.x, dimensions.y);
}

void ProcessorDockWidgetQt::resizeEvent(QResizeEvent* event) {
    if (ignoreEvents_) return;

    InviwoDockWidget::resizeEvent(event);
    const util::KeepTrueWhileInScope ignore(&resizeOngoing_);
    ProcessorWidget::setDimensions(utilqt::toGLM(event->size()));
}

void ProcessorDockWidgetQt::moveEvent(QMoveEvent* event) {
    if (ignoreEvents_) return;
    ProcessorWidget::setPosition(utilqt::toGLM(event->pos()));
    InviwoDockWidget::moveEvent(event);
}

void ProcessorDockWidgetQt::showEvent(QShowEvent* event) {
    if (ignoreEvents_) return;
    ProcessorWidget::setVisible(true);
    InviwoDockWidget::showEvent(event);
}

void ProcessorDockWidgetQt::hideEvent(QHideEvent* event) {
    if (ignoreEvents_) return;
    ProcessorWidget::setVisible(false);
    InviwoDockWidget::hideEvent(event);
}

void ProcessorDockWidgetQt::updateVisible(bool visible) {
    const util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    InviwoDockWidget::setVisible(visible);
}
void ProcessorDockWidgetQt::updateDimensions(ivec2 dim) {
    if (resizeOngoing_) return;
    const util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    InviwoDockWidget::move(dim.x, dim.y);
}
void ProcessorDockWidgetQt::updatePosition(ivec2 pos) {
    const util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    InviwoDockWidget::move(pos.x, pos.y);
}

void ProcessorDockWidgetQt::updateFullScreen(bool fullScreen) {
    utilqt::setFullScreenAndOnTop(this, fullScreen, ProcessorWidget::isOnTop());
}

void ProcessorDockWidgetQt::updateOnTop(bool onTop) {
    utilqt::setFullScreenAndOnTop(this, ProcessorWidget::isFullScreen(), onTop);
}

}  // namespace inviwo
