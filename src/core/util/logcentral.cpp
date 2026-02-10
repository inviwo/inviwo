/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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

#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/stacktrace.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>

#include <chrono>
#include <iostream>

#include <fmt/std.h>
#include <fmt/chrono.h>

namespace inviwo {

LogCentral::LogCentral() : logVerbosity_(LogVerbosity::Info), logStacktrace_(false) {}

void LogCentral::setVerbosity(LogVerbosity verbosity) {
    const std::scoped_lock lock{mutex_};
    logVerbosity_ = verbosity;
}

LogVerbosity LogCentral::getVerbosity() {
    const std::scoped_lock lock{mutex_};
    return logVerbosity_;
}

void LogCentral::setLocalVerbosity(LogVerbosity v) { localVerbosity() = v; }
LogVerbosity LogCentral::getLocalVerbosity() { return localVerbosity(); }

LogVerbosity& LogCentral::localVerbosity() {
    static thread_local LogVerbosity verbosity = LogVerbosity::Info;
    return verbosity;
}

void LogCentral::registerLogger(std::weak_ptr<Logger> logger) {
    const std::scoped_lock lock{mutex_};
    loggers_.push_back(logger);
}

void LogCentral::log(std::string_view source, LogLevel level, LogAudience audience,
                     std::string_view file, std::string_view function, int line,
                     std::string_view msg) {

    auto breakLevel = MessageBreakLevel::Off;
    {
        const std::scoped_lock lock{mutex_};
        fmt::memory_buffer buff;
        if (logStacktrace_ && level == LogLevel::Error && audience == LogAudience::Developer) {
            fmt::format_to(std::back_inserter(buff), "{}", msg);

            const auto stacktrace = getStackTrace();
            // start at i == 3 to remove log and getStacktrace from stack trace
            for (size_t i = 3; i < stacktrace.size(); ++i) {
                fmt::format_to(std::back_inserter(buff), "\n{}", stacktrace[i]);
            }
            // append an extra line break to easier separate several stack traces in a row
            fmt::format_to(std::back_inserter(buff), "\n");
            msg = std::string_view{buff.data(), buff.size()};
        }

        if (level >= std::max(logVerbosity_, localVerbosity())) {
            // use remove if here to remove expired weak pointers while calling the loggers.
            std::erase_if(loggers_, [&](const std::weak_ptr<Logger>& logger) {
                if (auto l = logger.lock()) {
                    l->log(source, level, audience, file, function, line, msg);
                    return false;
                } else {
                    return true;
                }
            });
        }

        breakLevel = breakLevel_;
    }

    switch (breakLevel) {
        case MessageBreakLevel::Off:
            break;
        case MessageBreakLevel::Error:
            if (level >= LogLevel::Error) util::debugBreak();
            break;
        case MessageBreakLevel::Warn:
            if (level >= LogLevel::Warn) util::debugBreak();
            break;
        case MessageBreakLevel::Info:
            if (level >= LogLevel::Info) util::debugBreak();
            break;
        default:
            break;
    }
}

void LogCentral::setLogStacktrace(const bool& logStacktrace) {
    const std::scoped_lock lock{mutex_};
    logStacktrace_ = logStacktrace;
}

bool LogCentral::getLogStacktrace() const {
    const std::scoped_lock lock{mutex_};
    return logStacktrace_;
}

void LogCentral::setMessageBreakLevel(MessageBreakLevel level) {
    const std::scoped_lock lock{mutex_};
    breakLevel_ = level;
}
MessageBreakLevel LogCentral::getMessageBreakLevel() const {
    const std::scoped_lock lock{mutex_};
    return breakLevel_;
}

LogCentral* LogCentral::instance_ = nullptr;

void util::log(SourceContext context, std::string_view message, LogLevel level,
               LogAudience audience) {
    util::log(LogCentral::getPtr(), context, message, level, audience);
}

void util::log(Logger* logger, SourceContext context, std::string_view message, LogLevel level,
               LogAudience audience) {
    logger->log(context.source(), level, audience, context.file(), context.function(),
                context.line(), message);
}

std::string_view enumToStr(LogLevel ll) {
    switch (ll) {
        case LogLevel::Info:
            return "Info";
        case LogLevel::Warn:
            return "Warn";
        case LogLevel::Error:
            return "Error";
    }
    throw Exception(SourceContext{}, "Found invalid LogLevel enum value '{}'",
                    static_cast<int>(ll));
}

std::string_view enumToStr(LogAudience la) {
    switch (la) {
        case LogAudience::User:
            return "User";
        case LogAudience::Developer:
            return "Developer";
    }
    throw Exception(SourceContext{}, "Found invalid LogAudience enum value '{}'",
                    static_cast<int>(la));
}

std::string_view enumToStr(MessageBreakLevel ll) {
    switch (ll) {
        case MessageBreakLevel::Info:
            return "Info";
        case MessageBreakLevel::Warn:
            return "Warn";
        case MessageBreakLevel::Error:
            return "Error";
        case MessageBreakLevel::Off:
            return "Off";
    }
    throw Exception(SourceContext{}, "Found invalid MessageBreakLevel enum value '{}'",
                    static_cast<int>(ll));
}

std::ostream& operator<<(std::ostream& ss, LogLevel ll) { return ss << enumToStr(ll); }
std::ostream& operator<<(std::ostream& ss, LogAudience la) { return ss << enumToStr(la); }
std::ostream& operator<<(std::ostream& ss, MessageBreakLevel ll) { return ss << enumToStr(ll); }

void log::detail::logDirectly(LogLevel level, SourceContext context, std::string_view message) {
    auto& os = level == LogLevel::Error ? std::cerr : std::cout;
    const auto time = std::chrono::system_clock::now();
    fmt::print(os, "{:%H:%M:06.3%S} {:5} {:35} {}:{}\n{}", time, level, context.source(),
               context.file(), context.line(), message);
}

log::SuppressLoggingLocal::SuppressLoggingLocal(LogVerbosity verbosity, LogCentral* lc)
    : lc_{lc}, old_{lc ? lc->getLocalVerbosity() : LogVerbosity::Info} {
    if (lc_) lc_->setLocalVerbosity(verbosity);
}

log::SuppressLoggingLocal::~SuppressLoggingLocal() {
    if (lc_) lc_->setLocalVerbosity(old_);
}

}  // namespace inviwo
