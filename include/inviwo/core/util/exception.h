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

#ifndef IVW_EXCEPTION_H
#define IVW_EXCEPTION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <iostream>
#include <string>
#include <functional>
#include <stdexcept>

#include <warn/push>
#include <warn/ignore/dll-interface-base>
#include <warn/ignore/dll-interface>

namespace inviwo {

struct IVW_CORE_API ExceptionContext {
    ExceptionContext(std::string caller = "", std::string file = "", std::string function = "",
                     int line = 0);

    const std::string& getCaller();
    const std::string& getFile();
    const std::string& getFunction();
    const int& getLine();

private:
    std::string caller_;
    std::string file_;
    std::string function_;
    int line_;
};

#define IvwContext                                                                             \
    ExceptionContext(parseTypeIdName(std::string(typeid(this).name())), std::string(__FILE__), \
                     std::string(__FUNCTION__), __LINE__)

#define IvwContextCustom(source) \
    ExceptionContext(source, std::string(__FILE__), std::string(__FUNCTION__), __LINE__)

using ExceptionHandler = std::function<void(ExceptionContext)>;

class IVW_CORE_API Exception : public std::exception {
public:
    Exception(const std::string& message = "", ExceptionContext context = ExceptionContext());
    virtual ~Exception() throw();
    virtual std::string getMessage() const throw();
    virtual const char* what() const throw() override;
    virtual const ExceptionContext& getContext() const;

private:
    std::string message_;
    ExceptionContext context_;
};

class IVW_CORE_API IgnoreException : public Exception {
public:
    IgnoreException(const std::string& message = "", ExceptionContext context = ExceptionContext());
    virtual ~IgnoreException() throw() {}
};

class IVW_CORE_API AbortException : public Exception {
public:
    AbortException(const std::string& message = "", ExceptionContext context = ExceptionContext());
    virtual ~AbortException() throw() {}
};

class IVW_CORE_API FileException : public Exception {
public:
    FileException(const std::string& message = "", ExceptionContext context = ExceptionContext());
    virtual ~FileException() throw() {}
};

struct IVW_CORE_API StandardExceptionHandler {
    void operator()(ExceptionContext);
};



}  // namespace

#include <warn/pop> 

#endif  // IVW_EXCEPTION_H
