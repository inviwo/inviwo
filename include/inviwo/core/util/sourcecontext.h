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
#include <cstring>
#include <iostream>

namespace inviwo {

/**
 * Represents a location in source code
 */
class IVW_CORE_API SourceContext {
public:
    /**
     * Construct a SourceContext, this is usually not done manually, but rather one of the macros
     * bellow is used to automatically get the right arguments. SourceContext copies its arguments,
     * for a more lightweight version @see SourceLocation
     * @param caller usually the class name of *this in the current scope.
     * @param file filename path of the source file
     * @param function name of the function in the current scope
     * @param line line number in the current source file
     */
    SourceContext(std::string caller = "", std::string file = "", std::string function = "",
                  int line = 0)
        : caller_(caller), file_(file), function_(function), line_(line) {}

    /**
     * Usually the class name of *this in the current scope.
     */
    const std::string& getCaller() const { return caller_; };

    /**
     * The name and path the the source file
     */
    const std::string& getFile() const { return file_; };

    /**
     * Name of the function in the current scope
     */
    const std::string& getFunction() const { return function_; };

    /**
     * Line number in the current source file
     */
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

/**
 * Represents a location in source code, similar to SourceContext but much more lightweight.
 * SourceLocation does not take ownership of its given string, and assumes they are in static
 * storage, which is the case for the file and function macros used below. But care has to be taken
 * if one uses this class directly. The reason for this it so allow it to be fast, lightweight and
 * constexpr. And to avoid unnecessary copies of file and function names. For a owning version @see
 * SourceContext
 */
class IVW_CORE_API SourceLocation {
public:
    /**
     * This function does now take ownership of file and function!
     * @param file filename path of the source file
     * @param function name of the function in the current scope
     * @param line line number in the current source file
     */
    constexpr SourceLocation(const char* file, const char* function, int line)
        : file_{file}, function_{function}, line_{line} {}

    /**
     * The name and path the the source file
     */
    constexpr const char* getFile() const noexcept { return file_; };

    /**
     * Name of the function in the current scope
     */
    constexpr const char* getFunction() const noexcept { return function_; };

    /**
     * Line number in the current source file
     */
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
    size_t operator()(const ::inviwo::SourceContext& sl) const noexcept {
        size_t h = 0;
        ::inviwo::util::hash_combine(h, sl.getCaller());
        ::inviwo::util::hash_combine(h, sl.getFile());
        ::inviwo::util::hash_combine(h, sl.getFunction());
        ::inviwo::util::hash_combine(h, sl.getLine());
        return h;
    }
};

}  // namespace std
