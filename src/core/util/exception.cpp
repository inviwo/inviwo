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

#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/logcentral.h>

namespace inviwo {

Exception::Exception(const std::string& message, ExceptionContext context)
    : std::exception(), message_(message), context_(context) {}

Exception::~Exception() throw() {}

std::string Exception::getMessage() const throw() { return message_; };
const char* Exception::what() const throw() { return message_.c_str(); }

const ExceptionContext& Exception::getContext() const { return context_; }

IgnoreException::IgnoreException(const std::string& message, ExceptionContext context)
    : Exception(message, context) {}

AbortException::AbortException(const std::string& message, ExceptionContext context)
    : Exception(message, context) {}

FileException::FileException(const std::string& message, ExceptionContext context)
    : Exception(message, context) {}

ModuleInitException::ModuleInitException(const std::string& message, ExceptionContext context,  std::vector<std::string> modulesToDeregister)
    : Exception(message, context), modulesToDeregister_(std::move(modulesToDeregister)) {}

const std::vector<std::string>& ModuleInitException::getModulesToDeregister() const {
    return modulesToDeregister_;
}

void StandardExceptionHandler::operator()(ExceptionContext context) {
    try {
        throw;
    } catch (Exception& e) {
        util::log(e.getContext(), e.getMessage(), LogLevel::Error);
    } catch (std::exception& e) {
        util::log(context, std::string(e.what()), LogLevel::Error);
    } catch (...) {
        util::log(context, "Unknown error", LogLevel::Error);
    }
}

ExceptionContext::ExceptionContext(std::string caller /*= ""*/, std::string file /*= ""*/, std::string function /*= ""*/, int line /*= 0*/) : caller_(caller), file_(file), function_(function), line_(line) {

}

const std::string& ExceptionContext::getCaller() {
    return caller_;
}

const std::string& ExceptionContext::getFile() {
    return file_;
}

const std::string& ExceptionContext::getFunction() {
    return function_;
}

const int& ExceptionContext::getLine() {
    return line_;
}


}  // namespace
