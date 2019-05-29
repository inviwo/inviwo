/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/hashcombine.h>

#include <string>
#include <iostream>

namespace inviwo {

/**
 * Represents a location in source code
 */
class IVW_CORE_API SourceContext {
public:
    SourceContext(std::string caller = "", std::string file = "", std::string function = "",
                  int line = 0)
        : caller_(caller), file_(file), function_(function), line_(line) {}
    const std::string& getCaller() const { return caller_; };
    const std::string& getFile() const { return file_; };
    const std::string& getFunction() const { return function_; };
    int getLine() const { return line_; };

private:
    std::string caller_;
    std::string file_;
    std::string function_;
    int line_ = 0;
};

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             const SourceContext& ec) {
    ss << ec.getCaller() << " (" << ec.getFile() << ":" << ec.getLine() << ")";
    return ss;
}

#define IVW_CONTEXT                                                                         \
    SourceContext(parseTypeIdName(std::string(typeid(this).name())), std::string(__FILE__), \
                  std::string(__FUNCTION__), __LINE__)

#define IVW_CONTEXT_CUSTOM(source) \
    SourceContext(source, std::string(__FILE__), std::string(__FUNCTION__), __LINE__)

// Old deprecated macro, use uppercase
#define IvwContext                                                                          \
    SourceContext(parseTypeIdName(std::string(typeid(this).name())), std::string(__FILE__), \
                  std::string(__FUNCTION__), __LINE__)
// Old deprecated macro, use uppercase
#define IvwContextCustom(source) \
    SourceContext(source, std::string(__FILE__), std::string(__FUNCTION__), __LINE__)

class IVW_CORE_API SourceLocation {
public:
    constexpr SourceLocation(const char* file, const char* function, int line)
        : file_{file}, function_{function}, line_{line} {}

    constexpr const char* getFile() const noexcept { return file_; };
    constexpr const char* getFunction() const noexcept { return function_; };
    constexpr int getLine() const noexcept { return line_; };

private:
    const char* file_;
    const char* function_;
    int line_;
};

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             const SourceLocation& ec) {
    ss << ec.getFunction() << " (" << ec.getFile() << ":" << ec.getLine() << ")";
    return ss;
}

IVW_CORE_API constexpr bool operator==(const SourceLocation& a, const SourceLocation& b) noexcept {
    return a.getLine() == b.getLine() && std::strcmp(a.getFile(), b.getFile()) == 0 &&
           std::strcmp(a.getFunction(), b.getFunction()) == 0;
}

IVW_CORE_API constexpr bool operator!=(const SourceLocation& a, const SourceLocation& b) noexcept {
    return !(a == b);
}

#define IVW_SOURCE_LOCATION SourceLocation(__FILE__, __FUNCTION__, __LINE__)

}  // namespace inviwo

namespace std {

template <>
struct hash<::inviwo::SourceLocation> {
    constexpr size_t operator()(const ::inviwo::SourceLocation& sl) const noexcept {
        size_t h = 0;
        ::inviwo::util::hash_combine(h, sl.getFile());
        ::inviwo::util::hash_combine(h, sl.getFunction());
        ::inviwo::util::hash_combine(h, sl.getLine());
        return h;
    }
};

template <>
struct hash<::inviwo::SourceContext> {
    constexpr size_t operator()(const ::inviwo::SourceContext& sl) const noexcept {
        size_t h = 0;
        ::inviwo::util::hash_combine(h, sl.getCaller());
        ::inviwo::util::hash_combine(h, sl.getFile());
        ::inviwo::util::hash_combine(h, sl.getFunction());
        ::inviwo::util::hash_combine(h, sl.getLine());
        return h;
    }
};

}  // namespace std
