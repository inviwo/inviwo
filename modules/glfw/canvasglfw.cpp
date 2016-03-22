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

#include "canvasglfw.h"
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/processors/processorwidget.h>
#include <modules/openglqt/openglqtcapabilities.h>

namespace inviwo {

GLFWwindow* CanvasGLFW::sharedContext_ = nullptr;
int CanvasGLFW::glfwWindowCount_ = 0;
bool CanvasGLFW::alwaysOnTop_ = true;

CanvasGLFW::CanvasGLFW(std::string windowTitle, uvec2 dimensions)
    : CanvasGL(dimensions)
    , windowTitle_(windowTitle)
    , glWindow_(nullptr)
    , mouseButton_(MouseEvent::MOUSE_BUTTON_NONE)
    , mouseState_(MouseEvent::MOUSE_STATE_NONE)
    , mouseModifiers_(InteractionEvent::MODIFIER_NONE) {
    
    glfwWindowHint(GLFW_FLOATING, alwaysOnTop_ ? GL_TRUE : GL_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

#ifdef __APPLE__
    if (!sharedContext_ && OpenGLCapabilities::getPreferredProfile() == "core") {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }
#endif

    glWindow_ = glfwCreateWindow(getScreenDimensions().x, getScreenDimensions().y,
                                 windowTitle_.c_str(), nullptr, sharedContext_);

    if (!glWindow_) {
        glfwTerminate();
        throw Exception("Could not create GLFW window.", IvwContext);
    }

    if (!sharedContext_) sharedContext_ = glWindow_;

    // register callbacks
    glfwSetKeyCallback(glWindow_, keyboard);
    glfwSetMouseButtonCallback(glWindow_, mouseButton);
    glfwSetCursorPosCallback(glWindow_, mouseMotion);
    glfwSetScrollCallback(glWindow_, scroll);
    glfwSetWindowCloseCallback(glWindow_, closeWindow);
    glfwSetWindowUserPointer(glWindow_, this);
    glfwSetWindowSizeCallback(glWindow_, reshape);
    glfwSetWindowPosCallback(glWindow_, move);
}

CanvasGLFW::~CanvasGLFW() { 
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

void CanvasGLFW::setWindowSize(ivec2 size) {
    glfwSetWindowSize(glWindow_, size.x, size.y);
}

void CanvasGLFW::setWindowPosition(ivec2 pos) {
    glfwSetWindowPos(glWindow_, pos.x, pos.y);
}

void CanvasGLFW::setWindowTitle(std::string windowTitle) {
    windowTitle_ = windowTitle;
    glfwSetWindowTitle(glWindow_, windowTitle_.c_str());
}

void CanvasGLFW::closeWindow(GLFWwindow* window) { getCanvasGLFW(window)->hide(); }

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
    getCanvasGLFW(window)->getProcessorWidgetOwner()->ProcessorWidget::setPosition(ivec2(x,y));
}

void CanvasGLFW::setAlwaysOnTopByDefault(bool alwaysOnTop) { alwaysOnTop_ = alwaysOnTop; }

CanvasGLFW* CanvasGLFW::getCanvasGLFW(GLFWwindow* window) {
    return static_cast<CanvasGLFW*>(glfwGetWindowUserPointer(window));
}

CanvasGLFW* CanvasGLFW::getSharedContext() {
    if (sharedContext_)
        return getCanvasGLFW(sharedContext_);
    else
        return nullptr;
}

void CanvasGLFW::releaseContext() {}

void CanvasGLFW::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        // glfwSetWindowShouldClose(window, GL_TRUE);
        glfwTerminate();
        exit(0);
        return;
    }

    CanvasGLFW* thisCanvas = getCanvasGLFW(window);

    KeyboardEvent keyEvent(toupper(key), KeyboardEvent::MODIFIER_NONE,
                           KeyboardEvent::KEY_STATE_PRESS);

    thisCanvas->keyPressEvent(&keyEvent);
}

void CanvasGLFW::mouseButton(GLFWwindow* window, int button, int action, int mods) {
    CanvasGLFW* thisCanvas = getCanvasGLFW(window);
    thisCanvas->mouseButton_ = mapMouseButton(button);
    thisCanvas->mouseState_ = mapMouseState(action);
    thisCanvas->mouseModifiers_ = mapModifiers(mods);
    double x;
    double y;
    glfwGetCursorPos(window, &x, &y);
    ivec2 screenPos(floor(x), floor(y));
    ivec2 screenPosInvY(screenPos.x,
                        static_cast<int>(thisCanvas->getScreenDimensions().y) - 1 - screenPos.y);
    MouseEvent mouseEvent(screenPos, thisCanvas->mouseButton_, thisCanvas->mouseState_,
                          thisCanvas->mouseModifiers_, thisCanvas->getScreenDimensions(),
                          thisCanvas->getDepthValueAtCoord(screenPosInvY));

    if (thisCanvas->mouseState_ == MouseEvent::MOUSE_STATE_PRESS)
        thisCanvas->mousePressEvent(&mouseEvent);
    else if (thisCanvas->mouseState_ == MouseEvent::MOUSE_STATE_RELEASE)
        thisCanvas->mouseReleaseEvent(&mouseEvent);
}

