/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <inviwo/core/util/staticstring.h>

#include <fmt/format.h>

namespace inviwo {

TEST(StaticString, Empty) {
    constexpr StaticString empty{};
    static_assert(empty.size() == 0);
}

TEST(StaticString, String) {
    constexpr StaticString str{"test"};
    static_assert(str.size() == 4);
}

TEST(StaticString, Concat) {
    constexpr StaticString str1{"test1"};
    constexpr StaticString str2{"test2"};
    constexpr StaticString str3{str1, str2, "test3"};
    static_assert(str3.size() == str1.size() + str2.size() + 5);
    static_assert(str3 == "test1test2test3");
}

TEST(StaticString, Plus) {
    constexpr StaticString str2{"test2"};
    constexpr StaticString str3 = "test1" + str2 + "test3";
    static_assert(str3.size() == 5 + str2.size() + 5);
    static_assert(str3 == "test1test2test3");
}

TEST(StaticString, Format) {
    constexpr StaticString str1{"test2"};
    const auto str2 = fmt::format("{}", str1);
    EXPECT_EQ(str1, str2);
}

}  // namespace inviwo
