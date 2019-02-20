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

#include <iostream>
#include <string>

#include <inviwo/meta/cmake/grammar.hpp>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

using namespace inviwo::meta;
namespace p = tao::pegtl;

template <typename... Rules>
struct exact : p::seq<p::bof, Rules..., p::eof> {};

template <typename Rule>
bool test(const std::string& input) {
    std::string id;
    return p::parse_tree::parse<exact<Rule>, cmake::selector, cmake::action>(
               p::string_input<>(input, ""), id) != nullptr;
}

TEST(CMakeGrammar, empty) { EXPECT_TRUE(p::parse<exact<>>(p::string_input<>("", ""))); }

TEST(CMakeGrammar, space) {
    EXPECT_TRUE(test<cmake::space>(" "));
    EXPECT_FALSE(test<cmake::space>(" a"));
    EXPECT_TRUE(test<cmake::space>("    "));
    EXPECT_FALSE(test<cmake::space>("    a"));
    EXPECT_FALSE(test<cmake::space>("_"));
}

TEST(CMakeGrammar, line_comment) {
    EXPECT_TRUE(test<cmake::line_comment>("#"));
    EXPECT_TRUE(test<cmake::line_comment>("# str"));
}
TEST(CMakeGrammar, line_ending) {
    EXPECT_TRUE(test<cmake::line_ending>("\n"));
    EXPECT_FALSE(test<cmake::line_ending>("a\n"));
    EXPECT_FALSE(test<cmake::line_ending>("\na"));
    EXPECT_FALSE(test<cmake::line_ending>("\n\n"));
    EXPECT_TRUE(test<cmake::line_ending>("#\n"));
    EXPECT_TRUE(test<cmake::line_ending>("# comment\n"));
}

TEST(CMakeGrammar, identifier) {
    EXPECT_TRUE(test<cmake::identifier>("set"));
    EXPECT_FALSE(test<cmake::identifier>("5set"));
}

TEST(CMakeGrammar, bracket) { EXPECT_TRUE(test<cmake::bracket_argument>("[[test]]")); }

TEST(CMakeGrammar, unquoted_argument) {
    EXPECT_TRUE(test<cmake::unquoted_argument>("arg"));
    EXPECT_FALSE(test<cmake::unquoted_argument>("\targ"));
}
TEST(CMakeGrammar, quoted_element) {
    EXPECT_TRUE(test<cmake::quoted_element>("a"));
    EXPECT_TRUE(test<cmake::quoted_element>(" "));
    EXPECT_TRUE(test<cmake::quoted_element>("\t"));
    EXPECT_TRUE(test<cmake::quoted_element>("\t"));
    EXPECT_FALSE(test<cmake::quoted_element>("\""));
    EXPECT_FALSE(test<cmake::quoted_element>("\\"));
}
TEST(CMakeGrammar, quoted_argument) {
    EXPECT_TRUE(test<cmake::quoted_argument>(R"("a arg \t")"));
    EXPECT_FALSE(test<cmake::quoted_argument>(R"("a arg" \t")"));
}

TEST(CMakeGrammar, argument) {
    EXPECT_TRUE(test<cmake::arguments>("arg"));
    EXPECT_TRUE(test<cmake::arguments>("\"arg\""));
}

TEST(CMakeGrammar, arguments) { EXPECT_TRUE(test<cmake::arguments>("arg1 \"arg2\" ")); }

TEST(CMakeGrammar, command_invocation) {
    EXPECT_TRUE(test<cmake::command>("set()"));
    EXPECT_TRUE(test<cmake::command>("set(arg)"));
    EXPECT_TRUE(test<cmake::command>("set(arg1 \"arg2\")"));
}

TEST(CMakeGrammar, file_element) {
    EXPECT_TRUE(test<cmake::file_element>("set()\n"));
    EXPECT_TRUE(test<cmake::file_element>("set()  \n"));
    EXPECT_TRUE(test<cmake::file_element>("# comment\n"));
    EXPECT_TRUE(test<cmake::file_element>("  \n"));
    EXPECT_TRUE(test<cmake::file_element>("set()# comment\n"));
    EXPECT_TRUE(test<cmake::file_element>("set() # comment\n"));
}

TEST(CMakeGrammar, file) {
    EXPECT_TRUE(test<cmake::file>(R"(
set()
# comment
set(var "test")
it(${test})
    message(${test})
endif()
)"));
}

void print_node(const p::parse_tree::node& n, const std::string& s = "") {
    // detect the root node:
    if (n.is_root()) {
        std::cout << "ROOT" << std::endl;
    } else {
        if (n.has_content()) {
            auto content = n.content();
            std::transform(content.begin(), content.end(), content.begin(),
                           [](char c) { return c == '\n' ? '~' : c; });
            std::cout << s << n.name() << " \"" << content << "\"" << std::endl;
        } else {
            std::cout << s << n.name() << std::endl;
        }
    }
    // print all child nodes
    if (!n.children.empty()) {
        const auto s2 = s + "  ";
        for (auto& up : n.children) {
            print_node(*up, s2);
        }
    }
}

std::ostream& print(std::ostream& os, const p::parse_tree::node& n) {

    if (n.children.empty() && n.has_content()) os << n.content();
    for (auto& up : n.children) {
        print(os, *up);
    }
    return os;
}

TEST(CMakeGrammar, parse) {
    p::string_input<> in(R"(
set()
# comment
set(var "test")
if(${test})
    message(${test} "bla" var)
endif()
set(source_files
    file\n1.cpp
    file2.cpp
    (file2.cpp file3.cpp)
    [[bla]]
    file4.cpp
)
#[[
    test1
    test2
]]
)",
                         "");

    std::string id;
    const auto root = p::parse_tree::parse<cmake::file, cmake::selector, cmake::action>(in, id);
    EXPECT_TRUE(root != nullptr);
    // print_node(*root);
    // print(std::cout, *root);
}
