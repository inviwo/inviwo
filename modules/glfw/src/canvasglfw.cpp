/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>            // for dispatchFront
#include <inviwo/core/interaction/events/event.h>            // for Event
#include <inviwo/core/interaction/events/eventpropagator.h>  // for EventPropagator
#include <inviwo/core/interaction/pickingcontroller.h>       // for PickingController
#include <inviwo/core/network/networklock.h>                 // for NetworkLock
#include <inviwo/core/util/canvas.h>                         // for Canvas::ContextID, Canvas
#include <inviwo/core/util/exception.h>                      // for Exception
#include <inviwo/core/util/glmvec.h>                         // for ivec2, uvec2, size2_t, dvec2
#include <inviwo/core/util/logcentral.h>                     // for LogCentral, LogErrorCustom
#include <inviwo/core/util/rendercontext.h>                  // for CanvasContextHolder, RenderC...
#include <inviwo/core/util/sourcecontext.h>                  // for IVW_CONTEXT_CUSTOM
#include <modules/glfw/glfwexception.h>                      // for GLFWException
#include <modules/glfw/glfwuserdata.h>                       // for GLFWUserData, GLFWUserDataId
#include <modules/opengl/canvasgl.h>                         // for CanvasGL
#include <modules/opengl/inviwoopengl.h>                     // for GL_FALSE, GL_TRUE

#include <future>       // for future
#include <string_view>  // for string_view

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>               // for GLFWwindow, glfwWindowHint
#include <glm/common.hpp>             // for clamp
#include <glm/vec2.hpp>               // for operator+, vec<>::(anonymous)
#include <glm/vector_relational.hpp>  // for all, greaterThanEqual, lessT...

#include <inviwo/tracy/tracy.h>

namespace inviwo {

GLFWwindow* GLFWWindowHandler::sharedContext_ = nullptr;
int CanvasGLFW::glfwWindowCount_ = 0;
bool CanvasGLFW::alwaysOnTop_ = false;

GLFWWindowHandler::GLFWWindowHandler(Canvas* canvas, const std::string& title, uvec2 dimensions)
    : glWindow_(createWindow(title, dimensions)) {

    if (!sharedContext_) {
        sharedContext_ = glWindow_;
    }

    RenderContext::getPtr()->registerContext(getContextId(), title,
                                             std::make_unique<CanvasContextHolder>(canvas));
}

GLFWWindowHandler::~GLFWWindowHandler() {
    RenderContext::getPtr()->unRegisterContext(getContextId());
    glfwDestroyWindow(glWindow_);
    if (glWindow_ == sharedContext_) sharedContext_ = nullptr;
    RenderContext::getPtr()->activateDefaultRenderContext();
}

void GLFWWindowHandler::provideExternalContext(GLFWwindow* sharedContext) {
    if (!sharedContext_) {
        sharedContext_ = sharedContext;
    } else {
        throw Exception("Shared context can only be set once!", IVW_CONTEXT_CUSTOM("GLFW"));
    }
}

GLFWwindow* GLFWWindowHandler::sharedContext() { return sharedContext_; }

GLFWwindow* GLFWWindowHandler::createWindow(const std::string& title, uvec2 dimensions) {
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
        throw GLFWException("Could not create GLFW window.",
                            IVW_CONTEXT_CUSTOM("GLFWContextHandler"));
    }
    return win;
}

CanvasGLFW::CanvasGLFW(const std::string& title, uvec2 dimensions)
    : GLFWWindowHandler(this, title, dimensions)
    , CanvasGL()
    , userdata_{glWindow_}
    , eventManager_{glWindow_,
                    [this](Event* event) {
                        if (!propagator_) return;
                        NetworkLock lock;
                        pickingController_.propagateEvent(event, propagator_);
                        if (event->hasBeenUsed()) return;
                        propagator_->propagateEvent(event, nullptr);
                    },
                    [this](dvec2 pos) -> double { return getDepthValueAtNormalizedCoord(pos); }} {

    userdata_.set(GLFWUserDataId::Window, this);

    // register callbacks
    glfwSetWindowCloseCallback(glWindow_, [](GLFWwindow* window) {
        glfwSetWindowShouldClose(window, GL_FALSE);
        getCanvasGLFW(window)->setVisible(false);
    });
    glfwSetWindowSizeCallback(glWindow_, [](GLFWwindow* window, int width, int height) {
        auto canvas = getCanvasGLFW(window);
        if (canvas->onWindowSizeChange) {
            canvas->onWindowSizeChange(ivec2(width, height));
        }
    });
    glfwSetFramebufferSizeCallback(glWindow_, [](GLFWwindow* window, int width, int height) {
        auto canvas = getCanvasGLFW(window);
        canvas->image_.reset();
        canvas->pickingController_.setPickingSource(nullptr);

        if (canvas->onFramebufferSizeChange) {
            canvas->onFramebufferSizeChange(ivec2(width, height));
        }
    });
    glfwSetWindowPosCallback(glWindow_, [](GLFWwindow* window, int x, int y) {
        auto canvas = getCanvasGLFW(window);
        if (canvas->onPositionChange) {
            canvas->onPositionChange(ivec2(x, y));
        }
    });
}

