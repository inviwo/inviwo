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
    : InviwoDockWidget(title, parent), ProcessorWidget(p) {
    this->setObjectName("ProcessorDockWidgetQt");
    setDimensions(ivec2(200, 150));

    ivec2 dim = ProcessorWidget::getDimensions();
    ivec2 pos = ProcessorWidget::getPosition();

    setWindowTitle(utilqt::toQString(p->getDisplayName()));
    // make the widget to float and not sticky by default
    this->setFloating(true);
    this->setSticky(false);

    setDimensions(dim);

    if (auto mainWindow = utilqt::getApplicationMainWindow()) {

        // set default docking area to the right side
        mainWindow->addDockWidget(Qt::RightDockWidgetArea, this);
        // Move widget relative to main window to make sure that it is visible on screen.
        QPoint newPos =
            utilqt::movePointOntoDesktop(QPoint(pos.x, pos.y), QSize(dim.x, dim.y), true);

        if (!(newPos.x() == 0 && newPos.y() == 0)) {
            InviwoDockWidget::move(newPos);
        } else {  // We guess that this is a new widget and give a new position
            newPos = mainWindow->pos();
            newPos += utilqt::offsetWidget();
            InviwoDockWidget::move(newPos);
        }
    }

    idChange_ = p->onDisplayNameChange([this](std::string_view newName, std::string_view) {
        setWindowTitle(utilqt::toQString(newName));
    });
}

void ProcessorDockWidgetQt::setPosition(glm::ivec2 pos) {
    InviwoDockWidget::move(pos.x, pos.y);  // This will trigger a move event.
}

void ProcessorDockWidgetQt::setDimensions(ivec2 dimensions) {
    InviwoDockWidget::resize(dimensions.x, dimensions.y);  // This will trigger a resize event.
}

void ProcessorDockWidgetQt::resizeEvent(QResizeEvent* event) {
    ivec2 dim(event->size().width(), event->size().height());
    if (event->spontaneous()) {
        ProcessorDockWidgetQt::setDimensions(dim);
        return;
    }
    ProcessorDockWidgetQt::setDimensions(dim);
    InviwoDockWidget::resizeEvent(event);
}

void ProcessorDockWidgetQt::moveEvent(QMoveEvent* event) {
    ProcessorDockWidgetQt::setPosition(ivec2(event->pos().x(), event->pos().y()));
    InviwoDockWidget::moveEvent(event);
}

void ProcessorDockWidgetQt::showEvent(QShowEvent* event) {
    ProcessorWidget::setVisible(true);
    InviwoDockWidget::showEvent(event);
}

void ProcessorDockWidgetQt::hideEvent(QHideEvent* event) {
    ProcessorWidget::setVisible(false);
    InviwoDockWidget::hideEvent(event);
}

}  // namespace inviwo
