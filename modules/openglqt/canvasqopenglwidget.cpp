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
#include <QThread>
#include <QInputEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QGestureEvent>
#include <QPanGesture>
#include <QPinchGesture>
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
QOpenGLWidget* CanvasQOpenGLWidget::sharedGLContext_ = nullptr;

CanvasQOpenGLWidget::CanvasQOpenGLWidget(QWidget* parent, uvec2 dim)
    : QOpenGLWidget(parent), CanvasGL(dim), swapBuffersAllowed_(false) {
    setFormat(sharedFormat_);

    if (sharedGLContext_) {
        this->context()->setShareContext(sharedGLContext_->context());
    }
    create();

    setFocusPolicy(Qt::StrongFocus);

    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);

    QResizeEvent event(QSize(dim.x, dim.y), QSize(width(), height()));
    QOpenGLWidget::resizeEvent(&event);
    if (!sharedGLContext_) {
        sharedFormat_ = format();
        sharedGLContext_ = this;
        sharedCanvas_ = this;
    }
}

void CanvasQOpenGLWidget::defineDefaultContextFormat() {
    if (!sharedGLContext_) {
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

CanvasQOpenGLWidget* CanvasQOpenGLWidget::getSharedCanvas() { return sharedCanvas_; }

void CanvasQOpenGLWidget::resize(uvec2 size) {
    QOpenGLWidget::resize(size.x, size.y);
    CanvasGL::resize(size);
}

std::unique_ptr<Canvas> CanvasQOpenGLWidget::create() {
    auto thread = QThread::currentThread();
    auto res = dispatchFront([&thread]() {
        // auto canvas = util::make_unique<HiddenCanvasQOpenGLWidget>();
        // canvas->context()->moveToThread(thread);
        // return canvas;
        return nullptr;
    });
    return res.get();
}

void CanvasQOpenGLWidget::resizeEvent(QResizeEvent* event) {
    if (event->spontaneous()) {
        QOpenGLWidget::resizeEvent(event);
        return;
    }
    CanvasGL::resize(uvec2(event->size().width(), event->size().height()));
    QOpenGLWidget::resizeEvent(event);
}

#include <warn/push>
#include <warn/ignore/switch-enum>

bool CanvasQOpenGLWidget::event(QEvent* e) {
    switch (e->type()) {
        case QEvent::KeyPress: {
            auto keyEvent = static_cast<QKeyEvent*>(e);
            QWidget* parent = this->parentWidget();
            if (parent && keyEvent->key() == Qt::Key_F &&
                keyEvent->modifiers() == Qt::ShiftModifier) {
                if (parent->windowState() == Qt::WindowFullScreen) {
                    parent->showNormal();
                } else {
                    parent->showFullScreen();
                }
            }

            return eventMapper_.mapKeyPressEvent(keyEvent, [&](KeyboardEvent* ke) {
                Canvas::keyPressEvent(ke);
                if (ke->hasBeenUsed()) {
                    e->accept();
                } else {
                    QOpenGLWidget::keyPressEvent(static_cast<QKeyEvent*>(e));
                }
                return true;
            });
        }
        case QEvent::KeyRelease: {
            auto keyEvent = static_cast<QKeyEvent*>(e);
            return eventMapper_.mapKeyPressEvent(keyEvent, [&](KeyboardEvent* ke) {
                Canvas::keyPressEvent(ke);
                if (ke->hasBeenUsed()) {
                    e->accept();
                } else {
                    QOpenGLWidget::keyReleaseEvent(keyEvent);
                }
                return true;
            });
        }
        case QEvent::MouseButtonPress:
            return eventMapper_.mapMousePressEvent(static_cast<QMouseEvent*>(e), this,
                                                   [&](MouseEvent* me) {
                                                       Canvas::mousePressEvent(me);
                                                       return true;
                                                   });
        case QEvent::MouseButtonRelease:
            return eventMapper_.mapMouseReleaseEvent(static_cast<QMouseEvent*>(e), this,
                                                     [&](MouseEvent* me) {
                                                         Canvas::mouseReleaseEvent(me);
                                                         return true;
                                                     });

        case QEvent::MouseMove:
            return eventMapper_.mapMouseMoveEvent(static_cast<QMouseEvent*>(e), this,
                                                  [&](MouseEvent* me) {
                                                      Canvas::mouseMoveEvent(me);
                                                      return true;
                                                  });

        case QEvent::Wheel:
            return eventMapper_.mapWheelEvent(static_cast<QWheelEvent*>(e), this,
                                              [&](MouseEvent* me) {
                                                  Canvas::mouseWheelEvent(me);
                                                  return true;
                                              });

        case QEvent::TouchBegin:
        case QEvent::TouchEnd:
        case QEvent::TouchUpdate: {
            return eventMapper_.mapTouchEvent(static_cast<QTouchEvent*>(e), this,
                                              [&](TouchEvent* te) {
                                                  Canvas::touchEvent(te);
                                                  return true;
                                              });
        }
        case QEvent::Gesture:
            return eventMapper_.mapGestureEvent(static_cast<QGestureEvent*>(e), this,
                                                [&](GestureEvent* ge) {
                                                    Canvas::gestureEvent(ge);
                                                    return true;
                                                });
        default:
            return QOpenGLWidget::event(e);
    }
}

#include <warn/pop>

}  // namespace