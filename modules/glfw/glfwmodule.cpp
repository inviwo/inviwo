/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <modules/glfw/glfwmodule.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/util/rendercontext.h>
#include <modules/opengl/canvasprocessorgl.h>
#include <modules/glfw/canvasprocessorwidgetglfw.h>

namespace inviwo {

GLFWModule::GLFWModule(InviwoApplication* app) : InviwoModule(app, "GLFW") {
    if (!glfwInit()) {
        LogError("GLFW could not be initialized.");
    }

    GLFWSharedCanvas_ = new CanvasGLFW(InviwoApplication::getPtr()->getDisplayName());
    GLFWSharedCanvas_->initializeGL();

    RenderContext::getPtr()->setDefaultRenderContext(GLFWSharedCanvas_);
    GLFWSharedCanvas_->initializeSquare();
    GLFWSharedCanvas_->defaultGLState();

    registerProcessorWidgetAndAssociate<CanvasProcessorGL>(util::make_unique<CanvasProcessorWidgetGLFW>());
}

GLFWModule::~GLFWModule() {
    if (GLFWSharedCanvas_ == RenderContext::getPtr()->getDefaultRenderContext()) {
        RenderContext::getPtr()->setDefaultRenderContext(nullptr);
    }
    GLFWSharedCanvas_->deinitialize();
    delete GLFWSharedCanvas_;
}

} // namespace
