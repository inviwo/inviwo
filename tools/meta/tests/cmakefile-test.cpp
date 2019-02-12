/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <string>
#include <sstream>

#include <inviwo/meta/iter/range.hpp>
#include <inviwo/meta/cmake/cmakefile.hpp>
#include <filesystem>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

using namespace inviwo::meta::cmake;
using namespace std::literals;

TEST(Cmakefile, command_simple) {

    const auto in = R"(set(a b)
)"sv;
    auto cmakefile = CMakeFile(in);

    EXPECT_EQ(std::get<Command>(cmakefile.items[0]).identifier, "set");

    std::stringstream ss;
    cmakefile.print(ss);

    EXPECT_EQ(ss.str(), in);
}

TEST(Cmakefile, command_large) {
    const auto in = R"(
set()
# comment
set(var "test")
if(${test})
    message(${test} "bla" var)
endif()
set(source_files
    file\n1.cpp
    file2.cpp #comment
    (file2.cpp file3.cpp)
    [[bla]]
    file4.cpp
)
#[[
    test1
    test2
]]

)"sv;

    auto cmakefile = CMakeFile(in);

    EXPECT_EQ(std::get<Command>(cmakefile.items[1]).identifier, "set");

    std::stringstream ss;
    cmakefile.print(ss);

    EXPECT_EQ(ss.str(), in);
}

TEST(Cmakefile, range) {
    const auto in = R"(
set(source_files
    file.cpp
)
#[[test]]
set(header_files
    file.cpp
)
#Comment
set(test_files
    file.cpp
)
)"sv;

    auto cmakefile = CMakeFile(in);

    for (auto& cmd : cmakefile.commands()) {
        EXPECT_EQ(cmd.identifier, "set");

        auto args = inviwo::meta::Range{++(cmd.args().begin()), cmd.args().end()};
        for (auto& arg : args) {
            EXPECT_EQ(arg.value, "file.cpp");
            EXPECT_EQ(arg.value, "file.cpp");
        }
    }
}
