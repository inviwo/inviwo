/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <iosfwd>

namespace inviwo {

namespace utilgl {

namespace debug {

enum class Mode { Off, Debug, DebugSynchronous };
enum class BreakLevel { Off, High, Medium, Low, Notification };

enum class Source : GLenum {
    Api = GL_DEBUG_SOURCE_API,
    WindowSystem = GL_DEBUG_SOURCE_WINDOW_SYSTEM,
    ShaderCompiler = GL_DEBUG_SOURCE_SHADER_COMPILER,
    ThirdParty = GL_DEBUG_SOURCE_THIRD_PARTY,
    Application = GL_DEBUG_SOURCE_APPLICATION,
    Other = GL_DEBUG_SOURCE_OTHER,
    DontCare = GL_DONT_CARE
};

enum class Type : GLenum {
    Error = GL_DEBUG_TYPE_ERROR,
    DeprecatedBehavior = GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR,
    UndefinedBehavior = GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,
    Portability = GL_DEBUG_TYPE_PORTABILITY,
    Performance = GL_DEBUG_TYPE_PERFORMANCE,
    Marker = GL_DEBUG_TYPE_MARKER,
    PushGroup = GL_DEBUG_TYPE_PUSH_GROUP,
    PopGroup = GL_DEBUG_TYPE_POP_GROUP,
    Other = GL_DEBUG_TYPE_OTHER,
    DontCare = GL_DONT_CARE
};

enum class Severity : GLenum {
    Notification = GL_DEBUG_SEVERITY_NOTIFICATION,
    Low = GL_DEBUG_SEVERITY_LOW,
    Medium = GL_DEBUG_SEVERITY_MEDIUM,
    High = GL_DEBUG_SEVERITY_HIGH,
    DontCare = GL_DONT_CARE
};

inline Source toSouce(GLenum val) { return static_cast<Source>(val); }

inline Type toType(GLenum val) { return static_cast<Type>(val); }

inline Severity toSeverity(GLenum val) { return static_cast<Severity>(val); }

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

IVW_MODULE_OPENGL_API std::ostream& operator<<(std::ostream& ss, Mode m);

IVW_MODULE_OPENGL_API std::ostream& operator<<(std::ostream& ss, BreakLevel b);

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

IVW_MODULE_OPENGL_API std::ostream& operator<<(std::ostream& ss, Source s);

IVW_MODULE_OPENGL_API std::ostream& operator<<(std::ostream& ss, Type t);

IVW_MODULE_OPENGL_API std::ostream& operator<<(std::ostream& ss, Severity s);

}  // namespace debug

IVW_MODULE_OPENGL_API void handleOpenGLDebugModeChange(debug::Mode mode, debug::Severity severity);
IVW_MODULE_OPENGL_API void handleOpenGLDebugMode(Canvas::ContextID context);
IVW_MODULE_OPENGL_API bool setOpenGLDebugMode(debug::Mode mode, debug::Severity severity);
IVW_MODULE_OPENGL_API void handleOpenGLDebugMessagesChange(utilgl::debug::Severity severity);
IVW_MODULE_OPENGL_API bool configureOpenGLDebugMessages(utilgl::debug::Severity severity);
}  // namespace utilgl

}  // namespace inviwo
