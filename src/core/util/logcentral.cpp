/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/stacktrace.h>
#include <inviwo/core/util/filesystem.h>


namespace inviwo {

ConsoleLogger::ConsoleLogger() : Logger() {}
ConsoleLogger::~ConsoleLogger() {}

void ConsoleLogger::log(std::string logSource, unsigned int logLevel, const char* fileName,
                        const char* functionName, int lineNumber, std::string logMsg) {
    IVW_UNUSED_PARAM(fileName);
    IVW_UNUSED_PARAM(logLevel);
    IVW_UNUSED_PARAM(functionName);
    IVW_UNUSED_PARAM(lineNumber);
    std::cout << "(" << logSource << ") " << logMsg << std::endl;
}

FileLogger::FileLogger(std::string logPath)
    : Logger()
{
    if (filesystem::getFileExtension(logPath) != ""){
        fileStream_ = new std::ofstream(logPath.c_str());
    }
    else{
        fileStream_ = new std::ofstream(logPath.append("/inviwo-log.html").c_str());
    }
    (*fileStream_) << "<p><font size='+1'>Inviwo (V " << IVW_VERSION << ") Log File</font></p><br>" << std::endl;
    (*fileStream_) << "<p>" << std::endl;
}

FileLogger::~FileLogger() {
    (*fileStream_) << "</p>" << std::endl;
    fileStream_->close();
    delete fileStream_;
    fileStream_ = NULL;
}

void FileLogger::log(std::string logSource, unsigned int logLevel, const char* fileName,
                     const char* functionName, int lineNumber, std::string logMsg) {
    IVW_UNUSED_PARAM(fileName);
    IVW_UNUSED_PARAM(logLevel);
    IVW_UNUSED_PARAM(functionName);
    IVW_UNUSED_PARAM(lineNumber);

    switch (logLevel) {
        case inviwo::Info:
            (*fileStream_) << "<font color='#000000'>Info: ";
            break;

        case inviwo::Warn:
            (*fileStream_) << "<font color='#FF8000'>Warn: ";
            break;

        case inviwo::Error:
            (*fileStream_) << "<font color='#FF0000'>Error: ";
            break;
    }

    (*fileStream_) << "(" << logSource << ":" << lineNumber << ") " << logMsg;
    (*fileStream_) << "</font><br>" << std::endl;
}

LogCentral::LogCentral() : logLevel_(Info) , logStacktrace_(true) {
    loggers_ = new std::vector<Logger*>();
}
LogCentral::~LogCentral() {
    for (std::vector<Logger*>::iterator it = loggers_->begin(), itEnd = loggers_->end(); it != itEnd; ++it) {
        delete (*it);
    }
    delete loggers_;
}

void LogCentral::registerLogger(Logger* logger) {
    loggers_->push_back(logger);
}

void LogCentral::unregisterLogger(Logger* logger) {
    std::vector<Logger*>::iterator it = find(loggers_->begin(), loggers_->end(), logger);

    if (it != loggers_->end()) {
        delete (*it);
        loggers_->erase(it);
    }
}

void LogCentral::log(std::string logSource, unsigned int logLevel, const char* fileName, const char* functionName, int lineNumber,
                     std::string logMsg) {
    std::string msg = logMsg;

    if (logStacktrace_ && logLevel == Error) {
        std::stringstream ss;
        ss << logMsg;
        std::vector<std::string> stacktrace = getStackTrace();

        for (size_t i = 3; i < stacktrace.size(); ++i) { //start at i == 3 to remove log and getStacktrace from stackgrace
            ss << std::endl << stacktrace[i];
        }

        ss << std::endl; //append an extra line break to easier seperate several stacktraces in a row
        msg = ss.str();
    }

    if (logLevel >= logLevel_)
        for (size_t i=0; i<loggers_->size(); i++)
            loggers_->at(i)->log(logSource, logLevel, fileName, functionName, lineNumber, msg);
}

void LogCentral::setLogStacktrace(const bool& logStacktrace) {
    logStacktrace_ = logStacktrace;
}

bool LogCentral::getLogStacktrace()const {
    return logStacktrace_;
}

} // namespace