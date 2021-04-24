/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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
#include <modules/opengl/canvasgl.h>
#include <modules/glfw/glfwuserdata.h>
#include <modules/glfw/glfwwindoweventmanager.h>

#include <functional>

typedef struct GLFWwindow GLFWwindow;

namespace inviwo {

class IVW_MODULE_GLFW_API CanvasGLFW : public CanvasGL {
    friend class CanvasProcessorWidgetGLFW;

public:
    CanvasGLFW(const std::string& title = "", uvec2 dimensions = uvec2(128));
    virtual ~CanvasGLFW();

    virtual void activate() override;
    virtual void glSwapBuffers() override;

    void setVisible(bool visible);

    void setWindowSize(ivec2);
    ivec2 getWindowSize() const;
    ivec2 getFramebufferSize() const;
    void setWindowPosition(ivec2);
    ivec2 getWindowPosition() const;

    void setWindowTitle(std::string);

    static int getVisibleWindowCount();

    virtual void update() override;

    void setOnTop(bool);

    virtual std::unique_ptr<Canvas> createHiddenCanvas() override;
    virtual ContextID activeContext() const override;
    virtual ContextID contextId() const override;

    /**
     * Can be used to hand in an initial shared context if GLFW is already setup.
     * Should be called before the GLFW module is registered.
     */
    static void provideExternalContext(GLFWwindow* sharedContext);
    static GLFWwindow* sharedContext();

    static ivec2 movePointOntoDesktop(ivec2 pos, ivec2 size);

    // CanvasGL override
    virtual size2_t getCanvasDimensions() const override;

    std::function<void(bool)> onVisibilityChange;
    std::function<void(ivec2)> onPositionChange;
    std::function<void(ivec2)> onWindowSizeChange;
    std::function<void(ivec2)> onFramebufferSizeChange;

protected:
    void setFullScreen(bool fullscreen);
    static CanvasGLFW* getCanvasGLFW(GLFWwindow*);
    static CanvasGLFW* getSharedContext();

    virtual void releaseContext() override;

private:
    static GLFWwindow* createWindow(const std::string& title, uvec2 dimensions);

    std::string windowTitle_;
    GLFWwindow* glWindow_;
    GLFWUserData userdata_;
    GLFWWindowEventManager eventManager_;

    bool isFullScreen_{false};
    ivec2 oldPos_{0};
    ivec2 oldSize_{256};

    static GLFWwindow* sharedContext_;
    static int glfwWindowCount_;
    static bool alwaysOnTop_;
};

}  // namespace inviwo
