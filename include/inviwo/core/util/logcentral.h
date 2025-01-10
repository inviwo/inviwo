/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/singleton.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/fmtutils.h>
#include <inviwo/core/util/demangle.h>

#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>

namespace inviwo {

class Processor;

enum class LogLevel : int { Info, Warn, Error };
enum class LogVerbosity : int { Info, Warn, Error, None };
enum class LogAudience : int { User, Developer };
enum class MessageBreakLevel : int { Off, Error, Warn, Info };

IVW_CORE_API bool operator==(const LogLevel& lhs, const LogVerbosity& rhs);
IVW_CORE_API bool operator!=(const LogLevel& lhs, const LogVerbosity& rhs);
IVW_CORE_API bool operator<(const LogLevel& lhs, const LogVerbosity& rhs);
IVW_CORE_API bool operator>(const LogLevel& lhs, const LogVerbosity& rhs);
IVW_CORE_API bool operator<=(const LogLevel& lhs, const LogVerbosity& rhs);
IVW_CORE_API bool operator>=(const LogLevel& lhs, const LogVerbosity& rhs);

IVW_CORE_API bool operator==(const LogVerbosity& lhs, const LogLevel& rhs);
IVW_CORE_API bool operator!=(const LogVerbosity& lhs, const LogLevel& rhs);
IVW_CORE_API bool operator<(const LogVerbosity& lhs, const LogLevel& rhs);
IVW_CORE_API bool operator>(const LogVerbosity& lhs, const LogLevel& rhs);
IVW_CORE_API bool operator<=(const LogVerbosity& lhs, const LogLevel& rhs);
IVW_CORE_API bool operator>=(const LogVerbosity& lhs, const LogLevel& rhs);

IVW_CORE_API std::string_view enumToStr(LogLevel ll);
IVW_CORE_API std::string_view enumToStr(LogAudience la);
IVW_CORE_API std::string_view enumToStr(MessageBreakLevel ll);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, LogLevel ll);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, LogAudience la);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, MessageBreakLevel ll);

#define LogSpecial(logger, logLevel, message)                                                   \
    {                                                                                           \
        std::ostringstream stream__;                                                            \
        stream__ << message;                                                                    \
        logger->log(                                                                            \
            inviwo::util::parseTypeIdName(typeid(std::remove_const_t<decltype(*this)>).name()), \
            logLevel, inviwo::LogAudience::Developer, __FILE__, __FUNCTION__, __LINE__,         \
            stream__.str());                                                                    \
    }

#define LogCustomSpecial(logger, logLevel, source, message)                                   \
    {                                                                                         \
        std::ostringstream stream__;                                                          \
        stream__ << message;                                                                  \
        logger->log(source, logLevel, inviwo::LogAudience::Developer, __FILE__, __FUNCTION__, \
                    __LINE__, stream__.str());                                                \
    }

#define LogInfo(message) {LogSpecial(inviwo::LogCentral::getPtr(), inviwo::LogLevel::Info, message)}
#define LogWarn(message) {LogSpecial(inviwo::LogCentral::getPtr(), inviwo::LogLevel::Warn, message)}
#define LogError(message) \
    {LogSpecial(inviwo::LogCentral::getPtr(), inviwo::LogLevel::Error, message)}

#define LogInfoCustom(source, message) \
    {LogCustomSpecial(inviwo::LogCentral::getPtr(), inviwo::LogLevel::Info, source, message)}
#define LogWarnCustom(source, message) \
    {LogCustomSpecial(inviwo::LogCentral::getPtr(), inviwo::LogLevel::Warn, source, message)}
#define LogErrorCustom(source, message) \
    {LogCustomSpecial(inviwo::LogCentral::getPtr(), inviwo::LogLevel::Error, source, message)}

class IVW_CORE_API Logger {
public:
    Logger() = default;
    virtual ~Logger() = default;

    virtual void log(std::string_view logSource, LogLevel logLevel, LogAudience audience,
                     std::string_view file, std::string_view function, int line,
                     std::string_view msg) = 0;
};

class IVW_CORE_API LogCentral : public Singleton<LogCentral>, public Logger {
public:
    LogCentral();
    virtual ~LogCentral() = default;

    void setVerbosity(LogVerbosity verbosity);
    LogVerbosity getVerbosity();

    /**
     * \brief Register logger for use. LogCentral does not take ownership
     * of registered loggers.
     * @param logger Logger to register.
     */
    void registerLogger(std::weak_ptr<Logger> logger);

