/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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

#include <inviwo/core/util/filelogger.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/inviwocommondefines.h>

namespace inviwo {

FileLogger::FileLogger(std::string logPath) : Logger() {
    if (filesystem::getFileExtension(logPath) != "") {
        fileStream_ = filesystem::ofstream(logPath);
    } else {
        std::string_view header = R"(<style>
.warn {
    color: orange;
}
.error {
    color: red;
}
.level {
    font-weight: bold;
}
.nowrap {
    white-space: pre-wrap;
    tab-size: 4;
}
</style>
)";

        fileStream_ = filesystem::ofstream(logPath.append("/inviwo-log.html"));
        fileStream_ << header;
    }

    fileStream_ << "<div class='info'>Inviwo (v" << build::version << ") Log File</div>"
                << std::endl;

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    char formatedString[21];
    strftime(formatedString, sizeof(formatedString), "%Y-%m-%d %H:%M:%S ", &tm);
    fileStream_ << "<div class='info nowrap'>Created at: " << formatedString << "</div>"
                << std::endl;
}

FileLogger::~FileLogger() = default;

void FileLogger::log(std::string_view logSource, LogLevel logLevel, LogAudience /*audience*/,
                     [[maybe_unused]] std::string_view fileName,
                     [[maybe_unused]] std::string_view functionName,
                     [[maybe_unused]] int lineNumber, std::string_view logMsg) {

    switch (logLevel) {
        case LogLevel::Info:
            fileStream_ << "<div class='info nowrap'><span class='level'>Info: </span>";
            break;

        case LogLevel::Warn:
            fileStream_ << "<div class='warn nowrap'><span class='level'>Warn: </span>";
            break;

        case LogLevel::Error:
            fileStream_ << "<div class='error nowrap'><span class='level'>Error: </span>";
            break;
    }

    auto msg = util::htmlEncode(logMsg);
    replaceInString(msg, "\n", "<br/>");

    fileStream_ << "(" << util::htmlEncode(logSource) << ":" << lineNumber << ") " << msg;
    fileStream_ << "</div>" << std::endl;
}

}  // namespace inviwo
