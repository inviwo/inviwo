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
#include <inviwo/core/network/processornetworkevaluator.h>
#include <modules/opengl/openglcapabilities.h>

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
    , mouseModifiers_(InteractionEvent::MODIFIER_NONE)
{
    if(alwaysOnTop_){
        glfwWindowHint(GLFW_FLOATING, GL_TRUE);
    }
    else{
        glfwWindowHint(GLFW_FLOATING, GL_FALSE);
    }
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
}

CanvasGLFW::~CanvasGLFW() {
    glWindow_ = nullptr;
}

void CanvasGLFW::initialize() {
    CanvasGL::initialize();
}

void CanvasGLFW::initializeGL() {
    if(!sharedContext_){
        std::string preferProfile = OpenGLCapabilities::getPreferredProfile();
#ifdef __APPLE__
        if (preferProfile == "core") {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        } 
#endif
    }

    glWindow_ = glfwCreateWindow(getScreenDimensions().x, getScreenDimensions().y, windowTitle_.c_str(), nullptr, sharedContext_);

    if (!glWindow_) {
        glfwTerminate();
        throw Exception("Could not create GLFW window.");
    }

    if (!sharedContext_) {
        sharedContext_ = glWindow_;
    }

    glfwWindowCount_++;

    glfwSetKeyCallback(glWindow_, keyboard);
    glfwSetMouseButtonCallback(glWindow_, mouseButton);
    glfwSetCursorPosCallback(glWindow_, mouseMotion);
    glfwSetScrollCallback(glWindow_, scroll);
    glfwSetWindowCloseCallback(glWindow_, closeWindow);
    glfwSetWindowUserPointer(glWindow_, this);
    glfwSetWindowSizeCallback(glWindow_, reshape);
    
    activate();

    OpenGLCapabilities::initializeGLEW();
}

void CanvasGLFW::initializeSquare() {
    CanvasGL::initializeSquare();
}

void CanvasGLFW::deinitialize() {
    CanvasGL::deinitialize();
}

void CanvasGLFW::activate() {
    glfwMakeContextCurrent(glWindow_);
}

void CanvasGLFW::glSwapBuffers() {
    glfwSwapBuffers(glWindow_);
}

void CanvasGLFW::show(){
    glfwShowWindow(glWindow_);
    update();
}

void CanvasGLFW::hide(){
    glfwHideWindow(glWindow_);
}

void CanvasGLFW::setWindowSize(uvec2 size) {
    glfwSetWindowSize(glWindow_, static_cast<int>(size.x), static_cast<int>(size.y));
}

void CanvasGLFW::setWindowTitle(std::string windowTitle) {
    windowTitle_ = windowTitle;
    glfwSetWindowTitle(glWindow_, windowTitle_.c_str());
}

void CanvasGLFW::closeWindow(GLFWwindow* window) {
    glfwWindowCount_--;
    if(sharedContext_ != window)
        glfwDestroyWindow(window);
}

int CanvasGLFW::getWindowCount(){
    return glfwWindowCount_;
}

void CanvasGLFW::reshape(GLFWwindow* window, int width, int height) {
    getCanvasGLFW(window)->resize(uvec2(width, height));
}

void CanvasGLFW::setAlwaysOnTopByDefault(bool alwaysOnTop){
    alwaysOnTop_ = alwaysOnTop;
}

CanvasGLFW* CanvasGLFW::getCanvasGLFW(GLFWwindow* window){
    return static_cast<CanvasGLFW*>(glfwGetWindowUserPointer(window));
}

CanvasGLFW* CanvasGLFW::getSharedContext(){
    if(sharedContext_)
        return getCanvasGLFW(sharedContext_);
    else
        return nullptr;
}

void CanvasGLFW::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        //glfwSetWindowShouldClose(window, GL_TRUE);
        glfwTerminate();
        exit(0);
        return;
    }

    CanvasGLFW* thisCanvas = getCanvasGLFW(window);

    KeyboardEvent* keyEvent = new KeyboardEvent(
        toupper(key),
        KeyboardEvent::MODIFIER_NONE,
        KeyboardEvent::KEY_STATE_PRESS);

    thisCanvas->keyPressEvent(keyEvent);

    delete keyEvent;
}

