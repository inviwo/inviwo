/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/processors/processorwidgetqt.h>
#include <apps/inviwo/inviwomainwindow.h>
#include <QResizeEvent>
#include <QMoveEvent>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>

namespace inviwo {

ProcessorWidgetQt::ProcessorWidgetQt() : QWidget(nullptr), ProcessorWidget() {}

ProcessorWidgetQt::~ProcessorWidgetQt() {}

void ProcessorWidgetQt::initialize() {
    ProcessorWidget::initialize();
    ivec2 dim = ProcessorWidget::getDimensions();
    ivec2 pos = ProcessorWidget::getPosition();
    
    QWidget::resize(dim.x, dim.y);

    InviwoApplicationQt* app = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr());
    if (app) {

       QPoint newPos = app->movePointOntoDesktop(QPoint(pos.x,pos.y), this->size());

        if (!(newPos.x() == 0 && newPos.y() == 0)) {
            QWidget::move(newPos);
        } else { // We guess that this is a new widget and give a new position
            newPos = app->getMainWindow()->pos();
            newPos += app->offsetWidget();
            QWidget::move(newPos);
        }
    }
}

void ProcessorWidgetQt::deinitialize() {
    ProcessorWidget::deinitialize();
}

void ProcessorWidgetQt::setVisible(bool visible) {
    // The subsequent events will call ProcessorWidget.
    QWidget::setVisible(visible);
}

void ProcessorWidgetQt::show() {
    ProcessorWidgetQt::setVisible(true);
}

void ProcessorWidgetQt::hide() {
    ProcessorWidgetQt::setVisible(false);
}

void ProcessorWidgetQt::setPosition(glm::ivec2 pos) {
    //ProcessorWidget::setPosition(pos); Will be called by the Move event.
    QWidget::move(pos.x, pos.y);
}
    
void ProcessorWidgetQt::move(ivec2 pos) {
    ProcessorWidgetQt::setPosition(pos);
}

void ProcessorWidgetQt::setDimensions(ivec2 dimensions) {
    // ProcessorWidget::setDimensions(dimensions);  Will be called by the Resize event.
    QWidget::resize(dimensions.x, dimensions.y);
}

void ProcessorWidgetQt::resizeEvent(QResizeEvent* event) {
    ProcessorWidget::setDimensions(ivec2(event->size().width(), event->size().height()));
    QWidget::resizeEvent(event);
}

void ProcessorWidgetQt::showEvent(QShowEvent* event) {
    ProcessorWidget::setVisible(true);
    QWidget::showEvent(event);
}

void ProcessorWidgetQt::closeEvent(QCloseEvent* event) {
    ProcessorWidget::setVisible(false);
    QWidget::closeEvent(event);
}

void ProcessorWidgetQt::hideEvent(QHideEvent* event) {
    ProcessorWidget::setVisible(false);
    QWidget::hideEvent(event);
}
    
void ProcessorWidgetQt::moveEvent(QMoveEvent* event) {
    ProcessorWidget::setPosition(ivec2(event->pos().x(), event->pos().y()));
    QWidget::moveEvent(event);
}

} // namespace
