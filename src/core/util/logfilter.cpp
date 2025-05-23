/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/core/util/logfilter.h>

namespace inviwo {

LogFilter::LogFilter(Logger* logger) : LogFilter{logger, LogVerbosity::Info} {}

LogFilter::LogFilter(Logger* logger, LogVerbosity verbosity)
    : logVerbosity_{verbosity}, logger_{logger} {}

void LogFilter::setVerbosity(LogVerbosity verbosity) { logVerbosity_ = verbosity; }

LogVerbosity LogFilter::getVerbosity() { return logVerbosity_; }

void LogFilter::setLogger(Logger* logger) { logger_ = logger; }

Logger* LogFilter::getLogger() const { return logger_; }

void LogFilter::log(std::string_view logSource, LogLevel level, LogAudience audience,
                    std::string_view file, std::string_view function, int line,
                    std::string_view msg) {
    if (level >= logVerbosity_) {
        logger_->log(logSource, level, audience, file, function, line, std::move(msg));
    }
}

}  // namespace inviwo
