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

#include <iosfwd>

namespace inviwo {

namespace utilgl {

namespace debug {

enum class Mode { Off, Debug, DebugSynchronous };
enum class BreakLevel { Off, High, Medium, Low, Notification };

enum class Source : unsigned int {
    Api = static_cast<unsigned int>(GL_DEBUG_SOURCE_API),
    WindowSystem = static_cast<unsigned int>(GL_DEBUG_SOURCE_WINDOW_SYSTEM),
    ShaderCompiler = static_cast<unsigned int>(GL_DEBUG_SOURCE_SHADER_COMPILER),
    ThirdParty = static_cast<unsigned int>(GL_DEBUG_SOURCE_THIRD_PARTY),
    Application = static_cast<unsigned int>(GL_DEBUG_SOURCE_APPLICATION),
    Other = static_cast<unsigned int>(GL_DEBUG_SOURCE_OTHER),
    DontCare = static_cast<unsigned int>(GL_DONT_CARE)
};

enum class Type : unsigned int {
    Error = static_cast<unsigned int>(GL_DEBUG_TYPE_ERROR),
    DeprecatedBehavior = static_cast<unsigned int>(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR),
    UndefinedBehavior = static_cast<unsigned int>(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR),
    Portability = static_cast<unsigned int>(GL_DEBUG_TYPE_PORTABILITY),
    Performance = static_cast<unsigned int>(GL_DEBUG_TYPE_PERFORMANCE),
    Marker = static_cast<unsigned int>(GL_DEBUG_TYPE_MARKER),
    PushGroup = static_cast<unsigned int>(GL_DEBUG_TYPE_PUSH_GROUP),
    PopGroup = static_cast<unsigned int>(GL_DEBUG_TYPE_POP_GROUP),
    Other = static_cast<unsigned int>(GL_DEBUG_TYPE_OTHER),
    DontCare = static_cast<unsigned int>(GL_DONT_CARE)
};

enum class Severity : unsigned int {
    Notification = static_cast<unsigned int>(GL_DEBUG_SEVERITY_NOTIFICATION),
    Low = static_cast<unsigned int>(GL_DEBUG_SEVERITY_LOW),
    Medium = static_cast<unsigned int>(GL_DEBUG_SEVERITY_MEDIUM),
    High = static_cast<unsigned int>(GL_DEBUG_SEVERITY_HIGH),
    DontCare = static_cast<unsigned int>(GL_DONT_CARE)
};

inline Source toSouce(GLenum val) { return static_cast<Source>(static_cast<unsigned int>(val)); }

inline Type toType(GLenum val) { return static_cast<Type>(static_cast<unsigned int>(val)); }

inline Severity toSeverity(GLenum val) { return static_cast<Severity>(static_cast<unsigned int>(val)); }

inline LogLevel toLogLevel(Severity s) {
    switch (s) {
        case Severity::High:
            return LogLevel::Error;
        case Severity::Medium:
            return LogLevel::Warn;
        case Severity::Low:
        case Severity::Notification:
        case Severity::DontCare:
        default:
            return LogLevel::Info;
    }
}

namespace detail {

inline int toInt(Severity s) {
    switch (s) {
        case Severity::Notification:
            return 1;
        case Severity::Low:
            return 2;
        case Severity::Medium:
            return 3;
        case Severity::High:
            return 4;
        case Severity::DontCare:
            return 0;
        default:
            return 0;
    }
}
inline int toInt(BreakLevel b) {
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

inline bool operator==(const Severity& s, const BreakLevel& b) {
    const int bi = detail::toInt(b);
    const int si = detail::toInt(s);
    return si == bi;
}
inline bool operator!=(const Severity& s, const BreakLevel& b) { return !(s == b); }

inline bool operator<(const Severity& s, const BreakLevel& b) {
    const int bi = detail::toInt(b);
    const int si = detail::toInt(s);
    return si < bi;
}
inline bool operator<=(const Severity& s, const BreakLevel& b) { return s < b || s == b; }
inline bool operator>(const Severity& s, const BreakLevel& b) { return !(s < b); }
inline bool operator>=(const Severity& s, const BreakLevel& b) { return s > b || s == b; }

inline bool operator==(const BreakLevel& b, const Severity& s) { return s == b; }
inline bool operator!=(const BreakLevel& b, const Severity& s) { return s != b; }
inline bool operator<(const BreakLevel& b, const Severity& s) { return s > b; }
inline bool operator<=(const BreakLevel& b, const Severity& s) { return s >= b; }
inline bool operator>(const BreakLevel& b, const Severity& s) { return s < b; }
inline bool operator>=(const BreakLevel& b, const Severity& s) { return s <= b; }

IVW_MODULE_OPENGL_API std::string_view enumToStr(Mode s);
IVW_MODULE_OPENGL_API std::string_view enumToStr(BreakLevel t);
IVW_MODULE_OPENGL_API std::string_view enumToStr(Source s);
IVW_MODULE_OPENGL_API std::string_view enumToStr(Type t);
IVW_MODULE_OPENGL_API std::string_view enumToStr(Severity s);

IVW_MODULE_OPENGL_API std::ostream& operator<<(std::ostream& ss, Mode m);
IVW_MODULE_OPENGL_API std::ostream& operator<<(std::ostream& ss, BreakLevel b);
IVW_MODULE_OPENGL_API std::ostream& operator<<(std::ostream& ss, Source s);

IVW_MODULE_OPENGL_API std::ostream& operator<<(std::ostream& ss, Type t);

IVW_MODULE_OPENGL_API std::ostream& operator<<(std::ostream& ss, Severity s);

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
IVW_MODULE_OPENGL_API void setOpenGLErrorChecking(bool enable, bool breakOnError);

/**
 * Apply OpenGL per-call error checking settings to all existing contexts.
 * Called when the corresponding OpenGLSettings properties change.
 */
IVW_MODULE_OPENGL_API void handleOpenGLErrorCheckingChange(bool enable, bool breakOnError);
}  // namespace utilgl

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::utilgl::debug::Mode>
    : inviwo::FlagFormatter<inviwo::utilgl::debug::Mode> {};
template <>
struct fmt::formatter<inviwo::utilgl::debug::BreakLevel>
    : inviwo::FlagFormatter<inviwo::utilgl::debug::BreakLevel> {};

template <>
struct fmt::formatter<inviwo::utilgl::debug::Source>
    : inviwo::FlagFormatter<inviwo::utilgl::debug::Source> {};
template <>
struct fmt::formatter<inviwo::utilgl::debug::Type>
    : inviwo::FlagFormatter<inviwo::utilgl::debug::Type> {};

template <>
struct fmt::formatter<inviwo::utilgl::debug::Severity>
    : inviwo::FlagFormatter<inviwo::utilgl::debug::Severity> {};
#endif
