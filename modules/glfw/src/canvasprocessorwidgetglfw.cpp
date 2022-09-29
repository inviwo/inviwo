/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
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

#include <inviwo/core/interaction/events/resizeevent.h>    // for ResizeEvent
#include <inviwo/core/network/networklock.h>               // for NetworkLock
#include <inviwo/core/processors/canvasprocessorwidget.h>  // for CanvasProcessorWidget
#include <inviwo/core/processors/processor.h>              // for Processor
#include <inviwo/core/util/glmvec.h>                       // for ivec2, uvec2, size2_t
#include <inviwo/core/util/rendercontext.h>                // for RenderContext
#include <modules/glfw/canvasglfw.h>                       // for CanvasGLFW
#include <modules/glfw/canvasprocessorwidgetglfw.h>        // for CanvasProcessorWidgetGLFW

#include <functional>                                      // for __base, function
#include <memory>                                          // for unique_ptr

#include <glm/vec2.hpp>                                    // for vec<>::(anonymous)

namespace inviwo {
class Canvas;

CanvasProcessorWidgetGLFW::CanvasProcessorWidgetGLFW(Processor* p)
    : CanvasProcessorWidget(p), canvas_{new CanvasGLFW(p->getIdentifier(), getDimensions())} {
    canvas_->setEventPropagator(p);
    canvas_->setWindowSize(CanvasProcessorWidget::getDimensions());
    canvas_->setWindowPosition(CanvasProcessorWidget::getPosition());
    canvas_->setVisible(CanvasProcessorWidget::isVisible());
    canvas_->setOnTop(CanvasProcessorWidget::isOnTop());
    canvas_->setFullScreen(CanvasProcessorWidget::isFullScreen());

    canvas_->onPositionChange = [this](ivec2 pos) { CanvasProcessorWidget::setPosition(pos); };
    canvas_->onFramebufferSizeChange = [this](ivec2) { propagateResizeEvent(); };
}

CanvasProcessorWidgetGLFW::~CanvasProcessorWidgetGLFW() { updateVisible(false); }

void CanvasProcessorWidgetGLFW::setVisible(bool visible) {
    updateVisible(visible);
    CanvasProcessorWidget::setVisible(visible);
}

void CanvasProcessorWidgetGLFW::setDimensions(ivec2 dim) {
    updateDimensions(dim);
    CanvasProcessorWidget::setDimensions(dim);
}

void CanvasProcessorWidgetGLFW::setPosition(ivec2 pos) {
    updatePosition(pos);
    CanvasProcessorWidget::setPosition(pos);
}

void CanvasProcessorWidgetGLFW::setFullScreen(bool fullScreen) {
    updateFullScreen(fullScreen);
    CanvasProcessorWidget::setFullScreen(fullScreen);
}
void CanvasProcessorWidgetGLFW::setOnTop(bool onTop) {
    updateOnTop(onTop);
    CanvasProcessorWidget::setOnTop(onTop);
}

void CanvasProcessorWidgetGLFW::propagateResizeEvent() {
    auto previousScreenDimensions = screenDimensions_;
    screenDimensions_ = canvas_->getFramebufferSize();
    CanvasProcessorWidget::setDimensions(screenDimensions_);

    NetworkLock lock;
    RenderContext::getPtr()->activateDefaultRenderContext();
    ResizeEvent resizeEvent(screenDimensions_, previousScreenDimensions);
    getProcessor()->propagateEvent(&resizeEvent, nullptr);
}

Canvas* CanvasProcessorWidgetGLFW::getCanvas() const { return canvas_.get(); }

void CanvasProcessorWidgetGLFW::updateVisible(bool visible) { canvas_->setVisible(visible); }
void CanvasProcessorWidgetGLFW::updateDimensions(ivec2 dim) {
    canvas_->setWindowSize(uvec2(dim.x, dim.y));
}
void CanvasProcessorWidgetGLFW::updatePosition(ivec2 pos) {
    canvas_->setWindowPosition(uvec2(pos.x, pos.y));
}
void CanvasProcessorWidgetGLFW::updateFullScreen(bool fullScreen) {
    canvas_->setFullScreen(fullScreen);
};
void CanvasProcessorWidgetGLFW::updateOnTop(bool onTop) { canvas_->setOnTop(onTop); };

}  // namespace inviwo
