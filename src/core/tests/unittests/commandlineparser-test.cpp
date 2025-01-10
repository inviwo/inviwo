/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <inviwo/core/util/commandlineparser.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo {

TEST(CommandLineParserTest, DefaultTest) {
    const int argc = 1;
    const char* argv[argc] = {"unittests.exe"};
    CommandLineParser clp(argc, const_cast<char**>(argv));
    EXPECT_EQ(std::string{}, clp.getOutputPath().generic_string());
    EXPECT_EQ(std::string{}, clp.getWorkspacePath().generic_string());
    EXPECT_FALSE(clp.getQuitApplicationAfterStartup());
    EXPECT_FALSE(clp.getLoadWorkspaceFromArg());
    EXPECT_TRUE(clp.getShowSplashScreen());
}

TEST(CommandLineParserTest, CommandLineParserTest) {
    const int argc = 3;
    const char* argv[argc] = {"unittests.exe", "-w", "C:/Just/A/Path/"};
    CommandLineParser clp(argc, const_cast<char**>(argv));
    EXPECT_EQ(std::string{}, clp.getOutputPath().generic_string());
    EXPECT_EQ(std::string{"C:/Just/A/Path/"}, clp.getWorkspacePath().generic_string());
    EXPECT_FALSE(clp.getQuitApplicationAfterStartup());
    EXPECT_TRUE(clp.getLoadWorkspaceFromArg());
    EXPECT_TRUE(clp.getShowSplashScreen());
}

}  // namespace inviwo
