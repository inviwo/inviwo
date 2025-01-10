/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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
#pragma once

#include <modules/glfw/glfwmoduledefine.h>

#include <inviwo/core/interaction/events/keyboardkeys.h>
#include <inviwo/core/interaction/events/mousebuttons.h>
#include <inviwo/core/util/glmvec.h>

#include <functional>

typedef struct GLFWwindow GLFWwindow;

namespace inviwo {

class Event;

namespace util {

IVW_MODULE_GLFW_API MouseButton mapGLFWMouseButton(int mouseButtonGLFW);
IVW_MODULE_GLFW_API MouseState mapGLFWMouseState(int mouseStateGLFW);

IVW_MODULE_GLFW_API KeyModifiers mapGLFWModifiers(int modifiersGLFW);

IVW_MODULE_GLFW_API KeyState mapGLFWMKeyState(int actionGLFW);
IVW_MODULE_GLFW_API IvwKey mapGLFWMKey(int keyGLFW);

}  // namespace util

/**
 * A helper class to handle GLFW mouse/events
 */
class IVW_MODULE_GLFW_API GLFWWindowEventManager {
public:
    GLFWWindowEventManager(GLFWwindow* glWindow, std::function<void(Event*)> ep,
                           std::function<double(dvec2)> depth);
    virtual ~GLFWWindowEventManager();

private:
    static void keyboard(GLFWwindow*, int, int, int, int);
    static void character(GLFWwindow*, unsigned int);  ///< UTF32 encoded text input
    static void mouseButton(GLFWwindow*, int, int, int);
    static void mouseMotion(GLFWwindow*, double, double);
    static void scroll(GLFWwindow*, double, double);

    void propagateEvent(Event* event);

    static dvec2 normalPos(dvec2 pos, ivec2 size);

    MouseButton mouseButton_;
    MouseState mouseState_;
    KeyModifiers modifiers_;

    GLFWwindow* glWindow_;
    std::function<void(Event*)> eventPropagator_;
    std::function<double(dvec2)> depth_;
};

}  // namespace inviwo
