/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_STRINGCONVERSION_H
#define IVW_STRINGCONVERSION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/ostreamjoiner.h>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <functional>
#include <cctype>
#include <locale>

namespace inviwo {

template <class T>
std::string toString(T value) {
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

template <class T>
T stringTo(const std::string& str) {
    T result;
    std::istringstream stream;
    stream.str(str);
    stream >> result;
    return result;
}

namespace util {

/**
 * \brief convert the given std::string to std::wstring.
 * On Windows, MultiByteToWideChar is used for this conversion assuming utf8 encoding.
 * Otherwise, std::mbsrtowcs is used.
 *
 * @param str   multibyte character string
 * @return input converted to std::wstring
 */
std::wstring toWstring(const std::string& str);

}  // namespace util

/**
 * \brief Split string into substrings based on separating delimiter character.
 * Using delimiter ';' on string "aa;bb" will result in a vector contaning aa and bb.
 *
 * @note Empty substrings are not skipped, ";;" will generate an element.
 * @param str The string to split
 * @param delimeter The character use for splitting (default to space)
 * @return a vector containing the substrings
 */
IVW_CORE_API std::vector<std::string> splitString(const std::string& str, char delimeter = ' ');

template <typename T>
std::string joinString(const std::vector<T>& str, std::string delimeter = " ") {
    std::stringstream ss;
    std::copy(str.begin(), str.end(), util::make_ostream_joiner(ss, delimeter));
    return ss.str();
}

template <typename Iterator>
std::string joinString(Iterator begin, Iterator end, std::string delimeter = " ") {
    std::stringstream ss;
    std::copy(begin, end, util::make_ostream_joiner(ss, delimeter));
    return ss.str();
}

IVW_CORE_API std::string htmlEncode(const std::string& data);

IVW_CORE_API std::vector<std::string> splitStringWithMultipleDelimiters(
    const std::string& str, std::vector<char> delimiters = std::vector<char>());

IVW_CORE_API std::string removeSubString(const std::string& str, const std::string& strToRemove);

IVW_CORE_API std::string removeFromString(std::string str, char char_to_remove = ' ');
IVW_CORE_API void replaceInString(std::string& str, const std::string& oldStr,
                                  const std::string& newStr);
IVW_CORE_API std::string parseTypeIdName(std::string str);

IVW_CORE_API std::string toUpper(std::string str);
IVW_CORE_API std::string toLower(std::string str);

IVW_CORE_API size_t countLines(std::string str);

IVW_CORE_API std::string randomString(size_t length = 10);

// trim from start
IVW_CORE_API std::string ltrim(std::string s);
// trim from end
IVW_CORE_API std::string rtrim(std::string s);
// trim from both ends
IVW_CORE_API std::string trim(std::string s);

IVW_CORE_API std::string dotSeperatedToPascalCase(const std::string& s);
IVW_CORE_API std::string camelCaseToHeader(const std::string& s);

/**
 * \brief Case insensitive equal comparison of two strings.
 * @return true if all the elements in the two containers are the same.
 * @see CaseInsensitiveCompare
 */
IVW_CORE_API bool iCaseCmp(const std::string& l, const std::string& r);
/**
 * \brief Case insensitive alphabetical less than comparison of two strings, i.e. "aa" < "ab" =
 * true. Compares letters from left to right using std::lexicographical_compare.
 * @return true if the alphabetical order in the first string is less than the second string.
 */
IVW_CORE_API bool iCaseLess(const std::string& l, const std::string& r);
/**
 * \brief Case insensitive equal comparison of two strings to be used for template arguments.
 * @see iCaseCmp
 */
struct IVW_CORE_API CaseInsensitiveCompare {
    bool operator()(const std::string& a, const std::string& b) const;
};

/**
 * \brief convert the given duration from milliseconds to a string.
 * The returned string will have the format "%dd %dh %dmin %dsec %.3fms", where days, hours,
 * minutes, seconds, ... are suppressed up to the first non-zero unit if not needed. Milliseconds
 * and seconds are combined if larger than 1 second.
 *
 * @param ms  in milliseconds
 * @param includeZeros   if true, time units for zero values are always shown, e.g.
 *                       "2d 0h 0min 23s" vs. "2d 23s" and "2h 0min 0s" vs. "2h"
 * @param spacing   if true, a space is inserted between digits and units
 * @return duration formatted as string
 */
IVW_CORE_API std::string msToString(double ms, bool includeZeros = true, bool spacing = false);

/**
 * \brief convenience function for converting a std::chrono::duration to a string calling
 * msToString(double).
 *
 * @param duration       duration
 * @param includeZeros   if true, time units for zero values are always shown, e.g.
 *                       "2d 0h 0min 23s" vs. "2d 23s" and "2h 0min 0s" vs. "2h"
 * @param spacing   if true, a space is inserted between digits and units
 * @return duration formatted as string
 */
template <class Rep, class Period = std::ratio<1>>
std::string durationToString(std::chrono::duration<Rep, Period> duration, bool includeZeros = true,
                             bool spacing = false) {
    using milliseconds = std::chrono::duration<double, std::milli>;
    auto ms = std::chrono::duration_cast<milliseconds>(duration);
    return msToString(ms.count(), includeZeros, spacing);
}

}  // namespace inviwo

#endif  // IVW_STRINGCONVERSION_H
