/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/processors/processordockwidgetqt.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <inviwo/qt/widgets/propertylistwidget.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/processors/processor.h>

#include <QMoveEvent>
#include <QMainWindow>


namespace inviwo {

ProcessorDockWidgetQt::ProcessorDockWidgetQt(const QString &title, QWidget *parent)
    : InviwoDockWidget(title, parent)
{
    this->setObjectName("ProcessorDockWidgetQt");
    setDimensions(ivec2(200, 150));
    //setPosition(ivec2(300, 500));    
}

ProcessorDockWidgetQt* ProcessorDockWidgetQt::create() const {
    auto widget = new ProcessorDockWidgetQt;
    InviwoApplicationQt* app = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr());
    if (app) {
        widget->setParent(app->getMainWindow());
    }
    return widget;
}

void ProcessorDockWidgetQt::initialize() {
    ProcessorWidget::initialize();

    ivec2 dim = ProcessorWidget::getDimensions();
    ivec2 pos = ProcessorWidget::getPosition();
    
    setWindowTitle(QString::fromStdString(processor_->getIdentifier()));
    // make the widget to float and not sticky by default
    this->setFloating(true);
    this->setSticky(false);

    setDimensions(dim);

    InviwoApplicationQt* app = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr());
    if (app) {
        auto mainWindow = static_cast<InviwoMainWindow *>(app->getMainWindow());
        // set default docking area to the right side
        mainWindow->addDockWidget(Qt::RightDockWidgetArea, this);

        QPoint newPos = app->movePointOntoDesktop(QPoint(pos.x, pos.y), QSize(dim.x, dim.y), true);

        if (!(newPos.x() == 0 && newPos.y() == 0)) {
            InviwoDockWidget::move(newPos);
        }
        else { // We guess that this is a new widget and give a new position
            newPos = app->getMainWindow()->pos();
            newPos += app->offsetWidget();
            InviwoDockWidget::move(newPos);
        }
    }
    
}

void ProcessorDockWidgetQt::setVisible(bool visible) {
    InviwoDockWidget::setVisible(visible); // This will trigger show/hide events.
    ProcessorWidget::setVisible(visible);
}

void ProcessorDockWidgetQt::show() {
    ProcessorDockWidgetQt::setVisible(true);
}

void ProcessorDockWidgetQt::hide() {
    ProcessorDockWidgetQt::setVisible(false);
}

void ProcessorDockWidgetQt::setPosition(glm::ivec2 pos) {
    InviwoDockWidget::move(pos.x, pos.y); // This will trigger a move event.
}

void ProcessorDockWidgetQt::setDimensions(ivec2 dimensions) {
    InviwoDockWidget::resize(dimensions.x, dimensions.y); // This will trigger a resize event.
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

void ProcessorDockWidgetQt::setProcessor(Processor* processor) {
    ProcessorWidget::setProcessor(processor);
    if (processor) processor->ProcessorObservable::addObserver(this);
}

void ProcessorDockWidgetQt::onProcessorIdentifierChange(Processor*) {
    setWindowTitle(QString::fromStdString(processor_->getIdentifier()));
}

/*
void ProcessorDockWidgetQt::closeEvent(QCloseEvent* event) {
    //canvas_->hide();
    ProcessorDockWidgetQt::setVisible(false);
    InviwoDockWidget::closeEvent(event);
}

void ProcessorDockWidgetQt::showEvent(QShowEvent* event) {
    //ProcessorDockWidgetQt::setVisible(true);
    InviwoDockWidget::showEvent(event);
}

void ProcessorDockWidgetQt::hideEvent(QHideEvent* event) {
    //ProcessorDockWidgetQt::setVisible(false);
    InviwoDockWidget::hideEvent(event);
}
*/

} // namespace

