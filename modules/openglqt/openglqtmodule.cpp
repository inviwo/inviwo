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

#include <modules/openglqt/openglqtmodule.h>
#include <modules/openglqt/openglqtcapabilities.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <modules/opengl/canvasprocessorgl.h>
#include <modules/openglqt/canvasprocessorwidgetqt.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMainWindow>
#include <QMenuBar>
#include <warn/pop>

namespace inviwo {

OpenGLQtModule::OpenGLQtModule(InviwoApplication* app) : InviwoModule(app, "OpenGLQt") {
    // Create GL Context
    CanvasQt::defineDefaultContextFormat();
    sharedCanvas_ = util::make_unique<CanvasQt>();

    sharedCanvas_->defaultGLState();
    RenderContext::getPtr()->setDefaultRenderContext(sharedCanvas_.get());

    registerProcessorWidget<CanvasProcessorWidgetQt, CanvasProcessorGL>();
    registerCapabilities(util::make_unique<OpenGLQtCapabilities>());

    if (auto qtApp = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())) {
        if (auto win = qtApp->getMainWindow()) {
            auto menu = util::make_unique<OpenGLQtMenu>(win);
            win->menuBar()->addMenu(menu.release());
        }
    }
}

OpenGLQtModule::~OpenGLQtModule() {
    if (sharedCanvas_.get() == RenderContext::getPtr()->getDefaultRenderContext()) {
        RenderContext::getPtr()->setDefaultRenderContext(nullptr);
    }
}

}  // namespace
