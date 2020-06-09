/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2020 Inviwo Foundation
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

#include <modules/openglqt/hiddencanvasqt.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/rendercontext.h>
#include <modules/opengl/openglcapabilities.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QThread>
#include <QApplication>
#include <warn/pop>

namespace inviwo {

HiddenCanvasQt::HiddenCanvasQt(QSurfaceFormat format) : Canvas(size2_t{0}) {
    context_.setFormat(format);
    offScreenSurface_.setFormat(format);
    offScreenSurface_.create();
    context_.setShareContext(QOpenGLContext::globalShareContext());
}

void HiddenCanvasQt::initializeGL() {
    context_.create();
    activate();
    OpenGLCapabilities::initializeGLEW();
}

void HiddenCanvasQt::update() {}
void HiddenCanvasQt::activate() { context_.makeCurrent(&offScreenSurface_); }

std::unique_ptr<Canvas> HiddenCanvasQt::createHiddenCanvas() { return createHiddenQtCanvas(); }

std::unique_ptr<Canvas> HiddenCanvasQt::createHiddenQtCanvas() {
    auto thread = QThread::currentThread();
    // The context has to be created on the main thread.
    auto res = dispatchFront([&thread]() {
        auto canvas = std::make_unique<HiddenCanvasQt>();
        canvas->getContext()->moveToThread(thread);
        return canvas;
    });

    auto newContext = res.get();
    // OpenGL can be initialized in this thread
    newContext->initializeGL();
    RenderContext::getPtr()->setContextThreadId(newContext->contextId(),
                                                std::this_thread::get_id());
    return std::move(newContext);
}

Canvas::ContextID HiddenCanvasQt::activeContext() const {
    return static_cast<ContextID>(QOpenGLContext::currentContext());
}
Canvas::ContextID HiddenCanvasQt::contextId() const { return static_cast<ContextID>(&context_); }

void HiddenCanvasQt::releaseContext() {
    context_.doneCurrent();
    context_.moveToThread(QApplication::instance()->thread());
}

}  // namespace inviwo
