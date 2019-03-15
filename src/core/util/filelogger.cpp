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

#include <inviwo/core/util/filelogger.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo {

FileLogger::FileLogger(std::string logPath) : Logger() {
    if (filesystem::getFileExtension(logPath) != "") {
        fileStream_ = filesystem::ofstream(logPath);
    } else {
        fileStream_ = filesystem::ofstream(logPath.append("/inviwo-log.html"));
    }

    fileStream_ << "<style>" << std::endl
                << ".warn{" << std::endl
                << "    color: orange;" << std::endl
                << "}" << std::endl
                << ".error{" << std::endl
                << "    color: red;" << std::endl
                << "}" << std::endl
                << ".level{" << std::endl
                << "    font-weight: bold;" << std::endl
                << "}" << std::endl
                << "</style>" << std::endl;

    fileStream_ << "<div class ='info'>Inviwo (V " << IVW_VERSION << ") Log File</div>"
                << std::endl;

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    char formatedString[21];
    strftime(formatedString, sizeof(formatedString), "%Y-%m-%d %H:%M:%S ", &tm);
    fileStream_ << "<div class ='info'>Created at: " << formatedString << "</div>" << std::endl;
}

FileLogger::~FileLogger() = default;

void FileLogger::log(std::string logSource, LogLevel logLevel, LogAudience /*audience*/,
                     const char* fileName, const char* functionName, int lineNumber,
                     std::string logMsg) {
    IVW_UNUSED_PARAM(fileName);
    IVW_UNUSED_PARAM(logLevel);
    IVW_UNUSED_PARAM(functionName);
    IVW_UNUSED_PARAM(lineNumber);

    switch (logLevel) {
        case LogLevel::Info:
            fileStream_ << "<div class ='info'><span class='level'>Info: </span>";
            break;

        case LogLevel::Warn:
            fileStream_ << "<div class ='warn'><span class='level'>Warn: </span>";
            break;

        case LogLevel::Error:
            fileStream_ << "<div class ='error'><span class='level'>Error: </span>";
            break;
    }

    logMsg = htmlEncode(logMsg);
    replaceInString(logMsg, " ", "&nbsp;");
    replaceInString(logMsg, "\n", "<br/>");
    replaceInString(logMsg, "\t", "&nbsp;&nbsp;&nbsp;&nbsp;");

    fileStream_ << "(" << htmlEncode(logSource) << ":" << lineNumber << ") " << logMsg;
    fileStream_ << "</div>" << std::endl;
}

}  // namespace inviwo