    virtual void log(std::string_view source, LogLevel level, LogAudience audience,
                     std::string_view file, std::string_view function, int line,
                     std::string_view msg) override;

    void setLogStacktrace(const bool& logStacktrace = true);
    bool getLogStacktrace() const;

    void setMessageBreakLevel(MessageBreakLevel level);
    MessageBreakLevel getMessageBreakLevel() const;

private:
    friend Singleton<LogCentral>;
    static LogCentral* instance_;

    LogVerbosity logVerbosity_;
#include <warn/push>
#include <warn/ignore/dll-interface>
    std::vector<std::weak_ptr<Logger>> loggers_;
#include <warn/pop>
    bool logStacktrace_ = false;
    MessageBreakLevel breakLevel_ = MessageBreakLevel::Off;
};

namespace log {

/**
 * All log functions either take an explicit SourceContext argument,
 * or automatically extract one from the call site.
 * All functions that take a fmt::format_string do compile time format checks.
 *
 * # Basic logging
 * * `info(fmt::format_string<Args...>, Args&&...)`
 * * `warn(fmt::format_string<Args...>, Args&&...)`
 * * `error(fmt::format_string<Args...>, Args&&...)`
 *
 * # Runtime Log Level
 * * `report(LogLevel, SourceContext, fmt::format_string, Args&&...)`
 * * `report(LogLevel, SourceContext, std::string_view)`
 * * `report(LogLevel, std::string_view)`
 * * `message(LogLevel level, fmt::format_string<Args...>, Args&&...)`

 * # Log Exceptions
 * * `exception(const Exception&)`
 * * `exception(const std::exception&)`
 * * `exception(std::string_view)`
 * * `exception()`
 *
 * # Log to a custom logger
 * * `report(Logger&, LogLevel, SourceContext, fmt::format_string, Args&&...)`
 * * `report(Logger&, LogLevel, SourceContext, std::string_view)
 * * `report(Logger&, LogLevel, std::string_view)`
 * * `message(Logger&, LogLevel level, fmt::format_string<Args...>, Args&&...)`
 */

namespace detail {
IVW_CORE_API void logDirectly(LogLevel level, SourceContext context, std::string_view message);
}

inline void report(Logger& logger, LogLevel level, SourceContext context,
                   std::string_view message) {
    logger.log(context.source(), level, LogAudience::User, context.file(), context.function(),
               context.line(), message);
}
inline void report(LogLevel level, SourceContext context, std::string_view message) {
    if (LogCentral::isInitialized()) {
        ::inviwo::log::report(*LogCentral::getPtr(), level, context, message);
    } else {
        ::inviwo::log::detail::logDirectly(level, context, message);
    }
}

inline void report(Logger& logger, LogLevel level, std::string_view message,
                   SourceContext context = std::source_location::current()) {
    ::inviwo::log::report(logger, level, context, message);
}

inline void report(LogLevel level, std::string_view message,
                   SourceContext context = std::source_location::current()) {
    ::inviwo::log::report(level, context, message);
}

namespace detail {
inline void report(LogLevel level, SourceContext context, fmt::string_view format,
                   fmt::format_args&& args) {
    ::inviwo::log::report(level, context, fmt::vformat(format, args));
}
inline void report(Logger& logger, LogLevel level, SourceContext context, fmt::string_view format,
                   fmt::format_args&& args) {
    ::inviwo::log::report(logger, level, context, fmt::vformat(format, args));
}
}  // namespace detail

template <typename... Args>
inline void report(Logger& logger, LogLevel level, SourceContext context,
                   fmt::format_string<Args...> format, Args&&... args) {
    ::inviwo::log::detail::report(logger, level, context, format, fmt::make_format_args(args...));
}
template <typename... Args>
inline void report(LogLevel level, SourceContext context, fmt::format_string<Args...> format,
                   Args&&... args) {
    ::inviwo::log::detail::report(level, context, format, fmt::make_format_args(args...));
}

inline void exception(const Exception& e) {
    ::inviwo::log::report(LogLevel::Error, e.getContext(), e.getFullMessage());
}
template <typename... Args>
inline void exception(const Exception& e, fmt::format_string<Args...> format, Args&&... args) {
    ::inviwo::log::detail::report(LogLevel::Warn, e.getContext(), format,
                                  fmt::make_format_args(args...));
}
inline void exception(const std::exception& e,
                      SourceContext context = std::source_location::current()) {
    ::inviwo::log::report(LogLevel::Error, context, e.what());
}
inline void exception(std::string_view message,
                      SourceContext context = std::source_location::current()) {
    ::inviwo::log::report(LogLevel::Error, context, message);
}
inline void exception(SourceContext context = std::source_location::current()) {
    ::inviwo::log::report(LogLevel::Error, context, "Unknown Exception");
}

template <typename... Args>
struct message {
    message(LogLevel level, fmt::format_string<Args...> format, Args&&... args,
            SourceContext context = std::source_location::current()) {
        ::inviwo::log::detail::report(level, context, format, fmt::make_format_args(args...));
    }
    message(Logger& logger, LogLevel level, fmt::format_string<Args...> format, Args&&... args,
            SourceContext context = std::source_location::current()) {
        ::inviwo::log::detail::report(logger, level, context, format,
                                      fmt::make_format_args(args...));
    }
};
template <typename... Args>
message(LogLevel level, fmt::format_string<Args...>, Args&&...) -> message<Args...>;
template <typename... Args>
message(Logger& logger, LogLevel level, fmt::format_string<Args...>, Args&&...) -> message<Args...>;

template <typename... Args>
struct info {
    explicit info(fmt::format_string<Args...> format, Args&&... args,
                  SourceContext context = std::source_location::current()) {
        ::inviwo::log::detail::report(LogLevel::Info, context, format,
                                      fmt::make_format_args(args...));
    }
};
template <typename... Args>
info(fmt::format_string<Args...>, Args&&...) -> info<Args...>;

template <typename... Args>
struct warn {
    explicit warn(fmt::format_string<Args...> format, Args&&... args,
                  SourceContext context = std::source_location::current()) {
        ::inviwo::log::detail::report(LogLevel::Warn, context, format,
                                      fmt::make_format_args(args...));
    }
};
template <typename... Args>
warn(fmt::format_string<Args...>, Args&&...) -> warn<Args...>;

template <typename... Args>
struct error {
    explicit error(fmt::format_string<Args...> format, Args&&... args,
                   SourceContext context = std::source_location::current()) {
        ::inviwo::log::detail::report(LogLevel::Error, context, format,
                                      fmt::make_format_args(args...));
    }
};
template <typename... Args>
error(fmt::format_string<Args...>, Args&&...) -> error<Args...>;

}  // namespace log

