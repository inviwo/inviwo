/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2021 Inviwo Foundation
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
#include <inviwo/core/util/stringconversion.h>
#include <modules/opengl/openglcapabilities.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QThread>
#include <warn/pop>

#include <atomic>

namespace inviwo {

HiddenCanvasQt::HiddenCanvasQt(std::string_view name, QSurfaceFormat format)
    : Canvas(), context_{new QOpenGLContext()}, offScreenSurface_{new QOffscreenSurface()} {
    context_->setFormat(format);
    offScreenSurface_->setFormat(format);
    offScreenSurface_->create();
    context_->setShareContext(QOpenGLContext::globalShareContext());

    RenderContext::getPtr()->registerContext(contextId(), name,
                                             std::make_unique<CanvasContextHolder>(this));
}
HiddenCanvasQt::~HiddenCanvasQt() {
    RenderContext::getPtr()->unRegisterContext(contextId());

    delete context_;
    delete offScreenSurface_;
}

void HiddenCanvasQt::initializeGL() {
    context_->create();
    activate();
    OpenGLCapabilities::initializeGLEW();
}

void HiddenCanvasQt::update() {}
void HiddenCanvasQt::activate() { context_->makeCurrent(offScreenSurface_); }

std::unique_ptr<Canvas> HiddenCanvasQt::createHiddenCanvas() { return createHiddenQtCanvas(); }

std::unique_ptr<Canvas> HiddenCanvasQt::createHiddenQtCanvas() {
    static std::atomic<int> hiddenContextCount = 0;
    const auto contextNumber = hiddenContextCount++;
    const auto name = "Background Context " + toString(contextNumber);

    auto thread = QThread::currentThread();
    // The context has to be created on the main thread.
    auto res = dispatchFront([&thread, name]() {
        auto canvas = std::make_unique<HiddenCanvasQt>(name);
        canvas->getContext()->moveToThread(thread);
        return canvas;
    });

    auto newContext = res.get();
    // OpenGL can be initialized in this thread
    newContext->initializeGL();

    // Since the qt context has to be created on the main thread and moved to the background we need
    // to update the registered thread id to the correct one here
    RenderContext::getPtr()->setContextThreadId(newContext->contextId(),
                                                std::this_thread::get_id());

    return newContext;
}

Canvas::ContextID HiddenCanvasQt::activeContext() const {
    return static_cast<ContextID>(QOpenGLContext::currentContext());
}
Canvas::ContextID HiddenCanvasQt::contextId() const { return static_cast<ContextID>(context_); }

void HiddenCanvasQt::releaseContext() {
    context_->doneCurrent();
    context_->moveToThread(QApplication::instance()->thread());
}

}  // namespace inviwo
