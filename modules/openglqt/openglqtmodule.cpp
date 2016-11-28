/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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
#include <modules/opengl/sharedopenglresources.h>
#include <modules/openglqt/canvasprocessorwidgetqt.h>
#include <inviwo/core/util/rendercontext.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/network/processornetworkevaluator.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMainWindow>
#include <QMenuBar>
#include <warn/pop>

namespace inviwo {

OpenGLQtModule::OpenGLQtModule(InviwoApplication* app)
    : InviwoModule(app, "OpenGLQt"), ProcessorNetworkEvaluationObserver() {
    // Create GL Context
    CanvasQt::defineDefaultContextFormat();
    sharedCanvas_ = util::make_unique<CanvasQt>(size2_t(16,16), "Default");

    sharedCanvas_->defaultGLState();
    RenderContext::getPtr()->setDefaultRenderContext(sharedCanvas_.get());

    registerProcessorWidget<CanvasProcessorWidgetQt, CanvasProcessorGL>();
    registerCapabilities(util::make_unique<OpenGLQtCapabilities>());

    if (auto mainWindow = utilqt::getApplicationMainWindow()) {
        auto menu = util::make_unique<OpenGLQtMenu>(mainWindow);
        mainWindow->menuBar()->addMenu(menu.release());
    }

    app->getProcessorNetworkEvaluator()->addObserver(this);
}

OpenGLQtModule::~OpenGLQtModule() {
    SharedOpenGLResources::getPtr()->reset();
    if (sharedCanvas_.get() == RenderContext::getPtr()->getDefaultRenderContext()) {
        RenderContext::getPtr()->setDefaultRenderContext(nullptr);
    }
}

void OpenGLQtModule::onProcessorNetworkEvaluationBegin() {
    // This is called before the network is evaluated, here we make sure that the default context is
    // active

    RenderContext::getPtr()->activateDefaultRenderContext();
}
void OpenGLQtModule::onProcessorNetworkEvaluationEnd() {
    // This is called after the network is evaluated, here we make sure that the gpu is done with
    // its work before we continue. This is needed to make sure that we have textures that are upto
    // data when we render the canvases.
    auto syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    auto res = glClientWaitSync(syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 100000000);
    
    switch (res) {
        case GL_ALREADY_SIGNALED:
            LogWarn("GL_ALREADY_SIGNALED");
            break;
        case GL_TIMEOUT_EXPIRED:
            LogWarn("GL_TIMEOUT_EXPIRED");
            break;
        case GL_WAIT_FAILED:
            LogWarn("GL_WAIT_FAILED");
            break;
        case GL_CONDITION_SATISFIED:
            break;
    }
    
    glDeleteSync(syncObj);

    LGL_ERROR;
}



}  // namespace
