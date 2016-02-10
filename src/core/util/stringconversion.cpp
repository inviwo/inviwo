/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#if defined(__clang__) || defined(__GNUC__)
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

namespace inviwo {

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
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&data[pos], 1); break;
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
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), (int(*)(int))std::tolower);
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
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
std::string rtrim(std::string s) {
    s.erase(
        std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
        s.end());
    return s;
}

// trim from both ends
std::string trim(std::string s) { return ltrim(rtrim(s)); }

IVW_CORE_API std::string removeSubString(const std::string& str, const std::string& strToRemove) {
    std::string newString(str);
    size_t pos;
    while ((pos = newString.find(strToRemove)) != std::string::npos) {
        newString.erase(pos, strToRemove.length());
    }
    return newString;
}

}  // namespace