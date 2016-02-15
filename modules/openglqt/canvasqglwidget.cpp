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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/openglqt/canvasqglwidget.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QOpenGLContext>
#include <QResizeEvent>
#include <warn/pop>

namespace inviwo {

inline QGLFormat GetQGLFormat() {
    QGLFormat sharedFormat = QGLFormat(QGL::Rgba | QGL::DoubleBuffer | QGL::AlphaChannel |
                                       QGL::DepthBuffer | QGL::StencilBuffer);
    sharedFormat.setProfile(QGLFormat::CoreProfile);
    sharedFormat.setVersion(10, 0);
    return sharedFormat;
}

QGLFormat CanvasQGLWidget::sharedFormat_ = GetQGLFormat();
CanvasQGLWidget* CanvasQGLWidget::sharedCanvas_ = nullptr;

CanvasQGLWidget::CanvasQGLWidget(QGLWidget* parent, uvec2 dim)
    : QGLWidget(sharedFormat_, parent, sharedCanvas_)
    , CanvasGL(dim)
    , swapBuffersAllowed_(false) {
    // This is our default rendering context
    // Initialized once. So "THE" first object of this class will
    // not have any shared context (or widget)
    // But Following objects, will share the context of initial object
    if (!sharedCanvas_) {
        sharedFormat_ = this->format();
        sharedCanvas_ = this;
        QGLWidget::glInit();
    }

    setAutoBufferSwap(false);
    setFocusPolicy(Qt::StrongFocus);

    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
}

CanvasQGLWidget::~CanvasQGLWidget() {
    if (sharedCanvas_ == this) sharedCanvas_ = nullptr;
}

void CanvasQGLWidget::defineDefaultContextFormat() {
    if (!sharedCanvas_) {
        std::string preferProfile = OpenGLCapabilities::getPreferredProfile();
        if (preferProfile == "core")
            sharedFormat_.setProfile(QGLFormat::CoreProfile);
        else if (preferProfile == "compatibility")
            sharedFormat_.setProfile(QGLFormat::CompatibilityProfile);
    }
}

void CanvasQGLWidget::activate() { context()->makeCurrent(); }

void CanvasQGLWidget::initializeGL() {
    OpenGLCapabilities::initializeGLEW();
    QGLWidget::initializeGL();
    activate();
}

void CanvasQGLWidget::glSwapBuffers() {
    if (swapBuffersAllowed_) {
        activate();
        QGLWidget::swapBuffers();
    }
}

void CanvasQGLWidget::update() { CanvasGL::update(); }

void CanvasQGLWidget::repaint() { QGLWidget::updateGL(); }

void CanvasQGLWidget::paintGL() {
    swapBuffersAllowed_ = true;
    CanvasGL::update();
}

void CanvasQGLWidget::resize(uvec2 size) {
    QGLWidget::resize(size.x, size.y);
    CanvasGL::resize(size);
}

void CanvasQGLWidget::resizeEvent(QResizeEvent* event) {
    if (event->spontaneous()) {
        QGLWidget::resizeEvent(event);
        return;
    }
    CanvasGL::resize(uvec2(event->size().width(), event->size().height()));
    QGLWidget::resizeEvent(event);
}

}  // namespace
