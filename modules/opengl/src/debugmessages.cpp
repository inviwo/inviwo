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

#include <modules/opengl/debugmessages.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/canvas.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/stacktrace.h>
#include <modules/opengl/openglsettings.h>

#include <fmt/base.h>

#include <glbinding/Binding.h>
#include <glbinding/CallbackMask.h>
#include <glbinding/FunctionCall.h>
#include <glbinding-aux/types_to_string.h>

#include <ostream>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <ranges>

#ifdef __cpp_lib_stacktrace
#include <stacktrace>
#endif


namespace inviwo::utilgl {

void logDebugMode(debug::Mode mode, debug::Severity severity, Canvas::ContextID context) {
    const auto rc = RenderContext::getPtr();
    switch (mode) {
        case debug::Mode::Off:
            log::report(LogLevel::Info, SourceContext("OpenGL Debug"_sl),
                        "Debugging off for context: {} ({})", rc->getContextName(context), context);
            break;
        case debug::Mode::Debug:
            log::report(LogLevel::Info, SourceContext("OpenGL Debug"_sl),
                        "Debugging active for context: {} ({}) at level: {}",
                        rc->getContextName(context), context, severity);
            break;
        case debug::Mode::DebugSynchronous:
            log::report(LogLevel::Info, SourceContext("OpenGL Debug"_sl),
                        "Synchronous debugging active for context: {} ({}) at level: {}",
                        rc->getContextName(context), context, severity);
            break;
    }
}

extern "C" {
static void APIENTRY openGLDebugMessageCallback(GLenum esource, GLenum etype, GLuint id,
                                                GLenum eseverity, GLsizei /*length*/,
                                                const GLchar* message, const void* /*module*/) {

    const auto source = debug::toSource(esource);
    const auto type = debug::toType(etype);
    const auto severity = debug::toSeverity(eseverity);

    std::string error = fmt::format("{}\n[Severity: {}. Type: {}, Source: {}, Id: {}", message,
                                    severity, type, source, id);
    if (const auto* rc = RenderContext::getPtr()) {
        const auto* context = rc->activeContext();
        fmt::format_to(std::back_inserter(error), "Context: {} ({})", rc->getContextName(context),
                       context);
    }
    fmt::format_to(std::back_inserter(error), "]");

    log::report(toLogLevel(severity), error);

    if (InviwoApplication::isInitialized()) {
        if (auto* openglSettings =
                InviwoApplication::getPtr()->getSettingsByType<OpenGLSettings>()) {
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
                        log::report(LogLevel::Info, SourceContext("OpenGL Debug"_sl),
                                    "Debug messages not supported");
                    }
                }
            });

        RenderContext::getPtr()->activateDefaultRenderContext();
    }
}

bool setOpenGLDebugMode(debug::Mode mode, debug::Severity severity) {
    if (!glbinding::Binding::DebugMessageCallback.isResolved()) return false;

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
                    const auto* rc = RenderContext::getPtr();
                    canvas->activate();
                    if (configureOpenGLDebugMessages(severity)) {
                        log::report(LogLevel::Info, SourceContext("OpenGL Debug"_sl),
                                    "Debug messages for level: {} for context: {} ({})", severity,
                                    rc->getContextName(id), id);
                    } else {
                        log::report(LogLevel::Info, SourceContext("OpenGL Debug"_sl),
                                    "Debug messages not supported");
                    }
                }
            });
        RenderContext::getPtr()->activateDefaultRenderContext();
    }
}

bool configureOpenGLDebugMessages(utilgl::debug::Severity severity) {
    if (!glbinding::Binding::DebugMessageControl.isResolved()) return false;

    using enum debug::Severity;

    auto set = [](bool n, bool l, bool m, bool h) {
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, toGL(Notification), 0, nullptr, n);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, toGL(Low), 0, nullptr, l);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, toGL(Medium), 0, nullptr, m);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, toGL(High), 0, nullptr, h);
    };

    switch (severity) {
        case Notification:
            set(true, true, true, true);
            break;
        case Low:
            set(false, true, true, true);
            break;
        case Medium:
            set(false, false, true, true);
            break;
        case High:
            set(false, false, false, true);
            break;
        case DontCare: {
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
            break;
        }
        default:
            break;
    }

    return true;
}

void handleOpenGLDebugMode(Canvas::ContextID context) {
    auto* openglSettings = InviwoApplication::getPtr()->getSettingsByType<OpenGLSettings>();
    auto mode = openglSettings->debugMessages_.getSelectedValue();
    auto severity = openglSettings->debugSeverity_.getSelectedValue();
    setOpenGLDebugMode(mode, severity);
    if (mode != debug::Mode::Off) {
        logDebugMode(mode, severity, context);
    }
    // Apply error checking setting for this new context
    setOpenGLErrorChecking(openglSettings->errorChecking_.get(),
                           openglSettings->breakOnError_.get());
}

