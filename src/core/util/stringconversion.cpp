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

#include <inviwo/core/util/stringconversion.h>

#include <random>
#include <iomanip>

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

namespace inviwo {

namespace util {

#if defined(_WIN32)
std::wstring toWstring(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
    std::wstring result(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], size_needed);
    return result;
}
#else
std::wstring toWstring(const std::string& str) {
    auto state = std::mbstate_t();
    const char* s = str.c_str();
    size_t len = std::mbsrtowcs(nullptr, &s, 0, &state) + 1;
    std::wstring result(len, 0);
    std::mbsrtowcs(&result[0], &s, result.size(), &state);
    return result;
}
#endif

}  // namespace util

std::vector<std::string> splitString(const std::string& str, char delimeter) {
    std::vector<std::string> strings;
    std::stringstream stream(str);
    std::string part;

    while (std::getline(stream, part, delimeter)) strings.push_back(part);

    return strings;
}

std::vector<std::string> splitStringWithMultipleDelimiters(const std::string& str,
                                                           std::vector<char> delimiters) {
    if (!delimiters.size()) {
        // adding default delimiters
        delimiters.push_back('_');
        delimiters.push_back('+');
        delimiters.push_back('-');
        delimiters.push_back('.');
        delimiters.push_back(' ');
    }

    std::string tempString = str;
    char lastDelimiter = delimiters[delimiters.size() - 1];

    for (size_t i = 0; i < delimiters.size() - 1; i++)
        replaceInString(tempString, toString(delimiters[i]), toString(lastDelimiter));

    return splitString(tempString, lastDelimiter);
}

std::string removeFromString(std::string str, char char_to_remove) {
    str.erase(std::remove(str.begin(), str.end(), char_to_remove), str.end());
    return str;
}

void replaceInString(std::string& str, const std::string& oldStr, const std::string& newStr) {
    size_t pos = 0;

    while ((pos = str.find(oldStr, pos)) != std::string::npos) {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

std::string htmlEncode(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for (size_t pos = 0; pos != data.size(); ++pos) {
        switch (data[pos]) {
            case '&':
                buffer.append("&amp;");
                break;
            case '\"':
                buffer.append("&quot;");
                break;
            case '\'':
                buffer.append("&apos;");
                break;
            case '<':
                buffer.append("&lt;");
                break;
            case '>':
                buffer.append("&gt;");
                break;
            default:
                buffer.append(&data[pos], 1);
                break;
        }
    }
    return buffer;
}

std::string parseTypeIdName(std::string str) {

#if defined(__clang__) || defined(__GNUC__)
    struct handle {
        char* p;
        handle(char* ptr) : p(ptr) {}
        ~handle() { std::free(p); }
    };
    const char* cstr = str.c_str();
    int status = -4;
    handle result(abi::__cxa_demangle(cstr, nullptr, nullptr, &status));
    if (status == 0) str = result.p;
#else
    replaceInString(str, "class", "");
    replaceInString(str, "const", "");
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64)
    replaceInString(str, "__ptr64", "");
#endif
#endif
    replaceInString(str, "inviwo::", "");
    return removeFromString(removeFromString(str, '*'), ' ');
}

std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return static_cast<unsigned char>(std::toupper(c)); });
    return str;
}

std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
    return str;
}

size_t countLines(std::string str) {
    size_t lineCount = 1;
    size_t position = 0;

    while (position < str.length()) {
        if (str.substr(position, 1) == "\n") lineCount++;

        position++;
    }

    return lineCount;
}

static const std::string possibleValues =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
std::string randomString(size_t length) {
    std::string s;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(possibleValues.size()) - 1);

    while (s.size() < length) s += possibleValues[dis(gen)];

    return s;
}

// trim from start
std::string ltrim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](auto c) {
                return !std::isspace(static_cast<unsigned char>(c));
            }));
    return s;
}

std::string dotSeperatedToPascalCase(const std::string& s) {
    std::stringstream ss;
    for (auto elem : splitString(s, '.')) {
        elem[0] = static_cast<char>(std::toupper(elem[0]));
        ss << elem;
    }
    return ss.str();
}

std::string camelCaseToHeader(const std::string& s) {
    if (s.empty()) return s;
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

bool iCaseCmp(const std::string& l, const std::string& r) {
    return l.size() == r.size() &&
           std::equal(l.cbegin(), l.cend(), r.cbegin(),
                      [](std::string::value_type l1, std::string::value_type r1) {
                          return std::tolower(l1) == std::tolower(r1);
                      });
}

bool iCaseLess(const std::string& l, const std::string& r) {
    return std::lexicographical_compare(l.cbegin(), l.cend(), r.cbegin(), r.cend(),
                                        [](std::string::value_type l1, std::string::value_type r1) {
                                            return std::tolower(l1) < std::tolower(r1);
                                        });
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

std::string removeSubString(const std::string& str, const std::string& strToRemove) {
    std::string newString(str);
    size_t pos;
    while ((pos = newString.find(strToRemove)) != std::string::npos) {
        newString.erase(pos, strToRemove.length());
    }
    return newString;
}

std::string msToString(double ms, bool includeZeros, bool spacing) {
    const std::string space = (spacing ? " " : "");
    std::stringstream ss;
    bool follows = false;

    size_t days = static_cast<size_t>(ms / (1000.0 * 3600.0 * 24.0));
    if (days > 0 || (follows && includeZeros)) {
        follows = true;
        ss << days << space << "d";
        ms -= 3600 * 1000 * 24 * days;
    }
    size_t hours = static_cast<size_t>(ms / (1000.0 * 3600.0));
    if (hours > 0 || (follows && includeZeros)) {
        if (follows) ss << " ";
        follows = true;
        ss << hours << space << "h";
        ms -= 3600 * 1000 * hours;
    }
    size_t minutes = static_cast<size_t>(ms / (1000.0 * 60.0));
    if (minutes > 0 || (follows && includeZeros)) {
        if (follows) ss << " ";
        follows = true;
        ss << minutes << space << "min";
        ms -= 60 * 1000 * minutes;
    }
    size_t seconds = static_cast<size_t>(ms / 1000.0);
    // combine seconds and milliseconds, iff there already something added to the string stream
    // _or_ there are more than one second
    if (seconds > 0 || (follows && includeZeros)) {
        if (follows) ss << " ";
        follows = true;
        ss << std::setprecision(4) << (ms / 1000.0) << space << "s";
    } else {
        // less than one second, no leading minutes/hours
        ss << std::setprecision(4) << ms << space << "ms";
    }

    return ss.str();
}

bool CaseInsensitiveCompare::operator()(const std::string& a, const std::string& b) const {
    return iCaseLess(a, b);
}

}  // namespace inviwo
