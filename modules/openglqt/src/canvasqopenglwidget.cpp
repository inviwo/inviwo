/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2020 Inviwo Foundation
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



#include <modules/openglqt/canvasqopenglwidget.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/util/rendercontext.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <QOpenGLContext>
#include <QResizeEvent>
#include <warn/pop>

namespace inviwo {

CanvasQOpenGLWidget::CanvasQOpenGLWidget(QWidget* parent, size2_t dim, std::string_view name)
    : QOpenGLWidget(parent), CanvasGL(dim), name_{name} {

    setFocusPolicy(Qt::StrongFocus);

    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
    installEventFilter(new utilqt::WidgetCloseEventFilter(this));
}

CanvasQOpenGLWidget::~CanvasQOpenGLWidget() {
    RenderContext::getPtr()->unRegisterContext(contextId());
}

void CanvasQOpenGLWidget::activate() { makeCurrent(); }

void CanvasQOpenGLWidget::initializeGL() {
    OpenGLCapabilities::initializeGLEW();
    // QOpenGLWidget docs:
    // There is no need to call makeCurrent() because this has already been done
    // when this function is called.
    // Note however that the framebuffer is not yet available at this stage,
    // so do not issue draw calls from here.
    // Defer such calls to paintGL() instead.

    QOpenGLWidget::initializeGL();

    RenderContext::getPtr()->registerContext(contextId(), name_,
                                             std::make_unique<CanvasContextHolder>(this));
    setupDebug();
}

void CanvasQOpenGLWidget::glSwapBuffers() {
    // Do nothing:
    // QOpenGLWidget will swap buffers after paintGL and we are calling this from CanvasGL::update()
    // QOpenGLWidget docs:
    // triggering a buffer swap just for the QOpenGLWidget is not possible since there is no real,
    // onscreen native surface for it.
    // Instead, it is up to the widget stack to manage composition and buffer swaps on the gui
    // thread. When a thread is done updating the framebuffer, call update() on the GUI/main thread
    // to schedule composition.
}

void CanvasQOpenGLWidget::update() {
    QOpenGLWidget::update();  // this will trigger a paint event.
}

void CanvasQOpenGLWidget::paintGL() { CanvasGL::update(); }

void CanvasQOpenGLWidget::resize(size2_t size) {
    // this should trigger a resize event.
    QOpenGLWidget::resize(static_cast<int>(size.x), static_cast<int>(size.y));
}

Canvas::ContextID CanvasQOpenGLWidget::activeContext() const {
    return static_cast<ContextID>(QOpenGLContext::currentContext());
}
Canvas::ContextID CanvasQOpenGLWidget::contextId() const {
    return static_cast<ContextID>(context());
}

void CanvasQOpenGLWidget::resizeEvent(QResizeEvent* event) {
    if (event->spontaneous()) {
        return;
    }
    setUpdatesEnabled(false);
    util::OnScopeExit enable([&]() { setUpdatesEnabled(true); });
    QOpenGLWidget::resizeEvent(event);
    // CanvasGL::resize may switch to default context so call this last
    CanvasGL::resize(uvec2(event->size().width(), event->size().height()));
}

void CanvasQOpenGLWidget::releaseContext() {
    doneCurrent();
    context()->moveToThread(QApplication::instance()->thread());
}

}  // namespace inviwo