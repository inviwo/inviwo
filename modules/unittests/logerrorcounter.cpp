/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "logerrorcounter.h"

namespace inviwo {

LogErrorCounter::LogErrorCounter() {}

LogErrorCounter::~LogErrorCounter() {}

void LogErrorCounter::log(std::string logSource, LogLevel logLevel, LogAudience audience,
                          const char* fileName, const char* functionName, int lineNumber,
                          std::string logMsg) {
    messageCount_[static_cast<LogLevel>(logLevel)]++;
}

size_t LogErrorCounter::getCount(const LogLevel& level) const {
    std::map<LogLevel, size_t>::const_iterator it = messageCount_.find(level);
    if (it == messageCount_.end()) return 0;
    return it->second;
}

size_t LogErrorCounter::getInfoCount() const { return getCount(LogLevel::Info); }

size_t LogErrorCounter::getWarnCount() const { return getCount(LogLevel::Warn); }

size_t LogErrorCounter::getErrorCount() const { return getCount(LogLevel::Error); }

}  // namespace
