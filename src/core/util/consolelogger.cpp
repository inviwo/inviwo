/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <inviwo/core/util/consolelogger.h>

#ifdef WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace inviwo {

ConsoleLogger::ConsoleLogger() = default;
ConsoleLogger::~ConsoleLogger() = default;

void ConsoleLogger::log(std::string logSource, LogLevel logLevel, LogAudience audience,
                        const char* fileName, const char* functionName, int lineNumber,
                        std::string logMsg) {
    IVW_UNUSED_PARAM(fileName);
    IVW_UNUSED_PARAM(logLevel);
    IVW_UNUSED_PARAM(functionName);
    IVW_UNUSED_PARAM(lineNumber);

    auto& os = logLevel == LogLevel::Error ? std::cerr : std::cout;

    #ifdef WIN32
    const auto h = logLevel == LogLevel::Error ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE;
    HANDLE hConsole = GetStdHandle(h);
    const auto k = [&]() {
        switch (logLevel) {
            case LogLevel::Info:
                return 15;
            case LogLevel::Warn:
                return 14;
            case LogLevel::Error:
                return 12;
            default:
                return 15;
        }
    }();
    FlushConsoleInputBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, k);

    os << std::setw(5) << logLevel << " (" << std::setw(25) << logSource << ") " << logMsg
        << std::endl;

    #else

    std::string red{ "\x1B[31m" };
    std::string yellow{ "\x1B[33m" };
    std::string reset{ "\x1B[0m" };

    const auto color = [&]() {
        switch (logLevel) {
            case LogLevel::Info:
                return "";
            case LogLevel::Warn:
                return yellow;
            case LogLevel::Error:
                return red;
            default:
                return "";
        }
    }();

    os << color << std::setw(5) << logLevel << " (" << std::setw(25) << logSource << ") " << logMsg
       << reset << std::endl;

    #endif
}

} // namespace

