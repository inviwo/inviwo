/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/fmtutils.h>

#include <functional>

namespace inviwo {

class Processor;

enum class EvaluationType { InitResource, PortOnChange, Process, NotReady };

constexpr std::string_view enumToStr(EvaluationType type) {
    switch (type) {
        case EvaluationType::InitResource:
            return "InitializeResources";
        case EvaluationType::PortOnChange:
            return "PortOnChange";
        case EvaluationType::Process:
            return "Process";
        case EvaluationType::NotReady:
            return "DoIfNotReady";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"), "Found invalid EvaluationType enum value '{}'",
                    static_cast<int>(type));
}

using EvaluationErrorHandler = std::function<void(Processor*, EvaluationType, ExceptionContext)>;

struct IVW_CORE_API StandardEvaluationErrorHandler {
    void operator()(Processor*, EvaluationType, ExceptionContext);
};

}  // namespace inviwo

template <>
struct fmt::formatter<inviwo::EvaluationType> : inviwo::FlagFormatter<inviwo::EvaluationType> {};