CanvasGLFW::~CanvasGLFW() {
    // activate current context before CanvasGL is deconstructed
    activate();
}

void CanvasGLFW::update() {
    activate();
    CanvasGL::update();
    RenderContext::getPtr()->activateDefaultRenderContext();
}

void CanvasGLFW::activate() {
    TRACY_ZONE_SCOPED_C(0x000088);
    glfwMakeContextCurrent(glWindow_);
}

std::unique_ptr<Canvas> CanvasGLFW::createHiddenCanvas() {
    auto res = dispatchFront([&]() { return std::make_unique<CanvasGLFW>("Background"); });
    return res.get();
}

Canvas::ContextID CanvasGLFW::activeContext() const {
    return static_cast<ContextID>(glfwGetCurrentContext());
}

Canvas::ContextID CanvasGLFW::contextId() const { return getContextId(); }

void CanvasGLFW::releaseContext() {}

void CanvasGLFW::glSwapBuffers() { 
	TRACY_ZONE_SCOPED_C(0x000088);
	glfwSwapBuffers(glWindow_); 
}

size2_t CanvasGLFW::getCanvasDimensions() const {
    return static_cast<size2_t>(getFramebufferSize());
}

void CanvasGLFW::setVisible(bool visible) {
    if (visible) {
        if (!glfwGetWindowAttrib(glWindow_, GLFW_VISIBLE)) {
            glfwWindowCount_++;
            glfwShowWindow(glWindow_);
            update();
        }
    } else {
        if (glfwGetWindowAttrib(glWindow_, GLFW_VISIBLE)) {
            glfwWindowCount_--;
            glfwHideWindow(glWindow_);
        }
    }
}

int CanvasGLFW::getVisibleWindowCount() { return glfwWindowCount_; }

void CanvasGLFW::setWindowSize(ivec2 size) { glfwSetWindowSize(glWindow_, size.x, size.y); }

ivec2 CanvasGLFW::getWindowSize() const {
    int width, height;
    glfwGetWindowSize(glWindow_, &width, &height);
    return {width, height};
}

ivec2 CanvasGLFW::getFramebufferSize() const {
    int width, height;
    glfwGetFramebufferSize(glWindow_, &width, &height);
    return {width, height};
}

void CanvasGLFW::setWindowPosition(ivec2 pos) {
    ivec2 size{};
    glfwGetWindowSize(glWindow_, &size.x, &size.y);
    pos = movePointOntoDesktop(pos, size);
    glfwSetWindowPos(glWindow_, pos.x, pos.y);
}

ivec2 CanvasGLFW::getWindowPosition() const {
    int x, y;
    glfwGetWindowPos(glWindow_, &x, &y);
    return {x, y};
}

void CanvasGLFW::setWindowTitle(const std::string& windowTitle) {
    glfwSetWindowTitle(glWindow_, windowTitle.c_str());
    RenderContext::getPtr()->setContextName(getContextId(), windowTitle);
}

void CanvasGLFW::setOnTop(bool onTop) { glfwSetWindowAttrib(glWindow_, GLFW_FLOATING, onTop); }

void CanvasGLFW::setFullScreen(bool fullscreen) {
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
CanvasGLFW* CanvasGLFW::getCanvasGLFW(GLFWwindow* window) {
    return GLFWUserData::get<CanvasGLFW>(window, GLFWUserDataId::Window);
}

ivec2 CanvasGLFW::movePointOntoDesktop(ivec2 pos, ivec2 size) {
    auto check_error = []() {
        const char* description;
        int code = glfwGetError(&description);
        if (description) {
            throw Exception(IVW_CONTEXT_CUSTOM("CanvasGLFW"), "GLFW Error: {} {}", code,
                            description);
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
