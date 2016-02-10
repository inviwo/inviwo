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

#include <modules/openglqt/canvasqt.h>
#include <modules/openglqt/hiddencanvasqt.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <modules/opengl/openglcapabilities.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QGestureEvent>
#include <QThread>
#include <warn/pop>

namespace inviwo {

inline QGLFormat GetQGLFormat() {
    QGLFormat sharedFormat = QGLFormat(QGL::Rgba | QGL::DoubleBuffer | QGL::AlphaChannel |
                                       QGL::DepthBuffer | QGL::StencilBuffer);
    sharedFormat.setProfile(QGLFormat::CoreProfile);
    sharedFormat.setVersion(10, 0);
    return sharedFormat;
}

QGLFormat CanvasQt::sharedFormat_ = GetQGLFormat();
CanvasQt* CanvasQt::sharedCanvas_ = nullptr;
QGLWidget* CanvasQt::sharedGLContext_ = nullptr;

CanvasQt::CanvasQt(QGLWidget* parent, uvec2 dim)
    : QGLWidget(sharedFormat_, parent, sharedGLContext_)
    , CanvasGL(dim)
    , swapBuffersAllowed_(false) {
    // This is our default rendering context
    // Initialized once. So "THE" first object of this class will 
    // not have any shared context (or widget)
    // But Following objects, will share the context of initial object
    if (!sharedGLContext_) {
        sharedFormat_ = this->format();
        sharedGLContext_ = this;
        sharedCanvas_ = this;
        QGLWidget::glInit();
    }

    setAutoBufferSwap(false);
    setFocusPolicy(Qt::StrongFocus);

    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
}

void CanvasQt::defineDefaultContextFormat() {
    if (!sharedGLContext_) {
        std::string preferProfile = OpenGLCapabilities::getPreferredProfile();
        if (preferProfile == "core")
            sharedFormat_.setProfile(QGLFormat::CoreProfile);
        else if (preferProfile == "compatibility")
            sharedFormat_.setProfile(QGLFormat::CompatibilityProfile);
    }
}

void CanvasQt::activate() { context()->makeCurrent(); }

void CanvasQt::initializeGL() {
    OpenGLCapabilities::initializeGLEW();
    QGLWidget::initializeGL();
    activate();
}

void CanvasQt::glSwapBuffers() {
    if (swapBuffersAllowed_) {
        activate();
        QGLWidget::swapBuffers();
    }
}

void CanvasQt::update() { CanvasGL::update(); }

void CanvasQt::repaint() { QGLWidget::updateGL(); }

void CanvasQt::paintGL() {
    swapBuffersAllowed_ = true;
    CanvasGL::update();
}

CanvasQt* CanvasQt::getSharedCanvas() { return sharedCanvas_; }

void CanvasQt::resize(uvec2 size) {
    QGLWidget::resize(size.x, size.y);
    CanvasGL::resize(size);
}

std::unique_ptr<Canvas> CanvasQt::create() {
    auto thread = QThread::currentThread();
    auto res = dispatchFront([&thread]() {
        auto canvas = util::make_unique<HiddenCanvasQt>();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
        canvas->context()->moveToThread(thread);
#endif
        return canvas;
    });
    return res.get();
}

void CanvasQt::resizeEvent(QResizeEvent* event) {
    if (event->spontaneous()) {
        QGLWidget::resizeEvent(event);
        return;
    }
    CanvasGL::resize(uvec2(event->size().width(), event->size().height()));
    QGLWidget::resizeEvent(event);
}

#include <warn/push>
#include <warn/ignore/switch-enum>

bool CanvasQt::event(QEvent* e) {
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
                    QGLWidget::keyPressEvent(static_cast<QKeyEvent*>(e));
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
                    QGLWidget::keyReleaseEvent(keyEvent);
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
            touchFallback(static_cast<QTouchEvent*>(e));
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
            return QGLWidget::event(e);
    }
}

void CanvasQt::touchFallback(QTouchEvent* touch) {
// Mouse events will be triggered for touch events by Qt4 and Qt >= 5.3.0
// https://bugreports.qt.io/browse/QTBUG-40038
#if (QT_VERSION < QT_VERSION_CHECK(5, 3, 0))
    if (touch->touchPoints().size() == 1 && eventMapper_.getLastNumFingers() < 2) {
        size_t nTouchPoints = touch->touchPoints().size();
        if (nTouchPoints == 0) return;

        QTouchEvent::TouchPoint firstPoint = touch->touchPoints()[0];

        MouseEvent* mouseEvent = nullptr;
        const ivec2 pos = utilqt::toGLM(firstPoint.pos());
        const ivec2 screenPosInvY{util::invertY(pos, getScreenDimensions())};

        double depth = getDepthValueAtCoord(screenPosInvY);
        switch (touch->touchPoints().front().state()) {
            case TouchPoint::TOUCH_STATE_STARTED:
                mouseEvent = new MouseEvent(
                    pos, MouseEvent::MOUSE_BUTTON_LEFT, MouseEvent::MOUSE_STATE_PRESS,
                    EventConverterQt::getModifier(touch), getScreenDimensions(), depth);
                Canvas::mousePressEvent(mouseEvent);
                break;
            case TouchPoint::TOUCH_STATE_UPDATED:
                mouseEvent = new MouseEvent(
                    pos, MouseEvent::MOUSE_BUTTON_LEFT, MouseEvent::MOUSE_STATE_MOVE,
                    EventConverterQt::getModifier(touch), getScreenDimensions(), depth);
                Canvas::mouseMoveEvent(mouseEvent);
                break;
            case TouchPoint::TOUCH_STATE_STATIONARY:
                break;  // Do not fire event while standing still.
            case TouchPoint::TOUCH_STATE_ENDED:
                mouseEvent = new MouseEvent(
                    pos, MouseEvent::MOUSE_BUTTON_LEFT, MouseEvent::MOUSE_STATE_RELEASE,
                    EventConverterQt::getModifier(touch), getScreenDimensions(), depth);
                Canvas::mouseReleaseEvent(mouseEvent);
                break;
            default:
                break;
        }
        delete mouseEvent;
    }
#endif
}

#include <warn/pop>

}  // namespace
