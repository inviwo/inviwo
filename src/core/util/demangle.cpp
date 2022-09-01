/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/core/util/demangle.h>
#include <inviwo/core/util/stringconversion.h>

#if defined(__clang__) || defined(__GNUC__)
#include <cstdlib>
#include <cxxabi.h>
#endif

namespace inviwo {

namespace util {

std::string demangle(const char* name) {
#if defined(__clang__) || defined(__GNUC__)
    struct handle {
        char* p;
        handle(char* ptr) : p(ptr) {}
        ~handle() { std::free(p); }
    };
    int status = -4;
    handle result(abi::__cxa_demangle(name, nullptr, nullptr, &status));
    return {status == 0 ? result.p : name};
#else
    std::string str{name};
    replaceInString(str, "class ", "");
    replaceInString(str, "struct ", "");
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64)
    replaceInString(str, "__ptr64", "");
#endif
    return str;
#endif
}

std::string parseTypeIdName(const char* name) {
    std::string str{demangle(name)};
    replaceInString(str, "inviwo::", "");
    replaceInString(str, "const", "");
    return removeFromString(removeFromString(str, '*'), ' ');
}

}  // namespace util

}  // namespace inviwo
