/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/logcentral.h>

#include <fmt/format.h>

#ifndef WIN32
#include <signal.h>
#endif

#if defined(WIN32) && defined(_MSC_VER)
extern "C" __declspec(dllimport) int __stdcall IsDebuggerPresent();
#else
#include <cstdio>
#include <cstdlib>

// detect if GDB/LLDB is present
// https://stackoverflow.com/questions/3596781/how-to-detect-if-the-current-process-is-being-run-by-gdb/8135517#8135517
int debuggerPresent = -1;
static void _sigtrap_handler(int signum) {
    debuggerPresent = 0;
    signal(SIGTRAP, SIG_DFL);
}
#endif

namespace inviwo {

#if defined(IVW_DEBUG) || defined(IVW_FORCE_ASSERTIONS)

void assertion(std::string_view fileName, std::string_view functionName, int lineNumber,
               std::string_view error) {

    const auto message = fmt::format("Assertion failed in ({}:{}, {}): {}", fileName, lineNumber,
                                     functionName, error);
    if (LogCentral::isInitialized()) {
        LogCentral::getPtr()->logAssertion(fileName, functionName, lineNumber, message);
    }

    util::debugBreak();
    exit(-1);
}

#else

void assertion(std::string_view, std::string_view, long, std::string_view) {}

#endif  // _DEBUG

void util::debugBreak() {
#if defined(WIN32) && defined(_MSC_VER)
    if (IsDebuggerPresent()) {
        __debugbreak();
    }
#else
    if (-1 == debuggerPresent) {
        signal(SIGTRAP, _sigtrap_handler);
        raise(SIGTRAP);
    }
#endif
}

}  // namespace inviwo
