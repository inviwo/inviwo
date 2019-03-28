/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/ostreamjoiner.h>

#include <sstream>

#ifdef WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#endif

namespace inviwo {

ConsoleLogger::ConsoleLogger() = default;
ConsoleLogger::~ConsoleLogger() = default;

void ConsoleLogger::log(std::string logSource, LogLevel logLevel, LogAudience /*audience*/,
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
                return WORD{FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE};
            case LogLevel::Warn:
                return WORD{FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY};
            case LogLevel::Error:
                return WORD{FOREGROUND_RED | FOREGROUND_INTENSITY};
            default:
                return WORD{FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE};
        }
    }();

    CONSOLE_SCREEN_BUFFER_INFO oldState;
    GetConsoleScreenBufferInfo(hConsole, &oldState);

    FlushConsoleInputBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, k);

    const auto width = oldState.dwSize.X;

#else

    const std::string none{""};
    const std::string red{"\x1B[31m"};
    const std::string yellow{"\x1B[33m"};
    const std::string reset{"\x1B[0m"};

    switch (logLevel) {
        case LogLevel::Info:
            os << none;
            break;
        case LogLevel::Warn:
            os << yellow;
            break;
        case LogLevel::Error:
            os << red;
            break;
        default:
            os << none;
            break;
    }

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    const auto width = w.ws_col;
#endif

    const size_t reserved = 33;
    const auto maxWidth = width - reserved - 1;
    auto lines = splitString(logMsg, '\n');
    std::vector<std::string> res;
    for (auto line : lines) {
        if (line.size() < maxWidth) {
            res.push_back(line);
        } else {
            size_t pos = 0;
            while (pos < line.size()) {
                res.push_back(trim(line.substr(pos, maxWidth)));
                pos += maxWidth;
            }
        }
    }

    std::stringstream ss;
    auto joiner = util::make_ostream_joiner(ss, "\n" + std::string(reserved, ' '));
    std::copy(res.begin(), res.end(), joiner);
    logMsg = ss.str();

    os << std::left << std::setw(5) << logLevel << " " << std::setw(25) << logSource << ": "
       << logMsg << std::endl;

#ifdef WIN32
    SetConsoleTextAttribute(hConsole, oldState.wAttributes);
#else
    os << reset;
#endif
}

}  // namespace inviwo
