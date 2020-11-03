/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2026 Inviwo Foundation
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

// Ensure we only include Qt OpenGL stuff, no Glew OpenGL
#include <QApplication>       // for QApplication
#include <QCoreApplication>   // for QCoreApplication
#include <QOffscreenSurface>  // for QOffscreenSurface
#include <QOpenGLContext>     // for QOpenGLContext
#include <QThread>            // for QThread

#include <inviwo/core/common/inviwoapplication.h>  // for dispatchFront
#include <inviwo/core/util/canvas.h>               // for Canvas, Canvas::ContextID
#include <inviwo/core/util/rendercontext.h>        // for CanvasContextHolder, RenderContext
#include <inviwo/core/util/stringconversion.h>     // for toString
#include <inviwo/core/util/threadutil.h>
#include <modules/opengl/openglcapabilities.h>  // for OpenGLCapabilities

#include <atomic>      // for atomic, __atomic_base
#include <functional>  // for __base
#include <future>      // for future
#include <string>      // for char_traits, operator+, basic_string
#include <thread>      // for get_id

#include <fmt/std.h>
#include <inviwo/tracy/tracyopengl.h>

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

void HiddenCanvasQt::createContext() { context_->create(); }
void HiddenCanvasQt::initializeGLEW() { OpenGLCapabilities::initializeGLEW(); }

void HiddenCanvasQt::update() {}
void HiddenCanvasQt::activate() { context_->makeCurrent(offScreenSurface_); }

std::unique_ptr<Canvas> HiddenCanvasQt::createHiddenCanvas() { return createHiddenQtCanvas(); }

std::unique_ptr<Canvas> HiddenCanvasQt::createHiddenQtCanvas() {
    static std::atomic<int> hiddenContextCount = 0;
    const auto contextNumber = hiddenContextCount++;
    const auto name = fmt::format("Background Context {}", contextNumber);

    auto* thread = QThread::currentThread();
    // The context has to be created on the main thread.
    auto res = dispatchFront([thread, name]() {
        auto canvas = std::make_unique<HiddenCanvasQt>(name);

        // Need to create the OpenGL context here on the main thread
        // This can deadlock if we try to initialize it on the background thread
        // Then we can move it to the background thread
        canvas->context_->create();
        canvas->getContext()->moveToThread(thread);
        return canvas;
    });

    // Wait for the context to be created
    auto newContext = res.get();

    // Activate the context to initialize GLEW in this thread
    newContext->activate();
    newContext->initializeGLEW();

    // Since the qt context has to be created on the main thread and moved to the background we need
    // to update the registered thread id to the correct one here
    RenderContext::getPtr()->setContextThreadId(newContext->contextId(),
                                                std::this_thread::get_id());

    TRACY_GPU_CONTEXT;
    TRACY_GPU_CONTEXT_NAME(name.data(), name.size());

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
