/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_LOGGER_H
#define IVW_LOGGER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/singleton.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stringconversion.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <string>
#include <vector>

namespace inviwo {

enum class IVW_CORE_API LogLevel : int { Info, Warn, Error };
enum class IVW_CORE_API LogAudience : int { User, Developer };

#define LogInfo(message)                                                                          \
    {                                                                                             \
        std::ostringstream stream__;                                                              \
        stream__ << message;                                                                      \
        inviwo::LogCentral::getPtr()->log(parseTypeIdName(std::string(typeid(this).name())),      \
                                          inviwo::LogLevel::Info, inviwo::LogAudience::Developer, \
                                          __FILE__, __FUNCTION__, __LINE__, stream__.str());      \
    }
#define LogWarn(message)                                                                          \
    {                                                                                             \
        std::ostringstream stream__;                                                              \
        stream__ << message;                                                                      \
        inviwo::LogCentral::getPtr()->log(parseTypeIdName(std::string(typeid(this).name())),      \
                                          inviwo::LogLevel::Warn, inviwo::LogAudience::Developer, \
                                          __FILE__, __FUNCTION__, __LINE__, stream__.str());      \
    }
#define LogError(message)                                                                          \
    {                                                                                              \
        std::ostringstream stream__;                                                               \
        stream__ << message;                                                                       \
        inviwo::LogCentral::getPtr()->log(parseTypeIdName(std::string(typeid(this).name())),       \
                                          inviwo::LogLevel::Error, inviwo::LogAudience::Developer, \
                                          __FILE__, __FUNCTION__, __LINE__, stream__.str());       \
    }

#define LogInfoCustom(source, message)                                                            \
    {                                                                                             \
        std::ostringstream stream__;                                                              \
        stream__ << message;                                                                      \
        inviwo::LogCentral::getPtr()->log(source, inviwo::LogLevel::Info,                         \
                                          inviwo::LogAudience::Developer, __FILE__, __FUNCTION__, \
                                          __LINE__, stream__.str());                              \
    }
#define LogWarnCustom(source, message)                                                            \
    {                                                                                             \
        std::ostringstream stream__;                                                              \
        stream__ << message;                                                                      \
        inviwo::LogCentral::getPtr()->log(source, inviwo::LogLevel::Warn,                         \
                                          inviwo::LogAudience::Developer, __FILE__, __FUNCTION__, \
                                          __LINE__, stream__.str());                              \
    }
#define LogErrorCustom(source, message)                                                           \
    {                                                                                             \
        std::ostringstream stream__;                                                              \
        stream__ << message;                                                                      \
        inviwo::LogCentral::getPtr()->log(source, inviwo::LogLevel::Error,                        \
                                          inviwo::LogAudience::Developer, __FILE__, __FUNCTION__, \
                                          __LINE__, stream__.str());                              \
    }

#define LogProcessorInfo(message)                                                                 \
    {                                                                                             \
        std::ostringstream stream__;                                                              \
        stream__ << message;                                                                      \
        inviwo::LogCentral::getPtr()->logProcessor(this->getIdentifier(), inviwo::LogLevel::Info, \
                                                   inviwo::LogAudience::User, stream__.str(),     \
                                                   __FILE__, __FUNCTION__, __LINE__);             \
    }

#define LogProcessorWarn(message)                                                                 \
    {                                                                                             \
        std::ostringstream stream__;                                                              \
        stream__ << message;                                                                      \
        inviwo::LogCentral::getPtr()->logProcessor(this->getIdentifier(), inviwo::LogLevel::Warn, \
                                                   inviwo::LogAudience::User, stream__.str(),     \
                                                   __FILE__, __FUNCTION__, __LINE__);             \
    }

#define LogProcessorError(message)                                                                 \
    {                                                                                              \
        std::ostringstream stream__;                                                               \
        stream__ << message;                                                                       \
        inviwo::LogCentral::getPtr()->logProcessor(this->getIdentifier(), inviwo::LogLevel::Error, \
                                                   inviwo::LogAudience::User, stream__.str(),      \
                                                   __FILE__, __FUNCTION__, __LINE__);              \
    }

#define LogNetworkInfo(message)                                                             \
    {                                                                                       \
        std::ostringstream stream__;                                                        \
        stream__ << message;                                                                \
        inviwo::LogCentral::getPtr()->logNetwork(inviwo::LogLevel::Info,                    \
                                                 inviwo::LogAudience::User, stream__.str(), \
                                                 __FILE__, __FUNCTION__, __LINE__);         \
    }

#define LogNetworkWarn(message)                                                             \
    {                                                                                       \
        std::ostringstream stream__;                                                        \
        stream__ << message;                                                                \
        inviwo::LogCentral::getPtr()->logNetwork(inviwo::LogLevel::Warn,                    \
                                                 inviwo::LogAudience::User, stream__.str(), \
                                                 __FILE__, __FUNCTION__, __LINE__);         \
    }

#define LogNetworkError(message)                                                            \
    {                                                                                       \
        std::ostringstream stream__;                                                        \
        stream__ << message;                                                                \
        inviwo::LogCentral::getPtr()->logNetwork(inviwo::LogLevel::Error,                   \
                                                 inviwo::LogAudience::User, stream__.str(), \
                                                 __FILE__, __FUNCTION__, __LINE__);         \
    }

class IVW_CORE_API Logger {
public:
    Logger(){};
    virtual ~Logger(){};

