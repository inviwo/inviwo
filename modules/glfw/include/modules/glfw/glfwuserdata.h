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
#pragma once

#include <modules/glfw/glfwmoduledefine.h>

#include <unordered_map>
#include <any>

typedef struct GLFWwindow GLFWwindow;

namespace inviwo {

enum class GLFWUserDataId { Window, Interaction, Other };

/**
 * A helper to be able to pass more data around with the glfw user pointer
 */
class IVW_MODULE_GLFW_API GLFWUserData {
public:
    GLFWUserData(GLFWwindow* win);

    template <typename T>
    void set(GLFWUserDataId id, T* ptr) {
        data_[id] = ptr;
    }
    template <typename T>
    T* get(GLFWUserDataId id) {
        return std::any_cast<T*>(data_[id]);
    }

    template <typename T>
    static T* get(GLFWwindow* win, GLFWUserDataId id) {
        return self(win)->get<T>(id);
    }

    template <typename T>
    static void set(GLFWwindow* win, GLFWUserDataId id, T* ptr) {
        self(win)->set(id, ptr);
    }

private:
    static GLFWUserData* self(GLFWwindow* win);

    std::unordered_map<GLFWUserDataId, std::any> data_;
};

}  // namespace inviwo
