/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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
#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/core/util/unindent.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <functional>
#include <cctype>
#include <locale>

#include <fmt/format.h>

namespace inviwo {

template <class T>
std::string toString(T value) {
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

template <class T>
T stringTo(std::string_view str) {
    std::stringstream stream;
    stream << str;
    T result;
    stream >> result;
    return result;
}

/**
 * @brief A string formating buffer
 * A utility for formating strings. Uses a stack buffer of 500 chars that will grow on the head if
 * needed. The StrBuffer is implicitly convertible to a string_view.
 */
struct IVW_CORE_API StrBuffer {
    /**
     * @brief Create an empty StrBuffer
     */
    StrBuffer() = default;

    /**
     * @brief Format args using format into a new StrBuffer
     */
    template <typename... Args>
    explicit StrBuffer(fmt::format_string<Args...> format, Args&&... args) {
        fmt::format_to(std::back_inserter(buff), format, std::forward<Args>(args)...);
    }

    /**
     * @brief Append new content into buffer using format and args
     */
    template <typename... Args>
    StrBuffer& append(fmt::format_string<Args...> format, Args&&... args) {
        fmt::format_to(std::back_inserter(buff), format, std::forward<Args>(args)...);
        return *this;
    }

    /**
     * @brief Clear buffer content and format args using format into buffer.
     */
    template <typename... Args>
    StrBuffer& replace(fmt::format_string<Args...> format, Args&&... args) {
        buff.clear();
        fmt::format_to(std::back_inserter(buff), format, std::forward<Args>(args)...);
        return *this;
    }

    /**
     * @brief Clear the buffer
     */
    void clear() { buff.clear(); }

    /**
     * @brief Check if buffer is empty
     */
    bool empty() const { return buff.size() == 0; }

    /**
     * @brief Get a string_view into the buffer
     */
    std::string_view view() const { return {buff.data(), buff.size()}; }

    /**
     * @brief Implicitly conversion to string_view
     */
    operator std::string_view() const { return {buff.data(), buff.size()}; }

    /**
     * @brief return a null-terminated c-style string. Will always append a '0' at the end of the
     * buffer
     */
    const char* c_str() {
        buff.push_back(0);
        return buff.data();
    }

    fmt::memory_buffer buff;
};

namespace util {

/**
 * \brief convert the given std::string to std::wstring.
 * On Windows, MultiByteToWideChar is used for this conversion assuming utf8 encoding.
 * Otherwise, std::mbsrtowcs is used.
 *
 * @param str   multibyte character string
 * @return input converted to std::wstring
 */
IVW_CORE_API std::wstring toWstring(std::string_view str);

/**
 * \brief convert the given std::wstring to std::string.
 * On Windows, WideCharToMultiByte is used for this conversion assuming utf8 encoding.
 * Otherwise, std::wcsrtombs is used.
 *
 * @param str   std::wstring character string
 * @return input converted to multibyte std::string
 */
IVW_CORE_API std::string fromWstring(std::wstring_view str);

/**
 * @brief Call a functor on each part of the string after splitting by sep
 * @param str The string to split
 * @param sep The delimiter to split by
 * @param func Function callback, should take a std::string_view as argument
 */
template <typename Func>
constexpr void forEachStringPart(std::string_view str, std::string_view sep, Func&& func) {
    if (str.empty()) return;
    for (size_t first = 0; first < str.size();) {
        const auto second = str.find(sep, first);
        std::invoke(func, str.substr(first, second - first));
        if (second == std::string_view::npos) break;
        first = second + sep.size();
    }
}

/**
 * @brief Divide a string into two parts by the first instance of a delimiter
 * @param str string to divide
 * @param delimiter not include in either returned strings
 * @return a pair of strings, if the delimiter is not found the first string will be the same as the
 * input str, and the second one will be empty
 */
constexpr std::pair<std::string_view, std::string_view> splitByFirst(std::string_view str,
                                                                     char delimiter = ' ') {
    const auto pos = str.find(delimiter);
    return {str.substr(0, pos), pos != str.npos ? str.substr(pos + 1) : std::string_view{}};
}

/**
 * @brief Divide a string into two parts by the first instance of a delimiter
 * @param str string to divide
 * @param delimiter not include in either returned strings
 * @return a pair of strings, if the delimiter is not found the first string will be the same as the
 * input str, and the second one will be empty
 */
constexpr std::pair<std::string_view, std::string_view> splitByFirst(std::string_view str,
                                                                     std::string_view delimiter) {
    const auto pos = str.find(delimiter);
    return {str.substr(0, pos),
            pos != str.npos ? str.substr(pos + delimiter.size()) : std::string_view{}};
}

/**
 * @brief Divide a string into two parts by the last instance of a delimiter
 * @param str string to divide
 * @param delimiter not include in either returned strings
 * @return a pair of strings, if the delimiter is not found the first string will empty, and the
 * second one will be equal to the input str
 */
constexpr std::pair<std::string_view, std::string_view> splitByLast(std::string_view str,
                                                                    char delimiter = ' ') {
    const auto pos = str.rfind(delimiter);
    return pos != str.npos ? std::pair{str.substr(0, pos), str.substr(pos + 1)}
                           : std::pair{std::string_view{}, str};
}

/**
 * @brief Divide a string into two parts by the last instance of a delimiter
 * @param str string to divide
 * @param delimiter not include in either returned strings
 * @return a pair of strings, if the delimiter is not found the first string will empty, and the
 * second one will be equal to the input str
 */
constexpr std::pair<std::string_view, std::string_view> splitByLast(std::string_view str,
                                                                    std::string_view delimiter) {
    const auto pos = str.rfind(delimiter);
    return pos != str.npos ? std::pair{str.substr(0, pos), str.substr(pos + delimiter.size())}
                           : std::pair{std::string_view{}, str};
}

/**
 * \brief Split string into substrings based on separating delimiter character.
 * Using delimiter ';' on string "aa;bb" will result in a vector contaning aa and bb.
 *
 * @note Empty substrings are not skipped, ";;" will generate an element.
 * @param str The string to split
 * @param delimiter The character use for splitting (default to space)
 * @return a vector containing the substrings as std::string
 */
IVW_CORE_API std::vector<std::string> splitString(std::string_view str, char delimiter = ' ');

/**
 * \brief Split string into substrings based on separating delimiter character.
 * Using delimiter ';' on string "aa;bb" will result in a vector contaning aa and bb.
 *
 * @note Empty substrings are not skipped, ";;" will generate an element.
 * @param str The string to split
 * @param delimiter The character use for splitting (default to space)
 * @return a vector containing the substrings as std::string_view
 */
IVW_CORE_API std::vector<std::string_view> splitStringView(std::string_view str,
                                                           char delimiter = ' ');

/**
 * \brief trims \p str from beginning and end by removing white spaces
 *
 * @param str   input string
 * @return trimmed stringview without leading/trailing white space
 */
constexpr std::string_view trim(std::string_view str) noexcept {
    const auto idx1 = str.find_first_not_of(" \f\n\r\t\v");
    if (idx1 == std::string_view::npos) return "";
    const auto idx2 = str.find_last_not_of(" \f\n\r\t\v");
    return str.substr(idx1, idx2 + 1 - idx1);
}

/**
 * \brief trims \p str from beginning removing white spaces
 *
 * @param str   input string
 * @return trimmed stringview without leading white space
 */
constexpr std::string_view ltrim(std::string_view str) noexcept {
    const auto idx1 = str.find_first_not_of(" \f\n\r\t\v");
    if (idx1 == std::string_view::npos) return "";
    return str.substr(idx1);
}

/**
 * \brief trims \p str from end by removing white spaces
 *
 * @param str   input string
 * @return trimmed stringview without trailing white space
 */
constexpr std::string_view rtrim(std::string_view str) noexcept {
    const auto idx2 = str.find_last_not_of(" \f\n\r\t\v");
    return str.substr(0, idx2 + 1);
}

/**
 * \brief Checks if provided string ends with suffix using case insensitive equal comparison.
 * @param str string to check last part of. Allowed to be smaller than suffix.
 * @param suffix Ending to match.
 * @return True if last part of str is equal to suffix, false otherwise.
 */
IVW_CORE_API bool iCaseEndsWith(std::string_view str, std::string_view suffix);

/**
 * \brief Elide parts of lines in \p str which are longer than \p maxLineLength and append \p abbrev
 * instead.
 * @param str            string with lines to abbreviate
 * @param abbrev         placeholder that gets added at the end of abbreviated lines
 * @param maxLineLength  lines that are longer are abbreviated
 * @return input string where no line is longer than \p maxLineLength + \p abbrev.size()
 */
IVW_CORE_API std::string elideLines(std::string_view str, std::string_view abbrev = "...",
                                    size_t maxLineLength = 500);

constexpr auto fmtHelp = IVW_UNINDENT(R"(
    To locate the error try breaking on fmt::format_error exceptions,
    or put a breakpoint in fmt::detail::error_handler::on_error.
    in lldb try "breakpoint set -E c++ -O fmt::format_error"
    or "breakpoint set -M fmt::v7::detail::error_handler::on_error".
    )");

IVW_CORE_API void urlEncodeTo(std::string_view text, StrBuffer& strBuffer);
IVW_CORE_API void htmlEncodeTo(std::string_view data, StrBuffer& strBuffer);
IVW_CORE_API std::string urlEncode(std::string_view text);
IVW_CORE_API std::string htmlEncode(std::string_view data);

}  // namespace util

// Keep this here to avoid breaking old code
using util::splitString;

template <typename T>
std::string joinString(const std::vector<T>& str, std::string_view delimiter = " ") {
    std::stringstream ss;
    std::copy(str.begin(), str.end(), util::make_ostream_joiner(ss, delimiter));
    return ss.str();
}

template <typename Iterator>
std::string joinString(Iterator begin, Iterator end, std::string_view delimiter = " ") {
    std::stringstream ss;
    std::copy(begin, end, util::make_ostream_joiner(ss, delimiter));
    return ss.str();
}

IVW_CORE_API std::vector<std::string> splitStringWithMultipleDelimiters(
    std::string_view str, std::vector<char> delimiters = {'_', '+', '-', '.', ' '});

IVW_CORE_API std::string removeSubString(std::string_view str, std::string_view strToRemove);

IVW_CORE_API std::string removeFromString(std::string str, char char_to_remove = ' ');
IVW_CORE_API void replaceInString(std::string& str, std::string_view oldStr,
                                  std::string_view newStr);

IVW_CORE_API std::string toUpper(std::string_view str);
IVW_CORE_API std::string toLower(std::string_view str);

IVW_CORE_API size_t countLines(std::string_view str);

IVW_CORE_API std::string randomString(size_t length = 10);

// trim from start
IVW_CORE_API std::string ltrim(std::string s);
// trim from end
IVW_CORE_API std::string rtrim(std::string s);
// trim from both ends
IVW_CORE_API std::string trim(std::string s);

IVW_CORE_API std::string dotSeperatedToPascalCase(std::string_view s);
IVW_CORE_API std::string camelCaseToHeader(std::string_view s);

/**
 * \brief Case insensitive equal comparison of two strings.
 * @return true if all the elements in the two containers are the same.
 * @see CaseInsensitiveCompare
 */
IVW_CORE_API bool iCaseCmp(std::string_view l, std::string_view r);
/**
 * \brief Case insensitive alphabetical less than comparison of two strings, i.e. "aa" < "ab" =
 * true. Compares letters from left to right using std::lexicographical_compare.
 * @return true if the alphabetical order in the first string is less than the second string.
 */
IVW_CORE_API bool iCaseLess(std::string_view l, std::string_view r);
/**
 * \brief Case insensitive equal comparison of two strings to be used for template arguments.
 * @see iCaseCmp
 */
struct IVW_CORE_API CaseInsensitiveCompare {
    bool operator()(std::string_view a, std::string_view b) const;
    using is_transparent = int;
};

[[deprecated("use util::parseTypeIdName() in demangle.h instead")]] IVW_CORE_API std::string
parseTypeIdName(const char* name);

[[deprecated("use util::msToString() in chronoutils.h instead")]] IVW_CORE_API std::string
msToString(double ms, bool includeZeros = true, bool spacing = false);

}  // namespace inviwo
