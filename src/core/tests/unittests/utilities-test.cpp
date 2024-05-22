/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/util/utilities.h>

#include <string>
#include <iostream>

namespace inviwo {

TEST(UtilitiesTests, StripIndentifierTest) {
    EXPECT_EQ("foo", util::stripIdentifier("foo"));
    EXPECT_EQ("foobar", util::stripIdentifier("foo bar"));
    EXPECT_EQ("foo123", util::stripIdentifier("foo123"));
    EXPECT_EQ("foo123_", util::stripIdentifier("foo123_"));
    EXPECT_EQ("foo123", util::stripIdentifier("(foo123)"));
    EXPECT_EQ("_1abc123", util::stripIdentifier("1abc123"));
    EXPECT_EQ("_1abc123", util::stripIdentifier("1abc123&!-=\"#%&/()=?`+@${[]}~*'-.,;:<>|"));
}

TEST(UtilitiesTests, ValidateIdentifierTest) {
    auto context = IvwContextCustom("utilities-test");

    EXPECT_THROW(util::validateIdentifier("", "property", context), Exception);
    EXPECT_THROW(util::validateIdentifier("1foo", "property", context), Exception);
    EXPECT_THROW(util::validateIdentifier("foo-bar", "property", context), Exception);
    EXPECT_NO_THROW(util::validateIdentifier("foobar", "property", context));
    EXPECT_NO_THROW(util::validateIdentifier("f2oobar", "property", context));
    EXPECT_NO_THROW(util::validateIdentifier("_f2oobar", "property", context));
}

}  // namespace inviwo
