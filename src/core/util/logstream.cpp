/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <inviwo/core/util/logstream.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

LogStream::LogStream(std::ostream& stream, std::string source, LogLevel level, LogAudience audience)
    : source_{source}
    , level_{level}
    , audience_{audience}
    , buffer_{}
    , stream_(stream)
    , orgBuffer_{stream_.rdbuf(this)} {}

LogStream::~LogStream() {
    sync();
    stream_.rdbuf(orgBuffer_);
}

namespace {
constexpr std::string_view bold = "\33[0;1m";
constexpr std::string_view grey = "\33[2;1m";
constexpr std::string_view italic = "\33[3;1m";
constexpr std::string_view underlined = "\33[4;1m";
constexpr std::string_view flashing = "\33[5;1m";
constexpr std::string_view inverted = "\33[7;1m";
constexpr std::string_view strikethrough = "\33[9;1m";
constexpr std::string_view darkgrey = "\33[30;1m";
constexpr std::string_view red = "\33[31;1m";
constexpr std::string_view green = "\33[32;1m";
constexpr std::string_view yellow = "\33[33;1m";
constexpr std::string_view blue = "\33[34;1m";
constexpr std::string_view pink = "\33[35;1m";
constexpr std::string_view lightblue = "\33[36;1m";
constexpr std::string_view brightwhite = "\33[37;1m";
constexpr std::string_view endcolor = "\33[0m";
}  // namespace

std::streamsize LogStream::xsputn(const char* s, std::streamsize count) {
    std::scoped_lock lock{mutex_};
    std::copy(s, s + count, std::back_inserter(buffer_));
    const auto found = buffer_.find_last_of('\n');
    if (found != std::string::npos) {
        auto line = buffer_.substr(0, found);
        for (auto&& elem :
             {bold, grey, italic, underlined, flashing, inverted, strikethrough, darkgrey, red,
              green, yellow, blue, pink, lightblue, brightwhite, endcolor}) {
            replaceInString(line, elem, "");
        }

        LogCentral::getPtr()->log(source_, level_, audience_, "", "", 0, line);
        buffer_.erase(buffer_.begin(), buffer_.begin() + found + 1);
    }

    return count;
}

int LogStream::sync() {
    std::scoped_lock lock{mutex_};
    if (!buffer_.empty()) {
        for (auto&& elem :
             {bold, grey, italic, underlined, flashing, inverted, strikethrough, darkgrey, red,
              green, yellow, blue, pink, lightblue, brightwhite, endcolor}) {
            replaceInString(buffer_, elem, "");
        }
        LogCentral::getPtr()->log(source_, level_, audience_, "", "", 0, buffer_);
        buffer_.clear();
    }
    return 0;
}

}  // namespace inviwo