void CanvasGLFW::mouseMotion(GLFWwindow* window, double x, double y) {
    CanvasGLFW* thisCanvas = getCanvasGLFW(window);
    ivec2 screenPos(floor(x), floor(y));
    ivec2 screenPosInvY(screenPos.x,
                        static_cast<int>(thisCanvas->getScreenDimensions().y) - 1 - screenPos.y);

    MouseEvent::MouseState state =
        (thisCanvas->mouseState_ == MouseEvent::MOUSE_STATE_PRESS ? MouseEvent::MOUSE_STATE_MOVE
                                                                  : thisCanvas->mouseState_);
    MouseEvent mouseEvent(screenPos, thisCanvas->mouseButton_, state, thisCanvas->mouseModifiers_,
                          thisCanvas->getScreenDimensions(),
                          thisCanvas->getDepthValueAtCoord(screenPosInvY));

    if (state == MouseEvent::MOUSE_STATE_MOVE)
        thisCanvas->mouseMoveEvent(&mouseEvent);
    else if (state == MouseEvent::MOUSE_STATE_RELEASE)
        thisCanvas->mouseReleaseEvent(&mouseEvent);
}

void CanvasGLFW::scroll(GLFWwindow* window, double xoffset, double yoffset) {
    CanvasGLFW* thisCanvas = getCanvasGLFW(window);
    thisCanvas->mouseButton_ = MouseEvent::MOUSE_BUTTON_MIDDLE;
    thisCanvas->mouseState_ = MouseEvent::MOUSE_STATE_WHEEL;
    thisCanvas->mouseModifiers_ = KeyboardEvent::MODIFIER_NONE;
    double x;
    double y;
    glfwGetCursorPos(window, &x, &y);
    ivec2 screenPos(floor(x), floor(y));
    ivec2 screenPosInvY(screenPos.x,
                        static_cast<int>(thisCanvas->getScreenDimensions().y) - 1 - screenPos.y);
    int delta = static_cast<int>(yoffset < 0.0 ? floor(yoffset) : ceil(yoffset));

    MouseEvent mouseEvent(screenPos, delta, thisCanvas->mouseButton_, thisCanvas->mouseState_,
                          MouseEvent::MOUSE_WHEEL_VERTICAL, thisCanvas->mouseModifiers_,
                          thisCanvas->getScreenDimensions(),
                          thisCanvas->getDepthValueAtCoord(screenPosInvY));

    thisCanvas->mouseWheelEvent(&mouseEvent);
}

MouseEvent::MouseButton CanvasGLFW::mapMouseButton(int mouseButtonGLFW) {
    if (mouseButtonGLFW == GLFW_MOUSE_BUTTON_LEFT)
        return MouseEvent::MOUSE_BUTTON_LEFT;
    else if (mouseButtonGLFW == GLFW_MOUSE_BUTTON_MIDDLE)
        return MouseEvent::MOUSE_BUTTON_MIDDLE;
    else if (mouseButtonGLFW == GLFW_MOUSE_BUTTON_RIGHT)
        return MouseEvent::MOUSE_BUTTON_RIGHT;
    else
        return MouseEvent::MOUSE_BUTTON_NONE;
}

MouseEvent::MouseState CanvasGLFW::mapMouseState(int mouseStateGLFW) {
    if (mouseStateGLFW == GLFW_PRESS)
        return MouseEvent::MOUSE_STATE_PRESS;
    else if (mouseStateGLFW == GLFW_RELEASE)
        return MouseEvent::MOUSE_STATE_RELEASE;
    else
        return MouseEvent::MOUSE_STATE_NONE;
}

InteractionEvent::Modifier CanvasGLFW::mapModifiers(int modifiersGLFW) {
    int result = KeyboardEvent::MODIFIER_NONE;

    if (modifiersGLFW & GLFW_MOD_ALT) result |= InteractionEvent::MODIFIER_ALT;

    if (modifiersGLFW & GLFW_MOD_CONTROL) result |= InteractionEvent::MODIFIER_CTRL;

    if (modifiersGLFW & GLFW_MOD_SHIFT) result |= InteractionEvent::MODIFIER_SHIFT;

    return static_cast<InteractionEvent::Modifier>(result);
}

std::unique_ptr<Canvas> CanvasGLFW::createHiddenCanvas() {
    auto res = dispatchFront([&]() {
        auto canvas = util::make_unique<CanvasGLFW>(windowTitle_, screenDimensions_);
        return std::move(canvas);
    });
    return res.get();
}

Canvas::ContextID CanvasGLFW::activeContext() const {
    return static_cast<ContextID>(glfwGetCurrentContext());
}

}  // namespace