void CanvasGLFW::mouseButton(GLFWwindow* window, int button, int action, int mods) {
    CanvasGLFW* thisCanvas = getCanvasGLFW(window);
    thisCanvas->mouseButton_ = mapMouseButton(button);
    thisCanvas->mouseState_ = mapMouseState(action);
    thisCanvas->mouseModifiers_ = mapModifiers(mods);
    double x;
    double y;
    glfwGetCursorPos(window, &x, &y);
    MouseEvent* mouseEvent = new MouseEvent(ivec2(floor(x), floor(y)), thisCanvas->mouseButton_,
        thisCanvas->mouseState_, thisCanvas->mouseModifiers_, thisCanvas->getScreenDimensions());

    if (thisCanvas->mouseState_ == MouseEvent::MOUSE_STATE_PRESS) thisCanvas->mousePressEvent(mouseEvent);
    else if (thisCanvas->mouseState_ == MouseEvent::MOUSE_STATE_RELEASE) thisCanvas->mouseReleaseEvent(mouseEvent);

    delete mouseEvent;
}

void CanvasGLFW::mouseMotion(GLFWwindow* window, double x, double y) {
    CanvasGLFW* thisCanvas = getCanvasGLFW(window);
    MouseEvent* mouseEvent = new MouseEvent(ivec2(floor(x), floor(y)), thisCanvas->mouseButton_,
        thisCanvas->mouseState_, thisCanvas->mouseModifiers_, thisCanvas->getScreenDimensions());

    if (thisCanvas->mouseState_ == MouseEvent::MOUSE_STATE_PRESS) thisCanvas->mousePressEvent(mouseEvent);
    else if (thisCanvas->mouseState_ == MouseEvent::MOUSE_STATE_RELEASE) thisCanvas->mouseReleaseEvent(mouseEvent);

    delete mouseEvent;
}

void CanvasGLFW::scroll(GLFWwindow* window, double xoffset, double yoffset) {
    CanvasGLFW* thisCanvas = getCanvasGLFW(window);
    thisCanvas->mouseButton_ = MouseEvent::MOUSE_BUTTON_MIDDLE;
    thisCanvas->mouseState_ = MouseEvent::MOUSE_STATE_WHEEL;
    thisCanvas->mouseModifiers_ = KeyboardEvent::MODIFIER_NONE;
    double x;
    double y;
    glfwGetCursorPos(window, &x, &y);
    int delta = (yoffset<0.0 ? floor(yoffset) : ceil(yoffset));
    MouseEvent* mouseEvent = new MouseEvent(ivec2(floor(x), floor(y)), delta, thisCanvas->mouseButton_,
        thisCanvas->mouseState_, MouseEvent::MOUSE_WHEEL_VERTICAL, thisCanvas->mouseModifiers_, thisCanvas->getScreenDimensions());

    thisCanvas->mouseWheelEvent(mouseEvent);

    delete mouseEvent;
}

MouseEvent::MouseButton CanvasGLFW::mapMouseButton(int mouseButtonGLFW) {
    if (mouseButtonGLFW == GLFW_MOUSE_BUTTON_LEFT) return MouseEvent::MOUSE_BUTTON_LEFT;
    else if (mouseButtonGLFW == GLFW_MOUSE_BUTTON_MIDDLE) return MouseEvent::MOUSE_BUTTON_MIDDLE;
    else if (mouseButtonGLFW == GLFW_MOUSE_BUTTON_RIGHT) return MouseEvent::MOUSE_BUTTON_RIGHT;
    else return MouseEvent::MOUSE_BUTTON_NONE;
}

MouseEvent::MouseState CanvasGLFW::mapMouseState(int mouseStateGLFW) {
    if (mouseStateGLFW == GLFW_PRESS) return MouseEvent::MOUSE_STATE_PRESS;
    else if (mouseStateGLFW == GLFW_RELEASE) return MouseEvent::MOUSE_STATE_RELEASE;
    else return MouseEvent::MOUSE_STATE_NONE;
}

InteractionEvent::Modifier CanvasGLFW::mapModifiers(int modifiersGLFW) {
    int result = KeyboardEvent::MODIFIER_NONE;

    if (modifiersGLFW & GLFW_MOD_ALT) result |= InteractionEvent::MODIFIER_ALT;

    if (modifiersGLFW & GLFW_MOD_CONTROL) result |= InteractionEvent::MODIFIER_CTRL;

    if (modifiersGLFW & GLFW_MOD_SHIFT) result |= InteractionEvent::MODIFIER_SHIFT;

    return static_cast<InteractionEvent::Modifier>(result);
}

} // namespace
