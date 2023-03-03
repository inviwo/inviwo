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

#pragma once

#include <modules/glfw/glfwmoduledefine.h>  // for IVW_MODULE_GLFW_API

#include <inviwo/core/util/canvas.h>              // for Canvas::ContextID, Canvas
#include <inviwo/core/util/glmvec.h>              // for ivec2, uvec2, size2_t
#include <modules/glfw/glfwuserdata.h>            // for GLFWUserData, GLFWwindow
#include <modules/glfw/glfwwindoweventmanager.h>  // for GLFWWindowEventManager
#include <modules/opengl/canvasgl.h>              // for CanvasGL

#include <functional>  // for function
#include <memory>      // for unique_ptr
#include <string>      // for string

typedef struct GLFWwindow GLFWwindow;

namespace inviwo {

/**
 * Helper class for handling the GLFW window and the associated rendering context. This class is
 * required by CanvasGLFW to ensure that the rendering context is still active while CanvasGL is
 * being destructed.
 * @see CanvasGLFW
 */
class IVW_MODULE_GLFW_API GLFWWindowHandler {
public:
    GLFWWindowHandler(Canvas* canvas, const std::string& title, uvec2 dimensions);
    ~GLFWWindowHandler();

    /**
     * Can be used to hand in an initial shared context if GLFW is already set up.
     * Should be called before the GLFW module is registered.
     */
    static void provideExternalContext(GLFWwindow* sharedContext);
    static GLFWwindow* sharedContext();

    static GLFWwindow* createWindow(const std::string& title, uvec2 dimensions);

    Canvas::ContextID getContextId() const { return static_cast<Canvas::ContextID>(glWindow_); }

    GLFWwindow* glWindow_;

private:
    static GLFWwindow* sharedContext_;
};

class IVW_MODULE_GLFW_API CanvasGLFW : public GLFWWindowHandler, public CanvasGL {
    friend class CanvasProcessorWidgetGLFW;

public:
    CanvasGLFW(const std::string& title = "", uvec2 dimensions = uvec2(128));
    virtual ~CanvasGLFW();

    virtual void update() override;
    virtual void activate() override;

    virtual std::unique_ptr<Canvas> createHiddenCanvas() override;
    virtual ContextID activeContext() const override;
    virtual ContextID contextId() const override;
    virtual void releaseContext() override;

    virtual void glSwapBuffers() override;
    virtual size2_t getCanvasDimensions() const override;

    void setVisible(bool visible);
    static int getVisibleWindowCount();

    void setWindowSize(ivec2);
    ivec2 getWindowSize() const;
    ivec2 getFramebufferSize() const;
    void setWindowPosition(ivec2);
    ivec2 getWindowPosition() const;

    void setWindowTitle(const std::string& windowTitle);

    void setOnTop(bool);

    static ivec2 movePointOntoDesktop(ivec2 pos, ivec2 size);

    std::function<void(bool)> onVisibilityChange;
    std::function<void(ivec2)> onPositionChange;
    std::function<void(ivec2)> onWindowSizeChange;
    std::function<void(ivec2)> onFramebufferSizeChange;

protected:
    void setFullScreen(bool fullscreen);
    static CanvasGLFW* getCanvasGLFW(GLFWwindow*);

private:
    GLFWUserData userdata_;
    GLFWWindowEventManager eventManager_;

    bool isFullScreen_{false};
    ivec2 oldPos_{0};
    ivec2 oldSize_{256};

    static int glfwWindowCount_;
    static bool alwaysOnTop_;
};

}  // namespace inviwo
