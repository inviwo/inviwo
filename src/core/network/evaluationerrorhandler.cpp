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

#include <inviwo/core/network/evaluationerrorhandler.h>
#include <inviwo/core/processors/processor.h>

namespace inviwo {

void StandardEvaluationErrorHandler::operator()(Processor* processor, EvaluationType type,
                                                ExceptionContext context) {
    const std::string id = processor->getIdentifier();
    const std::string func = [&]() {
        switch (type) {
            case EvaluationType::InitResource:
                return "InitializeResources";
            case EvaluationType::PortOnChange:
                return "PortOnChange";
            case EvaluationType::Process:
                return "Process";
            case EvaluationType::NotReady:
                return "DoIfNotReady";
            default:
                return "Unknown";
        }
    }();

    try {
        throw;
    } catch (Exception& e) {
        util::log(e.getContext(), id + " Error in " + func + ": " + e.getMessage(),
                  LogLevel::Error);

        if (!e.getStack().empty()) {
            std::stringstream ss;
            ss << "Stack Trace:\n";
            e.getStack(ss);
            util::log(e.getContext(), ss.str(), LogLevel::Info);
        }
    } catch (std::exception& e) {
        util::log(context, id + " Error in " + func + ": " + std::string(e.what()),
                  LogLevel::Error);
    } catch (...) {
        util::log(context, id + " Error in " + func + ": " + "Unknown error", LogLevel::Error);
    }
}

}  // namespace inviwo
