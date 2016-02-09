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

#include <modules/openglqt/canvasqwindow.h>

#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <modules/opengl/openglcapabilities.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QtGui/QOpenGLContext>
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

QSurfaceFormat CanvasQWindow::sharedFormat_ = GetQGLFormat();
CanvasQWindow* CanvasQWindow::sharedCanvas_ = nullptr;
QOpenGLContext* CanvasQWindow::sharedGLContext_ = nullptr;

CanvasQWindow::CanvasQWindow(QWindow* parent, uvec2 dim)
    : QWindow(parent), CanvasGL(dim), thisGLContext_(nullptr), swapBuffersAllowed_(false) {
    setSurfaceType(QWindow::OpenGLSurface);
    setFormat(sharedFormat_);
    create();

    thisGLContext_ = new QOpenGLContext(this);
    thisGLContext_->setFormat(sharedFormat_);

    // This is our default rendering context
    // Initialized once. So "THE" first object of this class will 
    // not have any shared context (or widget)
    // But Following objects, will share the context of initial object
    bool contextCreated;
    if (!sharedGLContext_) {
        contextCreated = thisGLContext_->create();
        sharedFormat_ = thisGLContext_->format();
        sharedGLContext_ = thisGLContext_;
        sharedCanvas_ = this;
        activate();
        initializeGL();
    } else {
        thisGLContext_->setShareContext(sharedGLContext_);
        contextCreated = thisGLContext_->create();
    }

    if (!contextCreated) {
        std::cout << "OpenGL context was not created successfully!" << std::endl;
        int major = sharedFormat_.majorVersion();
        int minor = sharedFormat_.minorVersion();
        std::cout << "GL Version: " << major << "." << minor << std::endl;
        std::cout << "GL Profile: " << (sharedFormat_.profile() == QSurfaceFormat::CoreProfile
                                            ? "Core"
                                            : "CompatibilityProfile")
                  << std::endl;
        const GLubyte* vendor = glGetString(GL_VENDOR);
        std::string vendorStr =
            std::string((vendor != nullptr ? reinterpret_cast<const char*>(vendor) : "INVALID"));
        std::cout << "GL Vendor: " << vendorStr << std::endl;
    }
}

void CanvasQWindow::defineDefaultContextFormat() {
    if (!sharedGLContext_) {
        std::string preferProfile = OpenGLCapabilities::getPreferredProfile();
        if (preferProfile == "core")
            sharedFormat_.setProfile(QSurfaceFormat::CoreProfile);
        else if (preferProfile == "compatibility")
            sharedFormat_.setProfile(QSurfaceFormat::CompatibilityProfile);
    }
}

void CanvasQWindow::activate() { thisGLContext_->makeCurrent(this); }

void CanvasQWindow::initializeGL() { OpenGLCapabilities::initializeGLEW(); }

void CanvasQWindow::glSwapBuffers() {
    if (swapBuffersAllowed_) {
        activate();
        thisGLContext_->swapBuffers(this);
    }
}

void CanvasQWindow::update() { CanvasGL::update(); }

void CanvasQWindow::repaint() {}

void CanvasQWindow::paintGL() {
    if (!isExposed()) return;
    swapBuffersAllowed_ = true;
    CanvasGL::update();
}

void CanvasQWindow::exposeEvent(QExposeEvent*) {
    if (isExposed()) paintGL();
}

CanvasQWindow* CanvasQWindow::getSharedCanvas() {
    return sharedCanvas_;
}

void CanvasQWindow::resize(uvec2 size) {
    QWindow::resize(size.x, size.y);
    CanvasGL::resize(size);
}

std::unique_ptr<Canvas> CanvasQWindow::create() {
    auto thread = QThread::currentThread();
    auto res = dispatchFront([&thread]() {
        // auto canvas = util::make_unique<HiddenCanvasQWindow>();
        //#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
        // canvas->context()->moveToThread(thread);
        //#endif
        // return canvas;
        return nullptr;
    });
    return res.get();
}

void CanvasQWindow::resizeEvent(QResizeEvent* event) {
    if (event->spontaneous()) {
        QWindow::resizeEvent(event);
        return;
    }
    CanvasGL::resize(uvec2(event->size().width(), event->size().height()));
    QWindow::resizeEvent(event);
}

#include <warn/push>
#include <warn/ignore/switch-enum>

bool CanvasQWindow::event(QEvent* e) {
    switch (e->type()) {
        case QEvent::KeyPress: {
            auto keyEvent = static_cast<QKeyEvent*>(e);
            auto parent = this->parent();
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
                    QWindow::keyPressEvent(static_cast<QKeyEvent*>(e));
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
                    QWindow::keyReleaseEvent(keyEvent);
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
        case QEvent::UpdateRequest:
            paintGL();
            return true;

        default:
            return QWindow::event(e);
    }
}

#include <warn/pop>

}  // namespace