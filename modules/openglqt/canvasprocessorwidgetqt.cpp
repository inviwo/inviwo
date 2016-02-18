/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <modules/openglqt/canvasprocessorwidgetqt.h>
#include <modules/openglqt/canvasqt.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <QGridLayout>

namespace inviwo {

CanvasProcessorWidgetQt::CanvasProcessorWidgetQt(Processor* p)
    : QWidget(dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())->getMainWindow())
    , CanvasProcessorWidget(p)
    , canvas_(nullptr) {
    
    setMinimumSize(32, 32);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);

    ivec2 dim = CanvasProcessorWidget::getDimensions();
    ivec2 pos = CanvasProcessorWidget::getPosition();

    setWindowTitle(QString::fromStdString(processor_->getIdentifier()));

    canvas_ = canvas_ptr(new CanvasQt(uvec2(dim.x, dim.y)), [&](CanvasQt* c){
        layout()->removeWidget(c);
        c->activate();
        delete c;
        RenderContext::getPtr()->activateDefaultRenderContext();
    });

    canvas_->setEventPropagator(processor_);
    canvas_->setProcessorWidgetOwner(this);
    canvas_->setMouseTracking(true);
    canvas_->setAttribute(Qt::WA_OpaquePaintEvent);

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->addWidget(canvas_.get(), 0, 0);
    setLayout(gridLayout);

    setWindowFlags(Qt::Tool);
    setDimensions(dim);

    if (auto app = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())) {
        QPoint newPos = app->movePointOntoDesktop(QPoint(pos.x, pos.y), QSize(dim.x, dim.y), true);

        if (!(newPos.x() == 0 && newPos.y() == 0)) {
            QWidget::move(newPos);
        } else {  // We guess that this is a new widget and give a new position
            newPos = app->getMainWindow()->pos();
            newPos += app->offsetWidget();
            QWidget::move(newPos);
        }
    }
    processor_->ProcessorObservable::addObserver(this);
    canvas_->setVisible(ProcessorWidget::isVisible());
    QWidget::setVisible(ProcessorWidget::isVisible());
}

CanvasProcessorWidgetQt::~CanvasProcessorWidgetQt() {
    this->hide();
}

void CanvasProcessorWidgetQt::setVisible(bool visible) {
    if(visible){
        canvas_->show();
        static_cast<CanvasProcessor*>(processor_)->triggerQueuedEvaluation();
    } else {
        canvas_->hide();
    }
    QWidget::setVisible(visible); // This will trigger show/hide events.
}
    
void CanvasProcessorWidgetQt::show() {
    CanvasProcessorWidgetQt::setVisible(true);
}
void CanvasProcessorWidgetQt::hide() {
    CanvasProcessorWidgetQt::setVisible(false);
}

void CanvasProcessorWidgetQt::setPosition(glm::ivec2 pos) {
    QWidget::move(pos.x, pos.y); // This will trigger a move event.
}

void CanvasProcessorWidgetQt::setDimensions(ivec2 dimensions) {
    QWidget::resize(dimensions.x, dimensions.y); // This will trigger a resize event.
}

Canvas* CanvasProcessorWidgetQt::getCanvas() const {
    return canvas_.get();
}

void CanvasProcessorWidgetQt::resizeEvent(QResizeEvent* event) {
    setUpdatesEnabled(false);
    util::OnScopeExit enable([&](){setUpdatesEnabled(true);});

    ivec2 dim(event->size().width(), event->size().height());
    if (event->spontaneous()) {
        CanvasProcessorWidget::setDimensions(dim);
    } else {
        CanvasProcessorWidget::setDimensions(dim);
        QWidget::resizeEvent(event);
    }
}

void CanvasProcessorWidgetQt::closeEvent(QCloseEvent* event) {
    canvas_->hide();
    CanvasProcessorWidget::setVisible(false);
    QWidget::closeEvent(event);
}

void CanvasProcessorWidgetQt::showEvent(QShowEvent* event) {
    CanvasProcessorWidget::setVisible(true);
    QWidget::showEvent(event);
}

void CanvasProcessorWidgetQt::hideEvent(QHideEvent* event) {
    CanvasProcessorWidget::setVisible(false);
    QWidget::hideEvent(event);
}

void CanvasProcessorWidgetQt::moveEvent(QMoveEvent* event) {
    CanvasProcessorWidget::setPosition(ivec2(event->pos().x(), event->pos().y()));
    QWidget::moveEvent(event);
}

void CanvasProcessorWidgetQt::onProcessorIdentifierChange(Processor*) {
    setWindowTitle(QString::fromStdString(processor_->getIdentifier()));
}



}  // namespace