namespace util {

IVW_CORE_API void log(SourceContext context, std::string_view message,
                      LogLevel level = LogLevel::Info,
                      LogAudience audience = LogAudience::Developer);

IVW_CORE_API void log(Logger* logger, SourceContext context, std::string_view message,
                      LogLevel level = LogLevel::Info,
                      LogAudience audience = LogAudience::Developer);

template <typename... Args>
void log(SourceContext context, LogLevel level, LogAudience audience,
         fmt::format_string<Args...> format, Args&&... args) {
    LogCentral::getPtr()->log(context.source(), level, audience, context.file(), context.function(),
                              context.line(), fmt::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
void logInfo(SourceContext context, fmt::format_string<Args...> format, Args&&... args) {
    LogCentral::getPtr()->log(context.source(), LogLevel::Info, LogAudience::Developer,
                              context.file(), context.function(), context.line(),
                              fmt::format(format, std::forward<Args>(args)...));
}
template <typename... Args>
void logWarn(SourceContext context, fmt::format_string<Args...> format, Args&&... args) {
    LogCentral::getPtr()->log(context.source(), LogLevel::Warn, LogAudience::Developer,
                              context.file(), context.function(), context.line(),
                              fmt::format(format, std::forward<Args>(args)...));
}
template <typename... Args>
void logError(SourceContext context, fmt::format_string<Args...> format, Args&&... args) {
    LogCentral::getPtr()->log(context.source(), LogLevel::Error, LogAudience::Developer,
                              context.file(), context.function(), context.line(),
                              fmt::format(format, std::forward<Args>(args)...));
}

}  // namespace util

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::LogLevel> : inviwo::FlagFormatter<inviwo::LogLevel> {};
template <>
struct fmt::formatter<inviwo::LogAudience> : inviwo::FlagFormatter<inviwo::LogAudience> {};
template <>
struct fmt::formatter<inviwo::MessageBreakLevel>
    : inviwo::FlagFormatter<inviwo::MessageBreakLevel> {};
#endif
