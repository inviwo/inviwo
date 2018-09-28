/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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
#include <inviwo/core/util/stringconversion.h>
#include <iostream>
#include <string>
#include <functional>
#include <stdexcept>
#include <vector>

#include <warn/push>
#include <warn/ignore/dll-interface-base>
#include <warn/ignore/dll-interface>

namespace inviwo {

struct IVW_CORE_API ExceptionContext {
    ExceptionContext(std::string caller = "", std::string file = "", std::string function = "",
                     int line = 0);
    const std::string& getCaller() const;
    const std::string& getFile() const;
    const std::string& getFunction() const;
    const int& getLine() const;

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
    virtual ~Exception() noexcept;
    virtual std::string getMessage() const noexcept;
    virtual const char* what() const noexcept override;
    virtual const ExceptionContext& getContext() const;
    const std::vector<std::string>& getStack() const;

private:
    std::string message_;
    ExceptionContext context_;
    std::vector<std::string> stack_;
};

class IVW_CORE_API RangeException : public Exception {
public:
    RangeException(const std::string& message = "", ExceptionContext context = ExceptionContext());
    virtual ~RangeException() noexcept = default;
};

class IVW_CORE_API NullPointerException : public Exception {
public:
    NullPointerException(const std::string& message = "",
                         ExceptionContext context = ExceptionContext());
    virtual ~NullPointerException() noexcept = default;
};

class IVW_CORE_API IgnoreException : public Exception {
public:
    IgnoreException(const std::string& message = "", ExceptionContext context = ExceptionContext());
    virtual ~IgnoreException() noexcept = default;
};

class IVW_CORE_API AbortException : public Exception {
public:
    AbortException(const std::string& message = "", ExceptionContext context = ExceptionContext());
    virtual ~AbortException() noexcept = default;
};

class IVW_CORE_API FileException : public Exception {
public:
    FileException(const std::string& message = "", ExceptionContext context = ExceptionContext());
    virtual ~FileException() noexcept = default;
};

class IVW_CORE_API ResourceException : public Exception {
public:
    ResourceException(const std::string& message = "",
                      ExceptionContext context = ExceptionContext());
    virtual ~ResourceException() noexcept = default;
};

class IVW_CORE_API ModuleInitException : public Exception {
public:
    ModuleInitException(const std::string& message = "",
                        ExceptionContext context = ExceptionContext(),
                        std::vector<std::string> modulesToDeregister = {});
    virtual ~ModuleInitException() noexcept = default;

    /**
     * When registering a module fails, also remove these modules.
     * Useful for implicit dependencies. Like OpenGL's dependency on GLFW or OpenGLQt module.
     */
    const std::vector<std::string>& getModulesToDeregister() const;

private:
    std::vector<std::string> modulesToDeregister_;
};

struct IVW_CORE_API StandardExceptionHandler {
    void operator()(ExceptionContext);
};

}  // namespace inviwo

#include <warn/pop>

#endif  // IVW_EXCEPTION_H
