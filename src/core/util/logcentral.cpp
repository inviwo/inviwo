/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>

namespace inviwo {

bool operator==(const LogLevel& lhs, const LogVerbosity& rhs) {
    return static_cast<LogVerbosity>(lhs) == rhs;
}

bool operator!=(const LogLevel& lhs, const LogVerbosity& rhs) { return !operator==(lhs, rhs); }

bool operator<(const LogLevel& lhs, const LogVerbosity& rhs) {
    return static_cast<LogVerbosity>(lhs) < rhs;
}

bool operator>(const LogLevel& lhs, const LogVerbosity& rhs) { return rhs < lhs; }

bool operator<=(const LogLevel& lhs, const LogVerbosity& rhs) { return !(rhs < lhs); }

bool operator>=(const LogLevel& lhs, const LogVerbosity& rhs) { return !(lhs < rhs); }

bool operator==(const LogVerbosity& lhs, const LogLevel& rhs) {
    return lhs == static_cast<LogVerbosity>(rhs);
}

bool operator!=(const LogVerbosity& lhs, const LogLevel& rhs) { return !operator==(lhs, rhs); }

bool operator<(const LogVerbosity& lhs, const LogLevel& rhs) {
    return lhs < static_cast<LogVerbosity>(rhs);
}

bool operator>(const LogVerbosity& lhs, const LogLevel& rhs) { return rhs < lhs; }

bool operator<=(const LogVerbosity& lhs, const LogLevel& rhs) { return !(rhs < lhs); }

bool operator>=(const LogVerbosity& lhs, const LogLevel& rhs) { return !(lhs < rhs); }

void Logger::logProcessor(Processor* processor, LogLevel level, LogAudience audience,
                          std::string msg, const char* file, const char* function, int line) {
    log("Processor " + processor->getIdentifier(), level, audience, file, function, line, msg);
}

void Logger::logNetwork(LogLevel level, LogAudience audience, std::string msg, const char* file,
                        const char* function, int line) {
    log("ProcessorNetwork", level, audience, file, function, line, msg);
}

void Logger::logAssertion(const char* file, const char* function, int line, std::string msg) {
    log("Assertion failed", LogLevel::Error, LogAudience::Developer, file, function, line, msg);
}

LogCentral::LogCentral() : logVerbosity_(LogVerbosity::Info), logStacktrace_(false) {}

void LogCentral::setVerbosity(LogVerbosity verbosity) { logVerbosity_ = verbosity; }

LogVerbosity LogCentral::getVerbosity() { return logVerbosity_; }

void LogCentral::registerLogger(std::weak_ptr<Logger> logger) { loggers_.push_back(logger); }

void LogCentral::log(std::string source, LogLevel level, LogAudience audience, const char* file,
                     const char* function, int line, std::string msg) {
    if (logStacktrace_ && level == LogLevel::Error && audience == LogAudience::Developer) {
        std::stringstream ss;
        ss << msg;

        std::vector<std::string> stacktrace = getStackTrace();
        // start at i == 3 to remove log and getStacktrace from stackgrace
        for (size_t i = 3; i < stacktrace.size(); ++i) {
            ss << std::endl << stacktrace[i];
        }
        // append an extra line break to easier separate several stacktraces in a row
        ss << std::endl;

        msg = ss.str();
    }

    if (level >= logVerbosity_) {
        // use remove if here to remove expired weak pointers while calling the loggers.
        util::erase_remove_if(loggers_, [&](const std::weak_ptr<Logger>& logger) {
            if (auto l = logger.lock()) {
                l->log(source, level, audience, file, function, line, msg);
                return false;
            } else {
                return true;
            }
        });
    }

    switch (breakLevel_) {
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

void LogCentral::logProcessor(Processor* processor, LogLevel level, LogAudience audience,
                              std::string msg, const char* file, const char* function, int line) {
    if (level >= logVerbosity_) {
        // use remove if here to remove expired weak pointers while calling the loggers.
        util::erase_remove_if(loggers_, [&](const std::weak_ptr<Logger>& logger) {
            if (auto l = logger.lock()) {
                l->logProcessor(processor, level, audience, msg, file, function, line);
                return false;
            } else {
                return true;
            }
        });
    }
}

void LogCentral::logNetwork(LogLevel level, LogAudience audience, std::string msg, const char* file,
                            const char* function, int line) {
    if (level >= logVerbosity_) {
        // use remove if here to remove expired weak pointers while calling the loggers.
        util::erase_remove_if(loggers_, [&](const std::weak_ptr<Logger>& logger) {
            if (auto l = logger.lock()) {
                l->logNetwork(level, audience, msg, file, function, line);
                return false;
            } else {
                return true;
            }
        });
    }
}

void LogCentral::logAssertion(const char* file, const char* function, int line, std::string msg) {
    util::erase_remove_if(loggers_, [&](const std::weak_ptr<Logger>& logger) {
        if (auto l = logger.lock()) {
            l->logAssertion(file, function, line, msg);
            return false;
        } else {
            return true;
        }
    });
}

void LogCentral::setLogStacktrace(const bool& logStacktrace) { logStacktrace_ = logStacktrace; }

bool LogCentral::getLogStacktrace() const { return logStacktrace_; }

void LogCentral::setMessageBreakLevel(MessageBreakLevel level) { breakLevel_ = level; }
MessageBreakLevel LogCentral::getMessageBreakLevel() const { return breakLevel_; }

LogCentral* LogCentral::instance_ = nullptr;

void util::log(ExceptionContext context, std::string message, LogLevel level,
               LogAudience audience) {
    util::log(LogCentral::getPtr(), context, message, level, audience);
}

void util::log(Logger* logger, ExceptionContext context, std::string message, LogLevel level,
               LogAudience audience) {
    logger->log(context.getCaller(), level, audience, context.getFile().c_str(),
                context.getFunction().c_str(), context.getLine(), message);
}

}  // namespace inviwo
