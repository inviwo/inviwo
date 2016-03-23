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
#include <QResizeEvent>
#include <QExposeEvent>
#include <QtGui/QOpenGLContext>
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

QOpenGLContext* CanvasQWindow::context() const { return thisGLContext_; }
QWindow* CanvasQWindow::parentWidget() const { return parent(); };

void CanvasQWindow::releaseContext() {
    delete thisGLContext_;
    thisGLContext_ = nullptr;
    sharedGLContext_ = nullptr;
    sharedCanvas_ = nullptr;
}

void CanvasQWindow::exposeEvent(QExposeEvent*) {
    if (isExposed()) paintGL();
}

void CanvasQWindow::resize(uvec2 size) {
    QWindow::resize(size.x, size.y);
    CanvasGL::resize(size);
}

void CanvasQWindow::resizeEvent(QResizeEvent* event) {
    if (event->spontaneous()) {
        QWindow::resizeEvent(event);
        return;
    }
    CanvasGL::resize(uvec2(event->size().width(), event->size().height()));
    QWindow::resizeEvent(event);
}

}  // namespace