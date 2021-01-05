/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/util/stringconversion.h>

#include <string>
#include <string_view>

using namespace std::string_view_literals;

namespace inviwo {

TEST(StringConversion, fromWstring) {
    const std::wstring wstr = L"widestr\U000000E5\U000000E4\U000000F6\U0001F609";
    const std::string u8str = u8"widestr\U000000E5\U000000E4\U000000F6\U0001F609";
    const auto conv = util::fromWstring(wstr);
    EXPECT_EQ(u8str, conv);
}

TEST(StringConversion, toWstring) {
    const std::wstring wstr = L"widestr\U000000E5\U000000E4\U000000F6\U0001F609";
    const std::string u8str = u8"widestr\U000000E5\U000000E4\U000000F6\U0001F609";
    const auto conv = util::toWstring(u8str);
    EXPECT_EQ(wstr, conv);
}

TEST(String, ltrim) {
    EXPECT_EQ(std::string(""), ltrim("  "));
    EXPECT_EQ(std::string(""), ltrim("\t"));
    EXPECT_EQ(std::string(""), ltrim("\t  "));
    EXPECT_EQ(std::string(""), ltrim("\t  \t"));

    EXPECT_EQ(std::string("a"), ltrim("a"));
    EXPECT_EQ(std::string("a  b"), ltrim("a  b"));

    EXPECT_EQ(std::string("a"), ltrim("  a"));
    EXPECT_EQ(std::string("a  b"), ltrim("  a  b"));
    EXPECT_EQ(std::string("a  "), ltrim("  a  "));
    EXPECT_EQ(std::string("a  b  "), ltrim("  a  b  "));

    EXPECT_EQ(std::string("a"), ltrim("\ta"));
    EXPECT_EQ(std::string("a  "), ltrim("\ta  "));
    EXPECT_EQ(std::string("a"), ltrim("\ra"));
    EXPECT_EQ(std::string("a"), ltrim("  \r a"));
}

TEST(String, rtrim) {
    EXPECT_EQ(std::string(""), trim("  "));
    EXPECT_EQ(std::string(""), trim("\t"));
    EXPECT_EQ(std::string(""), trim("\t  "));
    EXPECT_EQ(std::string(""), trim("\t  \t"));

    EXPECT_EQ(std::string("a"), rtrim("a"));
    EXPECT_EQ(std::string("a  b"), rtrim("a  b"));

    EXPECT_EQ(std::string("a"), rtrim("a  "));
    EXPECT_EQ(std::string("a  b"), rtrim("a  b  "));
    EXPECT_EQ(std::string("  a"), rtrim("  a  "));
    EXPECT_EQ(std::string("  a  b"), rtrim("  a  b  "));

    EXPECT_EQ(std::string("a"), rtrim("a\t"));
    EXPECT_EQ(std::string("  a"), rtrim("  a\t"));
    EXPECT_EQ(std::string("a"), rtrim("a\r"));
    EXPECT_EQ(std::string("a"), rtrim("a  \r "));
}

TEST(String, trim) {
    EXPECT_EQ(std::string(""), trim("  "));
    EXPECT_EQ(std::string(""), trim("\t"));
    EXPECT_EQ(std::string(""), trim("\t  "));
    EXPECT_EQ(std::string(""), trim("\t  \t"));

    EXPECT_EQ(std::string("a"), trim("a"));
    EXPECT_EQ(std::string("a  b"), trim("a  b"));

    EXPECT_EQ(std::string("a"), trim("  a"));
    EXPECT_EQ(std::string("a  b"), trim("  a  b"));
    EXPECT_EQ(std::string("a"), trim("  a  "));
    EXPECT_EQ(std::string("a  b"), trim("  a  b  "));

    EXPECT_EQ(std::string("a"), trim("\ta"));
    EXPECT_EQ(std::string("a"), trim("\ta  "));
    EXPECT_EQ(std::string("a"), trim("\ra\t"));
    EXPECT_EQ(std::string("a"), trim("  \r a  \r"));
}

TEST(StringView, constexpr_trim) {
    constexpr std::string_view result(util::trim(std::string_view("  a  b   "sv)));
    EXPECT_EQ("a  b"sv, result);
}

TEST(StringView, trim) {
    EXPECT_EQ(""sv, util::trim("  "sv));
    EXPECT_EQ(""sv, util::trim("\t"sv));
    EXPECT_EQ(""sv, util::trim("\t  "sv));
    EXPECT_EQ(""sv, util::trim("\t  \t"sv));

    EXPECT_EQ("a"sv, util::trim("a"sv));
    EXPECT_EQ("a  b"sv, util::trim("a  b"sv));

    EXPECT_EQ("a"sv, util::trim("  a"sv));
    EXPECT_EQ("a  b"sv, util::trim("  a  b"sv));
    EXPECT_EQ("a"sv, util::trim("  a  "sv));
    EXPECT_EQ("a  b"sv, util::trim("  a  b  "sv));

    EXPECT_EQ("a"sv, util::trim("\ta"sv));
    EXPECT_EQ("a"sv, util::trim("\ta  "sv));
    EXPECT_EQ("a"sv, util::trim("\ra\t"sv));
    EXPECT_EQ("a"sv, util::trim("  \r a  \r"sv));
}

}  // namespace inviwo
