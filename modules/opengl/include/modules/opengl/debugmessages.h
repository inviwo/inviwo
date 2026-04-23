/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <modules/opengl/openglmoduledefine.h>

#include <inviwo/core/util/canvas.h>
#include <inviwo/core/util/logcentral.h>
#include <modules/opengl/inviwoopengl.h>
#include <inviwo/core/util/fmtutils.h>

namespace inviwo::utilgl {

namespace debug {

enum class Mode : std::uint8_t { Off, Debug, DebugSynchronous };
enum class BreakLevel : std::uint8_t { Off, High, Medium, Low, Notification };

enum class Source : std::uint8_t {
    Api,
    WindowSystem,
    ShaderCompiler,
    ThirdParty,
    Application,
    Other,
    DontCare
};

enum class Type : std::uint8_t {
    Error,
    DeprecatedBehavior,
    UndefinedBehavior,
    Portability,
    Performance,
    Marker,
    PushGroup,
    PopGroup,
    Other,
    DontCare
};

enum class Severity : std::uint8_t { Notification, Low, Medium, High, DontCare };

constexpr Source toSource(GLenum val) {
    switch (val) {
        case GL_DEBUG_SOURCE_API:
            return Source::Api;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return Source::WindowSystem;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return Source::ShaderCompiler;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return Source::ThirdParty;
        case GL_DEBUG_SOURCE_APPLICATION:
            return Source::Application;
        case GL_DEBUG_SOURCE_OTHER:
            return Source::Other;
        case GL_DONT_CARE:
            [[fallthrough]];
        default:
            return Source::DontCare;
    }
}

constexpr Type toType(GLenum val) {
    switch (val) {
        case GL_DEBUG_TYPE_ERROR:
            return Type::Error;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return Type::DeprecatedBehavior;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return Type::UndefinedBehavior;
        case GL_DEBUG_TYPE_PORTABILITY:
            return Type::Portability;
        case GL_DEBUG_TYPE_PERFORMANCE:
            return Type::Performance;
        case GL_DEBUG_TYPE_MARKER:
            return Type::Marker;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return Type::PushGroup;
        case GL_DEBUG_TYPE_POP_GROUP:
            return Type::PopGroup;
        case GL_DEBUG_TYPE_OTHER:
            return Type::Other;
        case GL_DONT_CARE:
            [[fallthrough]];
        default:
            return Type::DontCare;
    }
}

constexpr Severity toSeverity(GLenum val) {
    switch (val) {
        case GL_DEBUG_SEVERITY_HIGH:
            return Severity::High;
        case GL_DEBUG_SEVERITY_MEDIUM:
            return Severity::Medium;
        case GL_DEBUG_SEVERITY_LOW:
            return Severity::Low;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            return Severity::Notification;
        case GL_DONT_CARE:
            [[fallthrough]];
        default:
            return Severity::DontCare;
    }
}

constexpr GLenum toGL(Severity s) {
    switch (s) {
        case Severity::High:
            return GL_DEBUG_SEVERITY_HIGH;
        case Severity::Medium:
            return GL_DEBUG_SEVERITY_MEDIUM;
        case Severity::Low:
            return GL_DEBUG_SEVERITY_LOW;
        case Severity::Notification:
            return GL_DEBUG_SEVERITY_NOTIFICATION;
        case Severity::DontCare:
            [[fallthrough]];
        default:
            return GL_DONT_CARE;
    }
}

constexpr LogLevel toLogLevel(Severity s) {
    switch (s) {
        case Severity::High:
            return LogLevel::Error;
        case Severity::Medium:
            return LogLevel::Warn;
        case Severity::Low:
        case Severity::Notification:
        case Severity::DontCare:
            [[fallthrough]];
        default:
            return LogLevel::Info;
    }
}

namespace detail {

constexpr int toInt(Severity s) {
    switch (s) {
        case Severity::Notification:
            return 1;
        case Severity::Low:
            return 2;
        case Severity::Medium:
            return 3;
        case Severity::High:
            return 4;
        case Severity::DontCare:  // NOLINT(bugprone-branch-clone)
            return 0;
        default:
            return 0;
    }
}
constexpr int toInt(BreakLevel b) {
    switch (b) {
        case BreakLevel::Off:
            return 5;
        case BreakLevel::High:
            return 4;
        case BreakLevel::Medium:
            return 3;
        case BreakLevel::Low:
            return 2;
        case BreakLevel::Notification:
            return 1;
        default:
            return 0;
    }
}
}  // namespace detail

constexpr bool operator==(const Severity& s, const BreakLevel& b) {
    const int bi = detail::toInt(b);
    const int si = detail::toInt(s);
    return si == bi;
}
constexpr auto operator<=>(const Severity& s, const BreakLevel& b) {
    const int bi = detail::toInt(b);
    const int si = detail::toInt(s);
    return si <=> bi;
}
constexpr bool operator==(const BreakLevel& b, const Severity& s) { return s == b; }
constexpr auto operator<=>(const BreakLevel& b, const Severity& s) { return s <=> b; }

IVW_MODULE_OPENGL_API std::string_view format_as(Mode s);
IVW_MODULE_OPENGL_API std::string_view format_as(BreakLevel t);
IVW_MODULE_OPENGL_API std::string_view format_as(Source s);
IVW_MODULE_OPENGL_API std::string_view format_as(Type t);
IVW_MODULE_OPENGL_API std::string_view format_as(Severity s);
}  // namespace debug

IVW_MODULE_OPENGL_API void handleOpenGLDebugModeChange(debug::Mode mode, debug::Severity severity);
IVW_MODULE_OPENGL_API void handleOpenGLDebugMode(Canvas::ContextID context);
IVW_MODULE_OPENGL_API bool setOpenGLDebugMode(debug::Mode mode, debug::Severity severity);
IVW_MODULE_OPENGL_API void handleOpenGLDebugMessagesChange(utilgl::debug::Severity severity);
IVW_MODULE_OPENGL_API bool configureOpenGLDebugMessages(utilgl::debug::Severity severity);

/**
 * Enable or disable per-call glGetError() checking for the current OpenGL context.
 * When enabled, glGetError() is called after every OpenGL function call and errors are
 * logged to LogCentral. If @p breakOnError is true, util::debugBreak() is also called.
 */
IVW_MODULE_OPENGL_API void setOpenGLErrorChecking(bool enable, bool breakOnError, size_t stackSize);

/**
 * Apply OpenGL per-call error checking settings to all existing contexts.
 * Called when the corresponding OpenGLSettings properties change.
 */
IVW_MODULE_OPENGL_API void handleOpenGLErrorCheckingChange(bool enable, bool breakOnError, size_t stackSize);
}  // namespace inviwo::utilgl
