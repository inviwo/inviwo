/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#ifndef WIN32
#include <signal.h>
#endif

namespace inviwo {

#if defined(IVW_DEBUG) || defined(IVW_FORCE_ASSERTIONS)

void assertion(const char* fileName, const char* functionName, long lineNumber,
               std::string message) {
    std::cout << "Assertion failed in (" << fileName << ":" << lineNumber << ", " << functionName
              << "): ";
    std::cout << message << std::endl;

    if (LogCentral::isInitialized()) {
        LogCentral::getPtr()->logAssertion(fileName, functionName, lineNumber, message);
    }

    util::debugBreak();
    exit(-1);
}

#else

void assertion(const char*, const char*, long, std::string) {}

#endif  // _DEBUG

void util::debugBreak() {
#ifdef WIN32
    __debugbreak();
#else
    raise(SIGTRAP);
#endif
}

}  // namespace inviwo