    virtual void log(std::string logSource, LogLevel logLevel, LogAudience audience,
                     const char* fileName, const char* functionName, int lineNumber,
                     std::string logMsg) = 0;

    virtual void logProcessor(std::string processorIdentifier, LogLevel level, LogAudience audience,
                              std::string msg, const char* file, const char* function, int line);

    virtual void logNetwork(LogLevel level, LogAudience audience, std::string msg, const char* file,
                            const char* function, int line);
};

class IVW_CORE_API ConsoleLogger : public Logger {
public:
    ConsoleLogger();
    virtual ~ConsoleLogger();

    virtual void log(std::string logSource, LogLevel logLevel, LogAudience audience,
                     const char* fileName, const char* functionName, int lineNumber,
                     std::string logMsg) override;
};

class IVW_CORE_API FileLogger : public Logger {
public:
    FileLogger(std::string logPath);
    virtual ~FileLogger();

    virtual void log(std::string logSource, LogLevel logLevel, LogAudience audience,
                     const char* fileName, const char* functionName, int lineNumber,
                     std::string logMsg) override;

private:
    std::ofstream* fileStream_;
};

class IVW_CORE_API LogCentral : public Singleton<LogCentral> {
public:
    LogCentral();
    virtual ~LogCentral();

    void setLogLevel(LogLevel logLevel) { logLevel_ = logLevel; }
    LogLevel getLogLevel() { return logLevel_; }

    /**
     * \brief Register logger for use. LogCentral takes ownership of registered loggers
     * @param logger Logger to register.
     */
    void registerLogger(Logger* logger);
    /**
     * \brief Unregister and delete logger.
     * @param logger Logger to unregister
     */
    void unregisterLogger(Logger* logger);
    void log(std::string source, LogLevel level, LogAudience audience, const char* file,
             const char* function, int line, std::string msg);

    void logProcessor(std::string processorIdentifier, LogLevel level, LogAudience audience,
                      std::string msg, const char* file = "", const char* function = "",
                      int line = 0);

    void logNetwork(LogLevel level, LogAudience audience, std::string msg, const char* file = "",
                    const char* function = "", int line = 0);

    void setLogStacktrace(const bool& logStacktrace = true);
    bool getLogStacktrace() const;

private:
    LogLevel logLevel_;
    #include <warn/push>
    #include <warn/ignore/dll-interface>
    std::vector<Logger*> loggers_;
    #include <warn/pop>
    bool logStacktrace_;
};

namespace util {

IVW_CORE_API void log(ExceptionContext context, std::string message,
                      LogLevel level = LogLevel::Info,
                      LogAudience audience = LogAudience::Developer);

}  // namespace

}  // namespace

#endif  // IVW_LOGGER_H
