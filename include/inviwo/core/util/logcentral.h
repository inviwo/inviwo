/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#define LogProcessorSpecial(logger, logLevel, message)                                            \
    {                                                                                             \
        std::ostringstream stream__;                                                              \
        stream__ << message;                                                                      \
        logger->logProcessor(this, logLevel, inviwo::LogAudience::User, stream__.str(), __FILE__, \
                             __FUNCTION__, __LINE__);                                             \
    }

#define LogNetworkSpecial(logger, logLevel, message)                                      \
    {                                                                                     \
        std::ostringstream stream__;                                                      \
        stream__ << message;                                                              \
        logger->logNetwork(logLevel, inviwo::LogAudience::User, stream__.str(), __FILE__, \
                           __FUNCTION__, __LINE__);                                       \
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

#define LogProcessorInfo(message) \
    {LogProcessorSpecial(inviwo::LogCentral::getPtr(), inviwo::LogLevel::Info, message)}
#define LogProcessorWarn(message) \
    {LogProcessorSpecial(inviwo::LogCentral::getPtr(), inviwo::LogLevel::Warn, message)}
#define LogProcessorError(message) \
    {LogProcessorSpecial(inviwo::LogCentral::getPtr(), inviwo::LogLevel::Error, message)}

#define LogNetworkInfo(message) \
    {LogNetworkSpecial(inviwo::LogCentral::getPtr(), inviwo::LogLevel::Info, message)}
#define LogNetworkWarn(message) \
    {LogNetworkSpecial(inviwo::LogCentral::getPtr(), inviwo::LogLevel::Warn, message)}
#define LogNetworkError(message) \
    {LogNetworkSpecial(inviwo::LogCentral::getPtr(), inviwo::LogLevel::Error, message)}

class IVW_CORE_API Logger {
public:
    Logger() = default;
    virtual ~Logger() = default;

    virtual void log(std::string_view logSource, LogLevel logLevel, LogAudience audience,
                     std::string_view file, std::string_view function, int line,
                     std::string_view msg) = 0;

    virtual void logProcessor(Processor* processor, LogLevel level, LogAudience audience,
                              std::string_view msg, std::string_view file,
                              std::string_view function, int line);

    virtual void logNetwork(LogLevel level, LogAudience audience, std::string_view msg,
                            std::string_view file, std::string_view function, int line);

    virtual void logAssertion(std::string_view file, std::string_view function, int line,
                              std::string_view msg);
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

    virtual void logProcessor(Processor* processor, LogLevel level, LogAudience audience,
                              std::string_view msg, std::string_view file = "",
                              std::string_view function = "", int line = 0) override;

    virtual void logNetwork(LogLevel level, LogAudience audience, std::string_view msg,
                            std::string_view file = "", std::string_view function = "",
                            int line = 0) override;

    virtual void logAssertion(std::string_view file, std::string_view function, int line,
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
 * All log function either take a explicit SourceContext argument,
 * or automatically extracts one from the call site.
 * All functions that takes a fmt::format_string does compile time format checks
 *
 * * `report(LogLevel, SourceContext, fmt::format_string, Args&&...)
 * * `report(LogLevel, SourceContext, std::string_view)
 * * `report(LogLevel, std::string_view)
 * * `exception(const Exception&)
 * * `message(LogLevel level, fmt::format_string<Args...>, Args&&...)
 * * `info(fmt::format_string<Args...>, Args&&...)
 * * warn(fmt::format_string<Args...>, Args&&...)
 * * error(fmt::format_string<Args...>, Args&&...)
 *
 */

inline void report(LogLevel level, SourceContext context, std::string_view message) {
    if (LogCentral::isInitialized()) {
        LogCentral::getPtr()->log(context.source(), level, LogAudience::User, context.file(),
                                  context.function(), context.line(), message);
    } else {
        // TODO log directly to std::cout here?
    }
}

inline void report(LogLevel level, std::string_view message,
                   SourceContext context = std::source_location::current()) {
    ::inviwo::log::report(level, context, message);
}

inline void implementation(LogLevel level, SourceContext context, fmt::string_view format,
                           fmt::format_args&& args) {
    ::inviwo::log::report(level, context, fmt::vformat(format, std::move(args)));
}

template <typename... Args>
inline void report(LogLevel level, SourceContext context, fmt::format_string<Args...> format,
                   Args&&... args) {
    ::inviwo::log::implementation(level, context, format, fmt::make_format_args(args...));
}

inline void exception(const Exception& e) {
    ::inviwo::log::report(LogLevel::Error, e.getContext(), e.getFullMessage());
}
template <typename... Args>
inline void exception(const Exception& e, fmt::format_string<Args...> format, Args&&... args) {
    ::inviwo::log::implementation(LogLevel::Warn, e.getContext(), format,
                                  fmt::make_format_args(args...));
}
inline void exception(const std::exception& e,
                      SourceContext context = std::source_location::current()) {
    ::inviwo::log::report(LogLevel::Error, context, e.what());
}
inline void exception(std::string_view message = "Unknown Exception",
                      SourceContext context = std::source_location::current()) {
    ::inviwo::log::report(LogLevel::Error, context, message);
}

template <typename... Args>
struct message {
    message(LogLevel level, fmt::format_string<Args...> format, Args&&... args,
            SourceContext context = std::source_location::current()) {
        ::inviwo::log::implementation(level, context, format, fmt::make_format_args(args...));
    }
};
template <typename... Args>
message(LogLevel level, fmt::format_string<Args...>, Args&&...) -> message<Args...>;

template <typename... Args>
struct info {
    info(fmt::format_string<Args...> format, Args&&... args,
         SourceContext context = std::source_location::current()) {
        ::inviwo::log::implementation(LogLevel::Info, context, format,
                                      fmt::make_format_args(args...));
    }
};
template <typename... Args>
info(fmt::format_string<Args...>, Args&&...) -> info<Args...>;

template <typename... Args>
struct warn {
    warn(fmt::format_string<Args...> format, Args&&... args,
         SourceContext context = std::source_location::current()) {
        ::inviwo::log::implementation(LogLevel::Warn, context, format,
                                      fmt::make_format_args(args...));
    }
};
template <typename... Args>
warn(fmt::format_string<Args...>, Args&&...) -> warn<Args...>;

template <typename... Args>
struct error {
    error(fmt::format_string<Args...> format, Args&&... args,
          SourceContext context = std::source_location::current()) {
        ::inviwo::log::implementation(LogLevel::Error, context, format,
                                      fmt::make_format_args(args...));
    }
};
template <typename... Args>
error(fmt::format_string<Args...>, Args&&...) -> error<Args...>;

}  // namespace log

namespace util {

IVW_CORE_API void log(ExceptionContext context, std::string_view message,
                      LogLevel level = LogLevel::Info,
                      LogAudience audience = LogAudience::Developer);

IVW_CORE_API void log(Logger* logger, ExceptionContext context, std::string_view message,
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
