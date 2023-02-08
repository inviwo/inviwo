/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>   // for InviwoApplication
#include <inviwo/core/properties/optionproperty.h>  // for OptionProperty
#include <inviwo/core/util/assertion.h>             // for debugBreak
#include <inviwo/core/util/canvas.h>                // for Canvas, Canvas::ContextID
#include <inviwo/core/util/logcentral.h>            // for LogCentral, LogInfoCustom, LogAudience
#include <inviwo/core/util/rendercontext.h>         // for RenderContext, ContextHolder
#include <modules/opengl/openglsettings.h>          // for OpenGLSettings

#include <ostream>        // for operator<<, basic_ostream
#include <string>         // for char_traits, string
#include <thread>         // for get_id, operator==, __thread_id, thread
#include <type_traits>    // for __underlying_type_impl<>::type, under...
#include <unordered_map>  // for unordered_map

namespace inviwo {

namespace utilgl {

void logDebugMode(debug::Mode mode, debug::Severity severity, Canvas::ContextID context) {
    const auto rc = RenderContext::getPtr();
    switch (mode) {
        case debug::Mode::Off:
            LogInfoCustom("OpenGL Debug",
                          "Debugging off for context: " << rc->getContextName(context) << " ("
                                                        << context << ")");
            break;
        case debug::Mode::Debug:
            LogInfoCustom("OpenGL Debug", "Debugging active for context: "
                                              << rc->getContextName(context) << " (" << context
                                              << ")  at level: " << severity);
            break;
        case debug::Mode::DebugSynchronous:
            LogInfoCustom("OpenGL Debug", "Synchronous debugging active for context: "
                                              << rc->getContextName(context) << " (" << context
                                              << ") at level: " << severity);
            break;
    }
}

extern "C" {
static void GLAPIENTRY openGLDebugMessageCallback(GLenum esource, GLenum etype, GLuint id,
                                                  GLenum eseverity, GLsizei /*length*/,
                                                  const GLchar* message, const void* /*module*/) {

    const auto source = debug::toSouce(esource);
    const auto type = debug::toType(etype);
    const auto severity = debug::toSeverity(eseverity);

    std::stringstream ss;
    ss << message << "\n";
    ss << "[Severity: " << severity;
    ss << ", Type: " << type;
    ss << ", Source: " << source;
    ss << ", Id: " << id;
    if (const auto rc = RenderContext::getPtr()) {
        const auto context = rc->activeContext();
        ss << ", Context:  " << rc->getContextName(context) << " (" << context << ")";
    }
    ss << "]";

    std::string error = ss.str();
    LogCentral::getPtr()->log("OpenGL Debug", toLogLevel(severity), LogAudience::Developer,
                              __FILE__, __FUNCTION__, __LINE__, error);

    if (auto app = InviwoApplication::getPtr()) {
        if (auto openglSettings = app->getSettingsByType<OpenGLSettings>()) {
            auto mode = openglSettings->debugMessages_.getSelectedValue();
            if (mode == debug::Mode::DebugSynchronous) {
                const auto debugbreak = openglSettings->breakOnMessage_.getSelectedValue();
                if (severity >= debugbreak) util::debugBreak();
            }
        }
    }
}
}

void handleOpenGLDebugModeChange(debug::Mode mode, debug::Severity severity) {
    if (RenderContext::getPtr()->hasDefaultRenderContext()) {
        RenderContext::getPtr()->forEachContext(
            [mode, severity](Canvas::ContextID id, const std::string& /*name*/,
                             ContextHolder* canvas, std::thread::id threadId) {
                if (threadId == std::this_thread::get_id()) {
                    canvas->activate();
                    if (setOpenGLDebugMode(mode, severity)) {
                        logDebugMode(mode, severity, id);
                    } else {
                        LogInfoCustom("OpenGL Debug", "Debug messages not supported");
                    }
                }
            });

        RenderContext::getPtr()->activateDefaultRenderContext();
    }
}

bool setOpenGLDebugMode(debug::Mode mode, debug::Severity severity) {
    if (!glDebugMessageCallback) return false;

    switch (mode) {
        case debug::Mode::Off:
            glDisable(GL_DEBUG_OUTPUT);
            glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            break;
        case debug::Mode::Debug:
            glEnable(GL_DEBUG_OUTPUT);
            glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(openGLDebugMessageCallback, nullptr);
            configureOpenGLDebugMessages(severity);
            break;
        case debug::Mode::DebugSynchronous:
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(openGLDebugMessageCallback, nullptr);
            configureOpenGLDebugMessages(severity);
            break;
        default:
            break;
    }

    return true;
}

void handleOpenGLDebugMessagesChange(utilgl::debug::Severity severity) {
    if (RenderContext::getPtr()->hasDefaultRenderContext()) {
        RenderContext::getPtr()->forEachContext(
            [severity](Canvas::ContextID id, const std::string& /*name*/, ContextHolder* canvas,
                       std::thread::id threadId) {
                if (threadId == std::this_thread::get_id()) {
                    const auto rc = RenderContext::getPtr();
                    canvas->activate();
                    if (configureOpenGLDebugMessages(severity)) {
                        LogInfoCustom("OpenGL Debug",
                                      "Debug messages for level: " << severity << " for context: "
                                                                   << rc->getContextName(id) << " ("
                                                                   << id << ")");
                    } else {
                        LogInfoCustom("OpenGL Debug", "Debug messages not supported");
                    }
                }
            });
        RenderContext::getPtr()->activateDefaultRenderContext();
    }
}

bool configureOpenGLDebugMessages(utilgl::debug::Severity severity) {
    if (!glDebugMessageControl) return false;

    using namespace debug;

    auto set = [](bool n, bool l, bool m, bool h) {
        auto g = [](Severity s) { return static_cast<std::underlying_type<Severity>::type>(s); };
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, g(Severity::Notification), 0, nullptr, n);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, g(Severity::Low), 0, nullptr, l);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, g(Severity::Medium), 0, nullptr, m);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, g(Severity::High), 0, nullptr, h);
    };

    switch (severity) {
        case Severity::Notification:
            set(true, true, true, true);
            break;
        case Severity::Low:
            set(false, true, true, true);
            break;
        case Severity::Medium:
            set(false, false, true, true);
            break;
        case Severity::High:
            set(false, false, false, true);
            break;
        case Severity::DontCare: {
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
            break;
        }
        default:
            break;
    }

    return true;
}

