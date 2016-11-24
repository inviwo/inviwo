/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/opengl/debugmessages.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <modules/opengl/openglsettings.h>
#include <inviwo/core/util/rendercontext.h>

namespace inviwo {

namespace utilgl {

void logDebugMode(OpenGLDebugMode mode, Canvas::ContextID context) {
    switch (mode) {
        case OpenGLDebugMode::Off:
            LogInfoCustom("OpenGL Debug", "Debugging off for context: " << context);
            break;
        case OpenGLDebugMode::Debug:
            LogInfoCustom("OpenGL Debug", "Debugging active for context: " << context);
            break;
        case OpenGLDebugMode::DebugSynchronous:
            LogInfoCustom("OpenGL Debug", "Synchronous debugging active for context: " << context);
            break;
    }
}

void APIENTRY openGLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                         GLsizei length, const GLchar* message,
                                         const void* module) {

    std::stringstream ss;
    ss << "message: " << message << "\n";
    ss << "type: ";
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            ss << "ERROR";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            ss << "DEPRECATED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            ss << "UNDEFINED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            ss << "PORTABILITY";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            ss << "PERFORMANCE";
            break;
        case GL_DEBUG_TYPE_OTHER:
            ss << "OTHER";
            break;
    }
    ss << "\n";

    ss << "source: ";
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            ss << "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            ss << "WINDOW_SUSTEM";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            ss << "SHADER COMPILER";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            ss << "THIRD PARTY";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            ss << "APPLICATION";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            ss << "OTHER";
            break;
    }
    ss << "\n";

    ss << "id: " << id << "\n";
    ss << "severity: ";
    LogLevel level = LogLevel::Info;
    switch (severity) {
        case GL_DEBUG_SEVERITY_LOW:
            ss << "LOW";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            ss << "MEDIUM";
            level = LogLevel::Warn;
            break;
        case GL_DEBUG_SEVERITY_HIGH:
            ss << "HIGH";
            level = LogLevel::Error;
            break;
    }

    std::string error = ss.str();
    LogCentral::getPtr()->log("OpenGL Debug", level, LogAudience::Developer, __FILE__, __FUNCTION__,
                              __LINE__, error);

    auto openglSettings = InviwoApplication::getPtr()->getSettingsByType<OpenGLSettings>();
    auto mode = openglSettings->debugMessages_.getSelectedValue();
    if (mode == OpenGLDebugMode::DebugSynchronous) {
        auto debugbreak = openglSettings->breakOnMessage_.getSelectedValue();

        
        switch (debugbreak) {
            case OpenGLSettings::BreakOnMessageLevel::Off: 
                break;
            case OpenGLSettings::BreakOnMessageLevel::Error:
                if (level == LogLevel::Error) util::debugBreak();
                break;
            case OpenGLSettings::BreakOnMessageLevel::Warn:
                if (level == LogLevel::Error || level == LogLevel::Warn) util::debugBreak();
                break;
            case OpenGLSettings::BreakOnMessageLevel::Info:
                util::debugBreak();
                break;
        }
    }
}

void handleOpenGLDebugModeChange(OpenGLDebugMode mode) {
    if (RenderContext::getPtr()->getDefaultRenderContext()) {
        RenderContext::getPtr()->activateDefaultRenderContext();
        setOpenGLDebugMode(mode);
        logDebugMode(mode, RenderContext::getPtr()->activeContext());
        LogInfoCustom(
            "OpenGL Debug",
            "To enable/disable debugging on all contexts inviwo might need to be restarted");
    }
}

void setOpenGLDebugMode(OpenGLDebugMode mode) {
    if (glDebugMessageCallback) {
        if (mode == OpenGLDebugMode::Off) {
            glDisable(GL_DEBUG_OUTPUT);
        } else if (mode == OpenGLDebugMode::Debug) {
            glEnable(GL_DEBUG_OUTPUT);
            glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(openGLDebugMessageCallback, nullptr);
            GLuint unusedIds = 0;
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
        } else if (mode == OpenGLDebugMode::DebugSynchronous) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(openGLDebugMessageCallback, nullptr);
            GLuint unusedIds = 0;
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
        }
    }
}

void handleOpenGLDebugMode(Canvas::ContextID context) {
    auto openglSettings = InviwoApplication::getPtr()->getSettingsByType<OpenGLSettings>();
    auto mode = openglSettings->debugMessages_.getSelectedValue();
    setOpenGLDebugMode(mode);
    logDebugMode(mode, context);
}

} // namespace

} // namespace
