/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2020 Inviwo Foundation
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

#include <modules/glfw/canvasglfw.h>
#include <modules/glfw/glfwexception.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/glmvec.h>

#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglcapabilities.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <fmt/format.h>

namespace inviwo {

GLFWwindow* CanvasGLFW::sharedContext_ = nullptr;
int CanvasGLFW::glfwWindowCount_ = 0;
bool CanvasGLFW::alwaysOnTop_ = false;

GLFWwindow* CanvasGLFW::createWindow(const std::string& title, uvec2 dimensions) {
    glfwWindowHint(GLFW_FLOATING, alwaysOnTop_ ? GL_TRUE : GL_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

#ifdef __APPLE__
    if (!sharedContext_) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }
#endif

    auto win = glfwCreateWindow(static_cast<int>(dimensions.x), static_cast<int>(dimensions.y),
                                title.c_str(), nullptr, sharedContext_);

    if (!win) {
        throw GLFWException("Could not create GLFW window.", IVW_CONTEXT_CUSTOM("CanvasGLFW"));
    }

    return win;
}

CanvasGLFW::CanvasGLFW(const std::string& windowTitle, uvec2 dimensions)
    : CanvasGL(dimensions)
    , windowTitle_(windowTitle)
    , glWindow_(createWindow(windowTitle, dimensions))
    , userdata_{glWindow_}
    , eventManager_{glWindow_, [this](Event* e) { propagateEvent(e); },
                    [this](dvec2 pos) -> double { return getDepthValueAtNormalizedCoord(pos); }} {

    userdata_.set(GLFWUserDataId::Window, this);

    if (!sharedContext_) sharedContext_ = glWindow_;

    // register callbacks
    glfwSetWindowCloseCallback(glWindow_, closeWindow);
    glfwSetWindowSizeCallback(glWindow_, reshape);
    glfwSetWindowPosCallback(glWindow_, move);

    RenderContext::getPtr()->registerContext(contextId(), windowTitle,
                                             std::make_unique<DefaultContextHolder>(this));
}

CanvasGLFW::~CanvasGLFW() {
    RenderContext::getPtr()->unRegisterContext(contextId());
    glfwDestroyWindow(glWindow_);
    if (glWindow_ == sharedContext_) sharedContext_ = nullptr;
}

void CanvasGLFW::activate() { glfwMakeContextCurrent(glWindow_); }

void CanvasGLFW::glSwapBuffers() { glfwSwapBuffers(glWindow_); }

void CanvasGLFW::show() {
    if (!glfwGetWindowAttrib(glWindow_, GLFW_VISIBLE)) {
        glfwWindowCount_++;
        glfwShowWindow(glWindow_);
        update();
    }
}

void CanvasGLFW::hide() {
    if (glfwGetWindowAttrib(glWindow_, GLFW_VISIBLE)) {
        glfwWindowCount_--;
        glfwHideWindow(glWindow_);
    }
}

void CanvasGLFW::setWindowSize(ivec2 size) { glfwSetWindowSize(glWindow_, size.x, size.y); }

void CanvasGLFW::setWindowPosition(ivec2 pos) {
    ivec2 size{};
    glfwGetWindowSize(glWindow_, &size.x, &size.y);
    pos = movePointOntoDesktop(pos, size);
    glfwSetWindowPos(glWindow_, pos.x, pos.y);
}

void CanvasGLFW::setFullScreenInternal(bool fullscreen) {
    if (fullscreen && !isFullScreen_) {
        isFullScreen_ = true;
        glfwGetWindowPos(glWindow_, &oldPos_[0], &oldPos_[1]);
        glfwGetWindowSize(glWindow_, &oldSize_[0], &oldSize_[1]);

        int monitorCount;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

        GLFWmonitor* target = *monitors;  // Start with the primary monitor;
        for (GLFWmonitor** monitor = monitors; monitor < monitors + monitorCount; ++monitor) {
            ivec2 mpos;
            glfwGetMonitorPos(*monitor, &mpos[0], &mpos[1]);
            const GLFWvidmode* mode = glfwGetVideoMode(*monitor);
            if (glm::all(glm::greaterThanEqual(oldPos_, mpos)) &&
                glm::all(glm::lessThan(oldPos_, mpos + ivec2(mode->width, mode->height)))) {
                target = *monitor;  // Window is on this monitor
                break;
            }
        }
        const GLFWvidmode* mode = glfwGetVideoMode(target);
        glfwSetWindowMonitor(glWindow_, target, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    } else if (!fullscreen && isFullScreen_) {
        glfwSetWindowMonitor(glWindow_, nullptr, oldPos_.x, oldPos_.y, oldSize_.x, oldSize_.y,
                             GLFW_DONT_CARE);
    }
}

void CanvasGLFW::setWindowTitle(std::string windowTitle) {
    windowTitle_ = windowTitle;
    glfwSetWindowTitle(glWindow_, windowTitle_.c_str());
    RenderContext::getPtr()->setContextName(contextId(), windowTitle_);
}

void CanvasGLFW::closeWindow(GLFWwindow* window) {
    glfwSetWindowShouldClose(window, GL_FALSE);
    getCanvasGLFW(window)->hide();
}

int CanvasGLFW::getVisibleWindowCount() { return glfwWindowCount_; }

void CanvasGLFW::update() {
    activate();
    CanvasGL::update();
    RenderContext::getPtr()->activateDefaultRenderContext();
}

void CanvasGLFW::reshape(GLFWwindow* window, int width, int height) {
    getCanvasGLFW(window)->resize(uvec2(width, height));
}

void CanvasGLFW::move(GLFWwindow* window, int x, int y) {
    getCanvasGLFW(window)->getProcessorWidgetOwner()->ProcessorWidget::setPosition(ivec2(x, y));
}

void CanvasGLFW::setAlwaysOnTopByDefault(bool alwaysOnTop) { alwaysOnTop_ = alwaysOnTop; }

CanvasGLFW* CanvasGLFW::getCanvasGLFW(GLFWwindow* window) {
    return GLFWUserData::get<CanvasGLFW>(window, GLFWUserDataId::Window);
}

CanvasGLFW* CanvasGLFW::getSharedContext() {
    if (sharedContext_)
        return getCanvasGLFW(sharedContext_);
    else
        return nullptr;
}

void CanvasGLFW::releaseContext() {}

std::unique_ptr<Canvas> CanvasGLFW::createHiddenCanvas() {
    auto res = dispatchFront(
        [&]() { return std::make_unique<CanvasGLFW>("Background", screenDimensions_); });
    return res.get();
}

Canvas::ContextID CanvasGLFW::activeContext() const {
    return static_cast<ContextID>(glfwGetCurrentContext());
}

Canvas::ContextID CanvasGLFW::contextId() const { return static_cast<ContextID>(glWindow_); }

void CanvasGLFW::provideExternalContext(GLFWwindow* sharedContext) {
    if (!sharedContext_) {
        sharedContext_ = sharedContext;
    } else {
        throw Exception("Shared context can only be set once!", IVW_CONTEXT_CUSTOM("GLFW"));
    }
}

GLFWwindow* CanvasGLFW::sharedContext() { return sharedContext_; }

ivec2 CanvasGLFW::movePointOntoDesktop(ivec2 pos, ivec2 size) {
    auto check_error = []() {
        const char* description;
        int code = glfwGetError(&description);
        if (description) {
            throw Exception(fmt::format("GLFW Error: {} {}", code, description));
        }
    };

    try {
        int count;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        check_error();

        for (int i = 0; i < count; ++i) {
            if (GLFWmonitor* monitor = monitors[i]) {
                ivec2 screenPos{};
                ivec2 screenSize{};
                glfwGetMonitorWorkarea(monitor, &screenPos.x, &screenPos.y, &screenSize.x,
                                       &screenSize.y);
                check_error();

                if (glm::all(glm::greaterThanEqual(pos, screenPos)) &&
                    glm::all(glm::lessThanEqual(pos, screenPos + screenSize))) {

                    return pos;
                }

                if (glm::all(glm::greaterThanEqual(pos + size, screenPos)) &&
                    glm::all(glm::lessThanEqual(pos + size, screenPos + screenSize))) {

                    return glm::clamp(pos, screenPos, screenPos + screenSize);
                }
            }
        }

        if (auto monitor = glfwGetPrimaryMonitor()) {
            check_error();
            ivec2 screenPos{};
            ivec2 screenSize{};
            glfwGetMonitorWorkarea(monitor, &screenPos.x, &screenPos.y, &screenSize.x,
                                   &screenSize.y);
            check_error();
            return glm::clamp(pos, screenPos, screenPos + screenSize);
        } else {
            return pos;
        }
    } catch (const Exception& e) {
        LogErrorCustom("CanvasGLFW", e.getMessage());
        return pos;
    } catch (...) {
        LogErrorCustom("CanvasGLFW", "MovePointOntoDestop unknown exception");
        return pos;
    }
}

}  // namespace inviwo
