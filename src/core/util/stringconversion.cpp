/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2021 Inviwo Foundation
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

#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/safecstr.h>

#include <inviwo/core/util/demangle.h>
#include <inviwo/core/util/chronoutils.h>

#include <random>
#include <iomanip>
#include <clocale>
#include <algorithm>

#if defined(__clang__) || defined(__GNUC__)
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#include <cwchar>
#elif defined(WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <fmt/format.h>

namespace inviwo {

namespace util {

std::wstring toWstring(std::string_view str) {
#if defined(_WIN32)
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
    std::wstring result(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], size_needed);
    return result;
#else
    auto state = std::mbstate_t();
    SafeCStr safestr{str};
    auto sptr = safestr.c_str();
    const char* loc = nullptr;
    size_t len = std::mbsrtowcs(nullptr, &sptr, 0, &state);
    if (len == static_cast<std::size_t>(-1)) {
        loc = std::setlocale(LC_CTYPE, nullptr);
        std::setlocale(LC_CTYPE, "en_US.UTF-8");
        len = std::mbsrtowcs(nullptr, &sptr, 0, &state);
        if (len == static_cast<std::size_t>(-1)) {
            if (loc) std::setlocale(LC_CTYPE, loc);
            throw Exception("Invalid unicode sequence", IVW_CONTEXT_CUSTOM("String Conversion"));
        }
    }
    std::wstring result(len, 0);
    std::mbsrtowcs(result.data(), &sptr, result.size(), &state);
    if (loc) std::setlocale(LC_CTYPE, loc);
    return result;
#endif
}

std::string fromWstring(std::wstring_view str) {
#if defined(_WIN32)
    int s_size = static_cast<int>(str.size());
    if (s_size == 0) {  // WideCharToMultiByte does not support zero length, handle separately.
        return {};
    }

    int length = WideCharToMultiByte(CP_UTF8, 0, str.data(), s_size, nullptr, 0, nullptr, nullptr);
    if (length == 0) {
        throw Exception(IVW_CONTEXT_CUSTOM("String Conversion"),
                        "Invalid string conversion Error:{}", GetLastError());
    }

    std::string result(length, 0);
    length = WideCharToMultiByte(CP_UTF8, 0, str.data(), s_size, result.data(), length, nullptr,
                                 nullptr);
    if (length == 0) {
        throw Exception(IVW_CONTEXT_CUSTOM("String Conversion"),
                        "Invalid string conversion Error:{}", GetLastError());
    }
    return result;
#else
    auto state = std::mbstate_t();
    std::wstring safestr(str);
    const wchar_t* sptr = safestr.data();
    const char* loc = nullptr;
    size_t len = std::wcsrtombs(nullptr, &sptr, 0, &state);
    if (len == static_cast<std::size_t>(-1)) {
        loc = std::setlocale(LC_CTYPE, nullptr);
        std::setlocale(LC_CTYPE, "en_US.UTF-8");
        len = std::wcsrtombs(nullptr, &sptr, 0, &state);
        if (len == static_cast<std::size_t>(-1)) {
            if (loc) std::setlocale(LC_CTYPE, loc);
            throw Exception("Invalid unicode sequence", IVW_CONTEXT_CUSTOM("String Conversion"));
        }
    }
    std::string result(len, 0);
    std::wcsrtombs(result.data(), &sptr, result.size(), &state);
    if (loc) std::setlocale(LC_CTYPE, loc);
    return result;
#endif
}

bool iCaseEndsWith(std::string_view str, std::string_view suffix) {
    return str.size() >= suffix.size() &&
           // Compare last part of path with the extension
           iCaseCmp(str.substr(str.size() - suffix.size()), suffix);
}

std::string elideLines(std::string_view str, std::string_view abbrev, size_t maxLineLength) {
    std::string result;
    if (str.empty()) return result;
    const auto maxSize = maxLineLength - abbrev.size();
    util::forEachStringPart(str, "\n", [&](std::string_view line) {
        if (line.size() > maxLineLength) {
            result.append(line.substr(0, maxSize));
            result.append(abbrev);
        } else {
            result.append(line);
        }
        result.push_back('\n');
    });
    result.pop_back();
    return result;
}

}  // namespace util

std::vector<std::string> util::splitString(std::string_view str, char delimiter) {
    std::vector<std::string> output;
    size_t first = 0;

    while (first < str.size()) {
        const auto second = str.find_first_of(delimiter, first);

        if (first != second) output.emplace_back(str.substr(first, second - first));

        if (second == std::string_view::npos) break;

        first = second + 1;
    }

    return output;
}

std::vector<std::string_view> util::splitStringView(std::string_view str, char delimiter) {
    std::vector<std::string_view> output;
    size_t first = 0;

    while (first < str.size()) {
        const auto second = str.find_first_of(delimiter, first);

        if (first != second) output.emplace_back(str.substr(first, second - first));

        if (second == std::string_view::npos) break;

        first = second + 1;
    }

    return output;
}

std::vector<std::string> splitStringWithMultipleDelimiters(std::string_view str,
                                                           std::vector<char> delimiters) {
    std::vector<std::string> output;
    size_t first = 0;
    auto delim = std::string_view{delimiters.data(), delimiters.size()};

    while (first < str.size()) {
        const auto second = str.find_first_of(delim, first);

        if (first != second) output.emplace_back(str.substr(first, second - first));

        if (second == std::string_view::npos) break;

        first = second + 1;
    }

    return output;
}

std::string removeFromString(std::string str, char char_to_remove) {
    str.erase(std::remove(str.begin(), str.end(), char_to_remove), str.end());
    return str;
}

