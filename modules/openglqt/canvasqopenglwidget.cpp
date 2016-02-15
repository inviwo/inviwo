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

#include <modules/openglqt/canvasqopenglwidget.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <modules/opengl/openglcapabilities.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QOpenGLContext>
#include <QResizeEvent>
#include <warn/pop>

namespace inviwo {

inline QSurfaceFormat GetQGLFormat() {
    QSurfaceFormat sharedFormat = QSurfaceFormat();
    sharedFormat.setProfile(QSurfaceFormat::CoreProfile);
    sharedFormat.setVersion(10, 0);
    return sharedFormat;
}

QSurfaceFormat CanvasQOpenGLWidget::sharedFormat_ = GetQGLFormat();
CanvasQOpenGLWidget* CanvasQOpenGLWidget::sharedCanvas_ = nullptr;

CanvasQOpenGLWidget::CanvasQOpenGLWidget(QWidget* parent, uvec2 dim)
    : QOpenGLWidget(parent), CanvasGL(dim), swapBuffersAllowed_(false) {
    setFormat(sharedFormat_);

    if (sharedCanvas_) {
        this->context()->setShareContext(sharedCanvas_->context());
    }
    create();

    setFocusPolicy(Qt::StrongFocus);

    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);

    QResizeEvent event(QSize(dim.x, dim.y), QSize(width(), height()));
    QOpenGLWidget::resizeEvent(&event);
    if (!sharedCanvas_) {
        sharedFormat_ = format();
        sharedCanvas_ = this;
    }
}

CanvasQOpenGLWidget::~CanvasQOpenGLWidget() {
    if (sharedCanvas_ == this) sharedCanvas_ = nullptr;
}

void CanvasQOpenGLWidget::defineDefaultContextFormat() {
    if (!sharedCanvas_) {
        std::string preferProfile = OpenGLCapabilities::getPreferredProfile();
        if (preferProfile == "core")
            sharedFormat_.setProfile(QSurfaceFormat::CoreProfile);
        else if (preferProfile == "compatibility")
            sharedFormat_.setProfile(QSurfaceFormat::CompatibilityProfile);
    }
}

void CanvasQOpenGLWidget::activate() { makeCurrent(); }

void CanvasQOpenGLWidget::initializeGL() {
    OpenGLCapabilities::initializeGLEW();
    QOpenGLWidget::initializeGL();
    activate();
}

void CanvasQOpenGLWidget::glSwapBuffers() {
    if (swapBuffersAllowed_) {
        activate();
        context()->swapBuffers(context()->surface());
    }
}

void CanvasQOpenGLWidget::update() { CanvasGL::update(); }

void CanvasQOpenGLWidget::repaint() {}

void CanvasQOpenGLWidget::paintGL() {
    swapBuffersAllowed_ = true;
    CanvasGL::update();
}

void CanvasQOpenGLWidget::resize(uvec2 size) {
    QOpenGLWidget::resize(size.x, size.y);
    CanvasGL::resize(size);
}

void CanvasQOpenGLWidget::resizeEvent(QResizeEvent* event) {
    if (event->spontaneous()) {
        QOpenGLWidget::resizeEvent(event);
        return;
    }
    CanvasGL::resize(uvec2(event->size().width(), event->size().height()));
    QOpenGLWidget::resizeEvent(event);
}

}  // namespace