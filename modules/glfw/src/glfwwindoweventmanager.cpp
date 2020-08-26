/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <modules/glfw/glfwwindoweventmanager.h>

#include <inviwo/core/interaction/events/keyboardevent.h>  // for KeyboardEvent
#include <inviwo/core/interaction/events/keyboardkeys.h>   // for KeyModifiers, KeyModifier, Key...
#include <inviwo/core/interaction/events/mousebuttons.h>   // for MouseState, MouseButton, Mouse...
#include <inviwo/core/interaction/events/mouseevent.h>     // for MouseEvent
#include <inviwo/core/interaction/events/wheelevent.h>     // for WheelEvent
#include <inviwo/core/util/glm.h>                          // for invertY
#include <inviwo/core/util/glmvec.h>                       // for dvec2, ivec2, uvec2
#include <modules/glfw/glfwuserdata.h>                     // for GLFWUserData, GLFWUserDataId

#include <cctype>   // for toupper
#include <utility>  // for move

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>   // for glfwGetWindowSize, GLFWwindow
#include <flags/flags.h>  // for none
#include <glm/vec2.hpp>   // for vec<>::(anonymous), operator/
#include <utf8/core.h>
#include <utf8/checked.h>

namespace inviwo {

MouseButton util::mapGLFWMouseButton(int mouseButtonGLFW) {
    if (mouseButtonGLFW == GLFW_MOUSE_BUTTON_LEFT)
        return MouseButton::Left;
    else if (mouseButtonGLFW == GLFW_MOUSE_BUTTON_MIDDLE)
        return MouseButton::Middle;
    else if (mouseButtonGLFW == GLFW_MOUSE_BUTTON_RIGHT)
        return MouseButton::Right;
    else
        return MouseButton::None;
}

MouseState util::mapGLFWMouseState(int mouseStateGLFW) {
    if (mouseStateGLFW == GLFW_PRESS)
        return MouseState::Press;
    else  // (mouseStateGLFW == GLFW_RELEASE)
        return MouseState::Release;
}

KeyModifiers util::mapGLFWModifiers(int modifiersGLFW) {
    KeyModifiers result(flags::none);

    if (modifiersGLFW & GLFW_MOD_ALT) result |= KeyModifier::Alt;

    if (modifiersGLFW & GLFW_MOD_CONTROL) result |= KeyModifier::Control;

    if (modifiersGLFW & GLFW_MOD_SHIFT) result |= KeyModifier::Shift;

    if (modifiersGLFW & GLFW_MOD_SUPER) result |= KeyModifier::Super;

    return result;
}

KeyState util::mapGLFWMKeyState(int actionGLFW) {
    return (actionGLFW == GLFW_PRESS) ? KeyState::Press : KeyState::Release;
}

IvwKey util::mapGLFWMKey(int keyGLFW) { return static_cast<IvwKey>(toupper(keyGLFW)); }

GLFWWindowEventManager::GLFWWindowEventManager(GLFWwindow* glWindow, std::function<void(Event*)> ep,
                                               std::function<double(dvec2)> depth)
    : mouseButton_(MouseButton::None)
    , mouseState_(MouseState::Release)
    , modifiers_(flags::none)
    , glWindow_{glWindow}
    , eventPropagator_{std::move(ep)}
    , depth_{std::move(depth)} {

    GLFWUserData::set(glWindow_, GLFWUserDataId::Interaction, this);

    glfwSetKeyCallback(glWindow_, keyboard);
    glfwSetCharCallback(glWindow_, character);
    glfwSetMouseButtonCallback(glWindow_, mouseButton);
    glfwSetCursorPosCallback(glWindow_, mouseMotion);
    glfwSetScrollCallback(glWindow_, scroll);
}

GLFWWindowEventManager::~GLFWWindowEventManager() {
    glfwSetKeyCallback(glWindow_, nullptr);
    glfwSetCharCallback(glWindow_, nullptr);
    glfwSetMouseButtonCallback(glWindow_, nullptr);
    glfwSetCursorPosCallback(glWindow_, nullptr);
    glfwSetScrollCallback(glWindow_, nullptr);
}

void GLFWWindowEventManager::keyboard(GLFWwindow* window, int key, int scancode, int action,
                                      int mods) {
    auto self = GLFWUserData::get<GLFWWindowEventManager>(window, GLFWUserDataId::Interaction);
    const auto keyState = util::mapGLFWMKeyState(action);
    const auto ivwkey = util::mapGLFWMKey(key);

    self->modifiers_ = util::mapGLFWModifiers(mods);
    KeyboardEvent keyEvent(ivwkey, keyState, self->modifiers_, scancode, "");
    self->propagateEvent(&keyEvent);
}

void GLFWWindowEventManager::character(GLFWwindow* window, unsigned int character) {
    auto self = GLFWUserData::get<GLFWWindowEventManager>(window, GLFWUserDataId::Interaction);

    // Convert event character from utf-32 to utf-8
    std::u32string input = {character};
    auto text = utf8::utf32to8(input);

    KeyboardEvent keyEvent(IvwKey::Unknown, KeyState::Press, self->modifiers_, character, text);
    self->propagateEvent(&keyEvent);
}

void GLFWWindowEventManager::mouseButton(GLFWwindow* window, int button, int action, int mods) {
    auto self = GLFWUserData::get<GLFWWindowEventManager>(window, GLFWUserDataId::Interaction);
    self->mouseButton_ = util::mapGLFWMouseButton(button);
    self->mouseState_ = util::mapGLFWMouseState(action);
    self->modifiers_ = util::mapGLFWModifiers(mods);

    dvec2 pos;
    glfwGetCursorPos(window, &pos.x, &pos.y);
    ivec2 size{};
    glfwGetWindowSize(window, &size.x, &size.y);
    pos = self->normalPos(pos, size);

    MouseEvent mouseEvent(self->mouseButton_, self->mouseState_, self->mouseButton_,
                          self->modifiers_, pos, uvec2{size}, self->depth_(pos));

    self->propagateEvent(&mouseEvent);
}

void GLFWWindowEventManager::mouseMotion(GLFWwindow* window, double x, double y) {
    auto self = GLFWUserData::get<GLFWWindowEventManager>(window, GLFWUserDataId::Interaction);

    ivec2 size{};
    glfwGetWindowSize(window, &size.x, &size.y);
    const auto pos = normalPos(dvec2(x, y), size);

    MouseState state =
        (self->mouseState_ == MouseState::Press ? MouseState::Move : self->mouseState_);
    MouseEvent mouseEvent(self->mouseButton_, state, self->mouseButton_, self->modifiers_, pos,
                          uvec2{size}, self->depth_(pos));

    self->propagateEvent(&mouseEvent);
}

void GLFWWindowEventManager::scroll(GLFWwindow* window, double xoffset, double yoffset) {
    auto self = GLFWUserData::get<GLFWWindowEventManager>(window, GLFWUserDataId::Interaction);

    dvec2 pos;
    glfwGetCursorPos(window, &pos.x, &pos.y);
    ivec2 size{};
    glfwGetWindowSize(window, &size.x, &size.y);
    pos = normalPos(pos, size);

    WheelEvent wheelEvent(self->mouseButton_, self->modifiers_, dvec2(xoffset, yoffset), pos,
                          uvec2{size}, self->depth_(pos));

    self->propagateEvent(&wheelEvent);
}

void GLFWWindowEventManager::propagateEvent(Event* event) { eventPropagator_(event); }

dvec2 GLFWWindowEventManager::normalPos(dvec2 pos, ivec2 size) {
    dvec2 dsize{size};
    return util::invertY(pos, dsize) / dsize;
}

}  // namespace inviwo
