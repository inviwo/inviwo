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

#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/logcentral.h>

#include <fmt/format.h>

#ifndef WIN32
#include <signal.h>
#endif

#if defined(WIN32)
extern "C" __declspec(dllimport) int __stdcall IsDebuggerPresent();
#elif defined(__APPLE__)
#include <assert.h>
#include <unistd.h>
#include <sys/sysctl.h>

// Function is taken from https://developer.apple.com/library/mac/qa/qa1361/_index.html
// Returns true if the current process is being debugged (either running under the
// debugger or has a debugger attached post facto).
static bool runningInDebugger() {
    int junk;
    int mib[4];
    struct kinfo_proc info;
    size_t size;

    // Initialize the flags so that, if sysctl fails for some bizarre
    // reason, we get a predictable result.
    info.kp_proc.p_flag = 0;

    // Initialize mib, which tells sysctl the info we want, in this case
    // we're looking for information about a specific process ID.
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    // Call sysctl.
    size = sizeof(info);
    junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
    ghoul_assert(junk == 0, "sysctl call failed");

    // We're being debugged if the P_TRACED flag is set.
    return ((info.kp_proc.p_flag & P_TRACED) != 0);
}
#endif

namespace inviwo {

#if defined(IVW_DEBUG) || defined(IVW_FORCE_ASSERTIONS)

void assertion(std::string_view fileName, std::string_view functionName, long lineNumber,
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
#ifdef WIN32
    if (IsDebuggerPresent()) {
        __debugbreak();
    }
#elif __APPLE__
    if (runningInDebugger()) {
        raise(SIGTRAP);
    }
#else
    raise(SIGTRAP);
#endif
}

}  // namespace inviwo