void handleOpenGLErrorCheckingChange(bool enable, bool breakOnError) {
    if (RenderContext::getPtr()->hasDefaultRenderContext()) {
        RenderContext::getPtr()->forEachContext(
            [enable, breakOnError](Canvas::ContextID /*id*/, const std::string& /*name*/,
                                   ContextHolder* canvas, std::thread::id threadId) {
                if (threadId == std::this_thread::get_id()) {
                    canvas->activate();
                    setOpenGLErrorChecking(enable, breakOnError);
                }
            });
        RenderContext::getPtr()->activateDefaultRenderContext();
    }
}

namespace {
std::string argsToString(const glbinding::FunctionCall& call) {
    std::stringstream ss;
    for (unsigned i = 0; i < call.parameters.size(); ++i) {
        ss << call.parameters[i].get();
        if (i < call.parameters.size() - 1) ss << ", ";
    }
    return std::move(ss).str();
}

void logStackTrace(size_t count) {
#ifdef __cpp_lib_stacktrace

    auto stack = std::stacktrace::current();
    auto items = stack | std::views::drop_while([](auto& entry) {
                     return !entry.description().contains("glbinding");
                 }) |
                 std::views::drop_while(
                     [](auto& entry) { return entry.description().contains("glbinding"); }) |
                 std::views::take(count);

    for (auto&& entry : items) {
        log::report(LogLevel::Error, SourceContext("OpenGL Error Check"_sl), "{}({}): {}",
                    entry.source_file(), entry.source_line(), entry.description());
    }
#endif
}

}  // namespace

void setOpenGLErrorChecking(bool enable, bool breakOnError) {
    if (enable) {
        // Install the global after callback. When settings change, this is called again to
        // reinstall with the updated breakOnError value.
        glbinding::Binding::setAfterCallback([breakOnError](const glbinding::FunctionCall& call) {
            GLenum err{GL_NO_ERROR};
            while ((err = glGetError()) != GL_NO_ERROR) {

                log::report(LogLevel::Error, SourceContext("OpenGL Error Check"_sl),
                            "OpenGL error {} after calling {}({})", getGLErrorString(err),
                            call.function->name(), argsToString(call));
                logStackTrace(5);

                if (breakOnError) util::debugBreak();
            }
        });
        // Enable the After callback for all functions in this context except glGetError itself
        glbinding::Binding::addCallbackMaskExcept(
            glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue,
            {"glGetError"});
    } else {
        glbinding::Binding::removeCallbackMask(glbinding::CallbackMask::After |
                                               glbinding::CallbackMask::ParametersAndReturnValue);
    }
}

namespace debug {

std::string_view format_as(Mode m) {
    switch (m) {
        case Mode::Off:
            return "Off";
        case Mode::Debug:
            return "Debug";
        case Mode::DebugSynchronous:
            return "DebugSynchronous";
        default:
            return "";
    }
}
std::string_view format_as(BreakLevel b) {
    switch (b) {
        case BreakLevel::Off:
            return "Off";
        case BreakLevel::High:
            return "High";
        case BreakLevel::Medium:
            return "Medium";
        case BreakLevel::Low:
            return "Low";
        case BreakLevel::Notification:
            return "Notification";
        default:
            return "";
    }
}
std::string_view format_as(Source s) {
    switch (s) {
        case Source::Api:
            return "Api";
        case Source::WindowSystem:
            return "WindowSystem";
        case Source::ShaderCompiler:
            return "ShaderCompiler";
        case Source::ThirdParty:
            return "ThirdParty";
        case Source::Application:
            return "Application";
        case Source::Other:
            return "Other";
        case Source::DontCare:
            return "DontCare";
        default:
            return "";
    }
}
std::string_view format_as(Type t) {
    switch (t) {
        case Type::Error:
            return "Error";
        case Type::DeprecatedBehavior:
            return "DeprecatedBehavior";
        case Type::UndefinedBehavior:
            return "UndefinedBehavior";
        case Type::Portability:
            return "Portability";
        case Type::Performance:
            return "Performance";
        case Type::Marker:
            return "Marker";
        case Type::PushGroup:
            return "PushGroup";
        case Type::PopGroup:
            return "PopGroup";
        case Type::Other:
            return "Other";
        case Type::DontCare:
            return "DontCare";
        default:
            return "";
    }
}
std::string_view format_as(Severity s) {
    switch (s) {
        case Severity::Low:
            return "Low";
        case Severity::Medium:
            return "Medium";
        case Severity::High:
            return "High";
        case Severity::Notification:
            return "Notification";
        case Severity::DontCare:
            return "DontCare";
        default:
            return "";
    }
}

}  // namespace debug

}  // namespace inviwo::utilgl
