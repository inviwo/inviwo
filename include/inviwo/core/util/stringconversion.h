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

#ifndef IVW_STRINGCONVERSION_H
#define IVW_STRINGCONVERSION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
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

IVW_CORE_API std::vector<std::string> splitString(const std::string& str, char delimeter = ' ');

template <typename T>
std::string joinString(const std::vector<T>& str, std::string delimeter = " ") {
    std::stringstream ss;
    for (int i = 0; i < static_cast<int>(str.size()) - 1; ++i) {
        ss << str[i] << delimeter;
    }
    if (!str.empty()) ss << str.back();

    return ss.str();
}


template <typename Iterator>
std::string joinString(Iterator begin, Iterator end, std::string delimeter = " ") {
    std::stringstream ss;
    while (begin != end - 1) {
        ss << *begin << delimeter;
        ++begin;
    }
    ss << *begin;
    return ss.str();
}

IVW_CORE_API std::vector<std::string> splitStringWithMultipleDelimiters(
    const std::string& str, std::vector<char> delimiters = std::vector<char>());

IVW_CORE_API std::string removeSubString(const std::string& str, const std::string& strToRemove);


IVW_CORE_API std::string removeFromString(std::string str, char char_to_remove = ' ');
IVW_CORE_API void replaceInString(std::string& str, const std::string& oldStr,
                                  const std::string& newStr);
IVW_CORE_API std::string parseTypeIdName(std::string str);

IVW_CORE_API std::string toUpper(std::string str);
IVW_CORE_API std::string toLower(std::string str);

IVW_CORE_API unsigned int countLines(std::string str);

IVW_CORE_API std::string randomString(unsigned int length = 10);

// trim from start
IVW_CORE_API std::string ltrim(std::string s);
// trim from end
IVW_CORE_API std::string rtrim(std::string s);
// trim from both ends
IVW_CORE_API std::string trim(std::string s);

}  // namespace

#endif  // IVW_STRINGCONVERSION_H