void handleOpenGLDebugMode(Canvas::ContextID context) {
    auto openglSettings = InviwoApplication::getPtr()->getSettingsByType<OpenGLSettings>();
    auto mode = openglSettings->debugMessages_.getSelectedValue();
    auto severity = openglSettings->debugSeverity_.getSelectedValue();
    setOpenGLDebugMode(mode, severity);
    if (mode != debug::Mode::Off) {
        logDebugMode(mode, severity, context);
    }
}

namespace debug {

std::ostream& operator<<(std::ostream& ss, Mode m) {
    switch (m) {
        case Mode::Off:
            ss << "Off";
            break;
        case Mode::Debug:
            ss << "Debug";
            break;
        case Mode::DebugSynchronous:
            ss << "DebugSynchronous";
            break;
        default:
            break;
    }
    return ss;
}
std::ostream& operator<<(std::ostream& ss, BreakLevel b) {
    switch (b) {
        case BreakLevel::Off:
            ss << "Off";
            break;
        case BreakLevel::High:
            ss << "High";
            break;
        case BreakLevel::Medium:
            ss << "Medium";
            break;
        case BreakLevel::Low:
            ss << "Low";
            break;
        case BreakLevel::Notification:
            ss << "Notification";
            break;
        default:
            break;
    }
    return ss;
}
std::ostream& operator<<(std::ostream& ss, Source s) {
    switch (s) {
        case Source::Api:
            ss << "Api";
            break;
        case Source::WindowSystem:
            ss << "WindowSystem";
            break;
        case Source::ShaderCompiler:
            ss << "ShaderCompiler";
            break;
        case Source::ThirdParty:
            ss << "ThirdParty";
            break;
        case Source::Application:
            ss << "Application";
            break;
        case Source::Other:
            ss << "Other";
            break;
        case Source::DontCare:
            ss << "DontCare";
            break;
        default:
            break;
    }
    return ss;
}
std::ostream& operator<<(std::ostream& ss, Type t) {
    switch (t) {
        case Type::Error:
            ss << "Error";
            break;
        case Type::DeprecatedBehavior:
            ss << "DeprecatedBehavior";
            break;
        case Type::UndefinedBehavior:
            ss << "UndefinedBehavior";
            break;
        case Type::Portability:
            ss << "Portability";
            break;
        case Type::Performance:
            ss << "Performance";
            break;
        case Type::Marker:
            ss << "Marker";
            break;
        case Type::PushGroup:
            ss << "PushGroup";
            break;
        case Type::PopGroup:
            ss << "PopGroup";
            break;
        case Type::Other:
            ss << "Other";
            break;
        case Type::DontCare:
            ss << "DontCare";
            break;
        default:
            break;
    }
    return ss;
}
std::ostream& operator<<(std::ostream& ss, Severity s) {
    switch (s) {
        case Severity::Low:
            ss << "Low";
            break;
        case Severity::Medium:
            ss << "Medium";
            break;
        case Severity::High:
            ss << "High";
            break;
        case Severity::Notification:
            ss << "Notification";
            break;
        case Severity::DontCare:
            ss << "DontCare";
            break;
        default:
            break;
    }
    return ss;
}

}  // namespace debug

}  // namespace utilgl

}  // namespace inviwo