void replaceInString(std::string& str, std::string_view oldStr, std::string_view newStr) {
    size_t pos = 0;

    while ((pos = str.find(oldStr, pos)) != std::string::npos) {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

namespace {

constexpr bool isdigit(char ch) { return '0' <= ch && ch <= '9'; }
constexpr bool islower(char ch) { return 'a' <= ch && ch <= 'z'; }
constexpr bool isupper(char ch) { return 'A' <= ch && ch <= 'Z'; }
constexpr bool isalnum(char ch) { return islower(ch) || isupper(ch) || isdigit(ch); }

constexpr std::array<bool, 256> urlEscapeMap = []() {
    std::array<bool, 256> map{};
    const std::string_view urlValid = "~-_.+!*(),%#@?=;:/,+$";
    for (int i = 0; i < 256; ++i) {
        char c = static_cast<char>(i);
        map[i] = !isalnum(c) && urlValid.find(c) == std::string_view::npos;
    }
    return map;
}();
}  // namespace

void util::urlEncodeTo(std::string_view text, StrBuffer& strBuffer) {
    constexpr std::array<char, 16> hex = {'0', '1', '2', '3', '4', '5', '6', '7',
                                          '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    for (auto c : text) {
        if (urlEscapeMap[static_cast<unsigned>(c)]) {
            strBuffer.buff.push_back('%');
            strBuffer.buff.push_back(hex[(static_cast<unsigned>(c) >> 4) & 0xf]);
            strBuffer.buff.push_back(hex[(static_cast<unsigned>(c) >> 0) & 0xf]);
        } else {
            strBuffer.buff.push_back(c);
        }
    }
}

std::string util::urlEncode(std::string_view text) {
    StrBuffer strBuffer;
    urlEncodeTo(text, strBuffer);
    return std::string{strBuffer.view()};
}

void util::htmlEncodeTo(std::string_view data, StrBuffer& strBuffer) {
    for (size_t pos = 0; pos != data.size(); ++pos) {
        switch (data[pos]) {
            case '&':
                strBuffer.append("&amp;");
                break;
            case '\"':
                strBuffer.append("&quot;");
                break;
            case '\'':
                strBuffer.append("&apos;");
                break;
            case '<':
                strBuffer.append("&lt;");
                break;
            case '>':
                strBuffer.append("&gt;");
                break;
            default:
                strBuffer.buff.push_back(data[pos]);
                break;
        }
    }
}

std::string util::htmlEncode(std::string_view text) {
    StrBuffer strBuffer;
    htmlEncodeTo(text, strBuffer);
    return std::string{strBuffer.view()};
}

std::string toUpper(std::string_view str) {
    std::string res;
    res.reserve(str.size());
    std::transform(str.begin(), str.end(), std::back_inserter(res),
                   [](unsigned char c) { return static_cast<unsigned char>(std::toupper(c)); });
    return res;
}

std::string toLower(std::string_view str) {
    std::string res;
    res.reserve(str.size());
    std::transform(str.begin(), str.end(), std::back_inserter(res),
                   [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
    return res;
}

size_t countLines(std::string_view str) { return std::count(str.begin(), str.end(), '\n') + 1; }

std::string randomString(size_t length) {
    constexpr std::string_view possibleValues =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(possibleValues.size()) - 1);

    std::string s(length, ' ');
    for (auto& c : s) c = possibleValues[dis(gen)];

    return s;
}

std::string dotSeperatedToPascalCase(std::string_view s) {
    std::stringstream ss;
    util::forEachStringPart(s, ".", [&](std::string_view elem) {
        if (elem.size() > 0) ss << static_cast<char>(std::toupper(elem[0]));
        if (elem.size() > 1) ss << elem.substr(1);
    });
    return ss.str();
}

std::string camelCaseToHeader(std::string_view s) {
    if (s.empty()) return {};
    std::stringstream ss;
    char previous = ' ';
    for (auto c : s) {
        if (std::isalpha(c) && std::tolower(previous) == previous && std::toupper(c) == c)
            ss << " ";
        ss << c;
        previous = c;
    }
    auto str{ss.str()};
    str[0] = static_cast<char>(std::toupper(str[0]));
    return str;
}

bool iCaseCmp(std::string_view l, std::string_view r) {
    return std::equal(l.cbegin(), l.cend(), r.cbegin(), r.cend(),
                      [](std::string_view::value_type l1, std::string_view::value_type r1) {
                          return std::tolower(l1) == std::tolower(r1);
                      });
}

bool iCaseLess(std::string_view l, std::string_view r) {
    return std::lexicographical_compare(
        l.cbegin(), l.cend(), r.cbegin(), r.cend(),
        [](std::string_view::value_type l1, std::string_view::value_type r1) {
            return std::tolower(l1) < std::tolower(r1);
        });
}

// trim from start
std::string ltrim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](auto c) {
                return !std::isspace(static_cast<unsigned char>(c));
            }));
    return s;
}

// trim from end
std::string rtrim(std::string s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](auto c) { return !std::isspace(static_cast<unsigned char>(c)); })
                .base(),
            s.end());
    return s;
}

// trim from both ends
std::string trim(std::string s) { return ltrim(rtrim(s)); }

std::string removeSubString(std::string_view str, std::string_view strToRemove) {
    std::string newString(str);
    size_t pos;
    while ((pos = newString.find(strToRemove)) != std::string::npos) {
        newString.erase(pos, strToRemove.length());
    }
    return newString;
}

bool CaseInsensitiveCompare::operator()(std::string_view a, std::string_view b) const {
    return iCaseLess(a, b);
}

std::string parseTypeIdName(const char* name) { return util::parseTypeIdName(name); }

std::string msToString(double ms, bool includeZeros, bool spacing) {
    return util::msToString(ms, includeZeros, spacing);
}

}  // namespace inviwo
