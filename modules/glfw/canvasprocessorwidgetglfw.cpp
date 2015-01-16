/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <modules/glfw/canvasprocessorwidgetglfw.h>
#include <modules/glfw/canvasglfw.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

CanvasProcessorWidgetGLFW::CanvasProcessorWidgetGLFW()
    : CanvasProcessorWidget()
    , canvas_(NULL)
    , hasSharedCanvas_(false) {}

CanvasProcessorWidgetGLFW::~CanvasProcessorWidgetGLFW() {}

CanvasProcessorWidgetGLFW* CanvasProcessorWidgetGLFW::create() const {
    return new CanvasProcessorWidgetGLFW();
}

void CanvasProcessorWidgetGLFW::initialize() {
    CanvasProcessorWidget::initialize();

    ivec2 dim = getDimensions();
    uvec2 dimU = uvec2(dim.x, dim.y);

    CanvasGLFW::setAlwaysOnTopByDefault(true);
    CanvasGLFW* sharedCanvas = CanvasGLFW::getSharedContext();
    if (!sharedCanvas->getProcessorWidgetOwner()) {
        canvas_ = sharedCanvas;
        canvas_->setWindowTitle(processor_->getIdentifier());
        hasSharedCanvas_ = true;
    } else {
        canvas_ = new CanvasGLFW(processor_->getIdentifier(), dimU);
        canvas_->initializeGL();
    }

    if(!canvas_->isInitialized())
        canvas_->initialize();

    canvas_->setProcessorWidgetOwner(this);
    canvas_->setWindowSize(dimU);
}

void CanvasProcessorWidgetGLFW::deinitialize() {
    if (canvas_) {
        this->hide();
        if(hasSharedCanvas_)
            canvas_->setProcessorWidgetOwner(NULL);
        else
            delete canvas_;
        canvas_ = NULL;
    }

    ProcessorWidget::deinitialize();
}

void CanvasProcessorWidgetGLFW::setVisible(bool visible) {
    if (visible) {
        canvas_->show();
        static_cast<CanvasProcessor*>(processor_)->triggerQueuedEvaluation();
        CanvasProcessorWidget::setVisible(true);
    } else {
        canvas_->hide();
        CanvasProcessorWidget::setVisible(false);
    }
}

void CanvasProcessorWidgetGLFW::show() {
    CanvasProcessorWidgetGLFW::setVisible(true);
}

void CanvasProcessorWidgetGLFW::hide() {
    CanvasProcessorWidgetGLFW::setVisible(false);
}

void CanvasProcessorWidgetGLFW::setDimensions(ivec2 dim) {
    CanvasProcessorWidget::setDimensions(dim);
    canvas_->setWindowSize(uvec2(dim.x, dim.y));
}

Canvas* CanvasProcessorWidgetGLFW::getCanvas() const {
    return canvas_;
}



} // namespace
