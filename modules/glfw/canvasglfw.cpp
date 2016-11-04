/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
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
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/util/rendercontext.h>
#include <modules/openglqt/openglqtcapabilities.h>

namespace inviwo {

GLFWwindow* CanvasGLFW::sharedContext_ = nullptr;
int CanvasGLFW::glfwWindowCount_ = 0;
bool CanvasGLFW::alwaysOnTop_ = true;

CanvasGLFW::CanvasGLFW(std::string windowTitle, uvec2 dimensions)
    : CanvasGL(dimensions)
    , windowTitle_(windowTitle)
    , glWindow_(nullptr)
    , mouseButton_(MouseButton::None)
    , mouseState_(MouseState::Release)
    , mouseModifiers_(flags::none) {
    
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

    glWindow_ = glfwCreateWindow(static_cast<int>(getCanvasDimensions().x), 
                                 static_cast<int>(getCanvasDimensions().y),
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

dvec2 CanvasGLFW::normalPos(dvec2 pos) const {
    return util::invertY(pos, this->getCanvasDimensions()) / dvec2(this->getCanvasDimensions());
}

void CanvasGLFW::releaseContext() {}

void CanvasGLFW::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        // glfwSetWindowShouldClose(window, GL_TRUE);
        glfwTerminate();
        exit(0);
        return;
    }

    auto thisCanvas = getCanvasGLFW(window);

    KeyboardEvent keyEvent(static_cast<IvwKey>(toupper(key)));

    thisCanvas->propagateEvent(&keyEvent);
}

void CanvasGLFW::mouseButton(GLFWwindow* window, int button, int action, int mods) {
    auto thisCanvas = getCanvasGLFW(window);
    thisCanvas->mouseButton_ = mapMouseButton(button);
    thisCanvas->mouseState_ = mapMouseState(action);
    thisCanvas->mouseModifiers_ = mapModifiers(mods);

    dvec2 pos;
    glfwGetCursorPos(window, &pos.x, &pos.y);
    pos = thisCanvas->normalPos(pos);

    MouseEvent mouseEvent(thisCanvas->mouseButton_, thisCanvas->mouseState_,
                          thisCanvas->mouseButton_, thisCanvas->mouseModifiers_, pos,
                          thisCanvas->getImageDimensions(),
                          thisCanvas->getDepthValueAtNormalizedCoord(pos));

    thisCanvas->propagateEvent(&mouseEvent);
}

void CanvasGLFW::mouseMotion(GLFWwindow* window, double x, double y) {
    auto thisCanvas = getCanvasGLFW(window);

    const auto pos = thisCanvas->normalPos(dvec2(x, y));

    MouseState state =
        (thisCanvas->mouseState_ == MouseState::Press ? MouseState::Move : thisCanvas->mouseState_);
    MouseEvent mouseEvent(thisCanvas->mouseButton_, state, thisCanvas->mouseButton_,
                          thisCanvas->mouseModifiers_, pos, thisCanvas->getImageDimensions(),
                          thisCanvas->getDepthValueAtNormalizedCoord(pos));

    thisCanvas->propagateEvent(&mouseEvent);
}

void CanvasGLFW::scroll(GLFWwindow* window, double xoffset, double yoffset) {
    auto thisCanvas = getCanvasGLFW(window);

    dvec2 pos;
    glfwGetCursorPos(window, &pos.x, &pos.y);
    pos = thisCanvas->normalPos(pos);

    WheelEvent wheelEvent(thisCanvas->mouseButton_, thisCanvas->mouseModifiers_,
                          dvec2(xoffset, yoffset), pos, thisCanvas->getImageDimensions(),
                          thisCanvas->getDepthValueAtNormalizedCoord(pos));

    thisCanvas->propagateEvent(&wheelEvent);
}

MouseButton CanvasGLFW::mapMouseButton(int mouseButtonGLFW) {
    if (mouseButtonGLFW == GLFW_MOUSE_BUTTON_LEFT)
        return MouseButton::Left;
    else if (mouseButtonGLFW == GLFW_MOUSE_BUTTON_MIDDLE)
        return MouseButton::Middle;
    else if (mouseButtonGLFW == GLFW_MOUSE_BUTTON_RIGHT)
        return MouseButton::Right;
    else
        return MouseButton::None;
}

MouseState CanvasGLFW::mapMouseState(int mouseStateGLFW) {
    if (mouseStateGLFW == GLFW_PRESS)
        return MouseState::Press;
    else // (mouseStateGLFW == GLFW_RELEASE)
        return MouseState::Release;
}

KeyModifiers CanvasGLFW::mapModifiers(int modifiersGLFW) {
    KeyModifiers result(flags::none);

    if (modifiersGLFW & GLFW_MOD_ALT) result |= KeyModifier::Alt;

    if (modifiersGLFW & GLFW_MOD_CONTROL) result |= KeyModifier::Control;

    if (modifiersGLFW & GLFW_MOD_SHIFT) result |= KeyModifier::Shift;

    return result;
}

std::unique_ptr<Canvas> CanvasGLFW::createHiddenCanvas() {
    auto res = dispatchFront([&]() {
        return util::make_unique<CanvasGLFW>(windowTitle_, screenDimensions_);
    });
    return res.get();
}

Canvas::ContextID CanvasGLFW::activeContext() const {
    return static_cast<ContextID>(glfwGetCurrentContext());
}

bool CanvasGLFW::isFullScreen() const {
    return false;
}

void CanvasGLFW::setFullScreen(bool fullscreen) {}

}  // namespace
