/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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
#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/fmtutils.h>
#include <inviwo/core/util/sourcecontext.h>
#include <inviwo/core/util/exception.h>

#include <string_view>

namespace inviwo {

class IVW_CORE_API ProcessorStatus {
public:
    enum class Status { Ready, NotReady, Error };

    constexpr ProcessorStatus(bool ready) : status_{ready ? Ready : NotReady}, reason_{} {}

    constexpr ProcessorStatus(Status status) : status_{status}, reason_{} {}
    ProcessorStatus(Status status, std::string_view reason) : status_{status}, reason_{reason} {}
    ProcessorStatus(std::string_view error) : status_{Error}, reason_{error} {}

    constexpr ProcessorStatus() : status_{NotReady} {}
    constexpr ProcessorStatus(const ProcessorStatus&) noexcept = default;
    constexpr ProcessorStatus(ProcessorStatus&&) noexcept = default;
    constexpr ProcessorStatus& operator=(const ProcessorStatus&) noexcept = default;
    constexpr ProcessorStatus& operator=(ProcessorStatus&&) noexcept = default;

    constexpr ProcessorStatus& operator=(bool ready) {
        status_ = ready ? Ready : NotReady;
        return *this;
    }

    constexpr operator bool() const { return status_ == Ready; }
    constexpr operator Status() const { return status_; }

    constexpr Status status() const { return status_; }
    std::string_view reason() const { return reason_; }

    constexpr std::string_view str() const;

    constexpr auto static Ready = Status::Ready;
    constexpr auto static NotReady = Status::NotReady;
    constexpr auto static Error = Status::Error;

    constexpr bool operator==(const ProcessorStatus& that) const { return status_ == that.status_; }
    constexpr bool operator==(ProcessorStatus::Status status) const { return status_ == status; }

private:
    Status status_;
    std::string_view reason_;
};

constexpr std::string_view enumToStr(ProcessorStatus::Status s) {
    using enum ProcessorStatus::Status;
    switch (s) {
        case Ready:
            return "Ready";
        case NotReady:
            return "Not Ready";
        case Error:
            return "Error";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"),
                    "Found invalid ProcessorStatus::Status enum value '{}'", static_cast<int>(s));
}

constexpr std::string_view ProcessorStatus::str() const { return enumToStr(status_); }

}  // namespace inviwo

template <>
struct fmt::formatter<inviwo::ProcessorStatus::Status>
    : inviwo::FlagFormatter<inviwo::ProcessorStatus::Status> {};

template <>
struct fmt::formatter<inviwo::ProcessorStatus> : fmt::formatter<fmt::string_view> {
    template <typename FormatContext>
    auto format(const inviwo::ProcessorStatus& status, FormatContext& ctx) const {
        fmt::memory_buffer buff;
        fmt::format_to(std::back_inserter(buff), "{}", status.status());
        if (status != inviwo::ProcessorStatus::Ready && !status.reason().empty()) {
            fmt::format_to(std::back_inserter(buff), ": {}", status.reason());
        }
        return formatter<fmt::string_view>::format(fmt::string_view(buff.data(), buff.size()), ctx);
    }
};
