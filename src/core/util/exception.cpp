/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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
#include <inviwo/core/util/stacktrace.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/safecstr.h>
#include <sstream>

#include <fmt/format.h>

namespace inviwo {

namespace {

std::vector<std::string> stackTrace() {
    if (InviwoApplication::isInitialized()) {
        if (InviwoApplication::getPtr()->getSystemSettings().stackTraceInException_) {
            auto stack = getStackTrace();
            const auto offset = std::min(size_t(4), stack.size());
            stack.erase(stack.begin(), stack.begin() + offset);
            return stack;
        }
    }
    return {};
}

bool breakOnException() {
    if (InviwoApplication::isInitialized()) {
        return InviwoApplication::getPtr()->getSystemSettings().breakOnException_;
    }
    return false;
}

}  // namespace

Exception::Exception(std::string_view message, ExceptionContext context)
    : std::runtime_error(SafeCStr{message}), context_(std::move(context)), stack_{stackTrace()} {
    if (breakOnException()) util::debugBreak();
}

Exception::Exception(std::string_view format, fmt::format_args&& args, ExceptionContext context)
    : std::runtime_error{fmt::vformat(format, std::move(args))}, context_{std::move(context)} {
    if (breakOnException()) util::debugBreak();
}

Exception::~Exception() noexcept = default;

std::string Exception::getMessage() const { return what(); }

std::string Exception::getFullMessage() const {
    std::stringstream ss;
    getFullMessage(ss);
    return ss.str();
}

void Exception::getFullMessage(std::ostream& ss, int maxFrames) const {
    ss << what() << "\n";
    ss << context_ << "\n";
    if (!stack_.empty()) {
        ss << "\nStack Trace:\n";
        getStack(ss, maxFrames);
    }
}

void Exception::getStack(std::ostream& os, int maxFrames) const {
    auto j = inviwo::util::make_ostream_joiner(os, "\n");
    if (maxFrames > 0 && static_cast<int>(stack_.size()) > maxFrames) {
        std::copy(stack_.begin(), stack_.begin() + maxFrames, j);
        os << "\n...";
    } else {
        std::copy(stack_.begin(), stack_.end(), j);
    }
}

const ExceptionContext& Exception::getContext() const { return context_; }

const std::vector<std::string>& Exception::getStack() const { return stack_; }

ModuleInitException::ModuleInitException(std::string_view message, ExceptionContext context,
                                         std::vector<std::string> modulesToDeregister)
    : Exception(message, std::move(context))
    , modulesToDeregister_(std::move(modulesToDeregister)) {}

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

std::ostream& operator<<(std::ostream& ss, const Exception& e) {
    e.getFullMessage(ss);
    return ss;
}

}  // namespace inviwo
