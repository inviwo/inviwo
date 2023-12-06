/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2023 Inviwo Foundation
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

#include <inviwo/core/util/shuntingyard.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/glm.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <fmt/format.h>

namespace inviwo {

namespace {
void print(const std::vector<shuntingyard::Token>& res) {
    fmt::println("Size {}", res.size());
    for (auto& item : res) {
        std::visit(
            util::overloaded{[](shuntingyard::Number n) { fmt::print("{} ", n.value); },
                             [](shuntingyard::Identifier n) { fmt::print("{} ", n.name); },
                             [](shuntingyard::Operator n) { fmt::print("{} ", n.name); },
                             [](shuntingyard::Function n) { fmt::print("{}{} ", n.name, n.nArgs); },
                             [](shuntingyard::Member n) { fmt::print("Get[{}] ", n.name); },
                             [](auto other) { fmt::print("? "); }},
            item);
    }
    fmt::println("\n");
}
}  // namespace

TEST(Shuntingyard, double) {

    auto res = shuntingyard::Calculator::toRPN("1.123");

    ASSERT_EQ(res.size(), 1);
    ASSERT_TRUE(std::holds_alternative<shuntingyard::Number>(res.front()));
    EXPECT_EQ(std::get<shuntingyard::Number>(res.front()).value, 1.123);
}

TEST(Shuntingyard, str) {

    auto res = shuntingyard::Calculator::toRPN("min");

    ASSERT_EQ(res.size(), 1);
    ASSERT_TRUE(std::holds_alternative<shuntingyard::Identifier>(res.front()));
    EXPECT_EQ(std::get<shuntingyard::Identifier>(res.front()).name, "min");
}

TEST(Shuntingyard, expr) {

    auto res = shuntingyard::Calculator::toRPN("min(x,y,3*2).x");
    print(res);
}

TEST(Shuntingyard, exprMore) {

    auto res = shuntingyard::Calculator::toRPN("min(x,y,z)");
    print(res);

    res = shuntingyard::Calculator::toRPN("min(x,y)");
    print(res);

    res = shuntingyard::Calculator::toRPN("min(x)");
    print(res);

    res = shuntingyard::Calculator::toRPN("min()");
    print(res);

    res = shuntingyard::Calculator::toRPN("min(min(x,y), min(x,y,z))");
    print(res);
}

TEST(Shuntingyard, calc) {

    std::string_view expr = "2*-3/2";
    auto res = shuntingyard::Calculator::toRPN(expr);
    print(res);
    fmt::println("{} = {}", expr, std::get<double>(shuntingyard::Calculator::calculate(res, {})));
}

TEST(Shuntingyard, calcMin) {

    std::string_view expr = "min(2,3)";
    auto res = shuntingyard::Calculator::toRPN(expr);
    print(res);
    fmt::println("{} = {}", expr, std::get<double>(shuntingyard::Calculator::calculate(res, {})));
}
TEST(Shuntingyard, calcMax) {

    std::string_view expr = "max(2,3)";
    auto res = shuntingyard::Calculator::toRPN(expr);
    print(res);
    fmt::println("{} = {}", expr, std::get<double>(shuntingyard::Calculator::calculate(res, {})));
}

TEST(Shuntingyard, calcAbs) {

    std::string_view expr = "abs(vec3(-1,2,-3))";
    auto res = shuntingyard::Calculator::toRPN(expr);
    print(res);
    fmt::println("{} = {}", expr, std::get<dvec3>(shuntingyard::Calculator::calculate(res, {})));
}

TEST(Shuntingyard, calcVec) {

    std::string_view expr = "vec3(2,3,4).z";
    auto res = shuntingyard::Calculator::toRPN(expr);
    print(res);
    fmt::println("{} = {}", expr, std::get<double>(shuntingyard::Calculator::calculate(res, {})));
}

TEST(Shuntingyard, shaderVec) {

    std::string_view expr = "vec3(2,3,4+3*(9-3.0)).z";
    auto res = shuntingyard::Calculator::toRPN(expr);
    print(res);
    fmt::println("{} = {}", expr, shuntingyard::Calculator::shaderCode(res, {}, {}));
}

}  // namespace inviwo
