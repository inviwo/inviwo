/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

/*
The MIT License (MIT)

Copyright (c) 2013 Brandon Amos <http://bamos.github.io>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// Source:
// http://www.daniweb.com/software-development/cpp/code/427500/calculator-using-shunting-yard-algorithm#
// Author: Jesse Brown
// Modifications: Brandon Amos

#include <inviwo/core/util/shuntingyard.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/safecstr.h>
#include <inviwo/core/util/glm.h>

#include <charconv>

#include <cstdlib>
#include <stdexcept>
#include <cmath>
#include <tuple>

namespace inviwo {

namespace shuntingyard {

namespace {

std::tuple<int, double> tryParseNumber(std::string_view str) {
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
    double result{};
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    if (ec == std::errc{}) {
        return {std::distance(str.data(), ptr), result};
    }
    return {0, 0.0};
#else
    SafeCStr buff{str};
    char* end;
    double result = strtod(buff.c_str(), &end);

    if (end == buff.c_str()) {
        return {0, 0.0};
    }
    return {std::distance(buff.c_str(), static_cast<const char*>(end)), result};
#endif
}

constexpr bool identifierFirst(unsigned char c) { return (c == '_' || std::isalpha(c)); };

constexpr bool identifierRest(unsigned char c) { return (c == '_' || std::isalnum(c)); };

constexpr std::tuple<int, std::string_view> tryParseIdentifier(std::string_view expression) {
    if (identifierFirst(expression.front())) {
        auto it = std::find_if(expression.begin() + 1, expression.end(),
                               [](char c) { return !identifierRest(c); });
        const auto size = std::distance(expression.begin(), it);
        const auto str = expression.substr(0, size);
        return {size, str};
    } else {
        return {0, std::string_view{}};
    }
}

constexpr bool nextIs(std::string_view str, char c, size_t pos = 0) {
    return str.size() > pos && str[pos] == c;
};

template <typename T>
constexpr bool nextIs(const std::vector<Token>& stack, size_t pos = 0) {
    if (stack.size() > pos && std::holds_alternative<T>(stack[stack.size() - pos - 1])) {
        return true;
    }
    return false;
}

template <typename T>
constexpr bool nextIsNot(const std::vector<Token>& stack) {
    if (!stack.empty() && !std::holds_alternative<T>(stack.back())) {
        return true;
    }
    return false;
}

constexpr void expectOpenParen(const std::vector<Token>& stack) {
    if (nextIs<OpenParen>(stack)) {
        return;
    }
    throw Exception("Invalid expression");
}
constexpr Function& getOpenParenAndFunc(std::vector<Token>& stack) {
    if (nextIs<OpenParen>(stack) && nextIs<Function>(stack, 1)) {
        return std::get<Function>(stack[stack.size() - 2]);
    }
    throw Exception("Invalid expression");
}

enum class Associative { Left, Right };

constexpr bool needsOperatorPop(Operator op1, const std::vector<Token>& stack) {
    // https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#operators
    constexpr auto precedence = [](Operator op) {
        if (op.unary) return 3;
        switch (op.name) {
            case '+':
                return 5;
            case '-':
                return 5;
            case '*':
                return 4;
            case '/':
                return 4;
            default:
                return 100;
        }
    };

    constexpr auto associativity = [](Operator op) {
        return op.unary ? Associative::Right : Associative::Left;
    };

    if (nextIs<Operator>(stack)) {
        const auto op2 = std::get<Operator>(stack.back());
        if (precedence(op2) < precedence(op1) ||
            (associativity(op1) == Associative::Left && precedence(op2) == precedence(op1))) {
            return true;
        }
    }
    return false;
}

struct EvalOp {

    char op;
    std::vector<Value>& stack;

    template <typename T1, typename T2>
    constexpr void impl(T1 l, T2 r) const {
        switch (op) {
            case '+':
                stack.push_back(l + r);
                return;
            case '-':
                stack.push_back(l - r);
                return;
            case '*':
                stack.push_back(l * r);
                return;
            case '/':
                stack.push_back(l / r);
                return;
        }
        throw Exception("Invalid equation");
    }

    constexpr void operator()(double s1, double s2) const { return impl(s1, s2); }

    template <size_t N>
    constexpr void operator()(double s, glm::vec<N, double, glm::defaultp> v) const {
        return impl(s, v);
    }

    template <size_t N>
    constexpr void operator()(glm::vec<N, double, glm::defaultp> v, double s) const {
        return impl(v, s);
    }

    template <size_t N>
    constexpr void operator()(glm::vec<N, double, glm::defaultp> v1,
                              glm::vec<N, double, glm::defaultp> v2) const {
        return impl(v1, v2);
    }

    template <typename T1, typename T2>
    constexpr void operator()(T1 l, T2 r) const {
        throw Exception("Invalid equation");
    }
};

template <typename T>
bool has(std::vector<Value>& stack, size_t ind) {
    if (ind <= stack.size()) {
        return std::holds_alternative<T>(stack[stack.size() - ind]);
    } else {
        throw Exception("Invalid expression");
    }
}
template <typename T>
decltype(auto) get(std::vector<Value>& stack, size_t ind) {
    if (ind <= stack.size()) {
        return std::get<T>(stack[stack.size() - ind]);
    } else {
        throw Exception("Invalid expression");
    }
}

template <typename T1, typename F>
bool functionWrap1(const F& func, std::vector<Value>& stack) {
    if (has<T1>(stack, 1)) {
        auto val = func(get<T1>(stack, 1));
        stack.pop_back();
        stack.push_back(val);
        return true;
    }
    return false;
}
template <typename T1, typename T2, typename F>
bool functionWrap2(const F& func, std::vector<Value>& stack) {
    if (has<T1>(stack, 2) && has<T2>(stack, 1)) {
        auto val = func(get<T1>(stack, 2), get<T2>(stack, 1));
        stack.erase(stack.end() - 2, stack.end());
        stack.push_back(val);
        return true;
    }
    return false;
}

void handleFunction(Function f, std::vector<Value>& stack) {
    static constexpr auto abs = [](auto&& a) -> decltype(auto) { return glm::abs(a); };
    static constexpr auto min = [](auto&& a, auto&& b) -> decltype(auto) { return glm::min(a, b); };
    static constexpr auto max = [](auto&& a, auto&& b) -> decltype(auto) { return glm::max(a, b); };

    static std::unordered_map<std::string, std::function<void(size_t, std::vector<Value>&)>,
                              StringHash, std::equal_to<>>

        functions = {
            {"abs",
             [](size_t nArgs, std::vector<Value>& stack) {
                 if (nArgs == 1) {
                     if (functionWrap1<double>(abs, stack)) {
                         return;
                     } else if (functionWrap1<dvec2>(abs, stack)) {
                         return;
                     } else if (functionWrap1<dvec3>(abs, stack)) {
                         return;
                     } else if (functionWrap1<dvec4>(abs, stack)) {
                         return;
                     }
                 }
                 throw Exception("Invalid function");
             }},
            {"min",
             [](size_t nArgs, std::vector<Value>& stack) {
                 if (nArgs == 2) {
                     if (functionWrap2<double, double>(min, stack)) {
                         return;
                     }
                 }
                 throw Exception("Invalid function");
             }},
            {"max",
             [](size_t nArgs, std::vector<Value>& stack) {
                 if (nArgs == 2) {
                     if (functionWrap2<double, double>(max, stack)) {
                         return;
                     }
                 }
                 throw Exception("Invalid function");
             }},
            {"vec3",
             [](size_t nArgs, std::vector<Value>& stack) {
                 if (nArgs == 1) {
                     auto a1 = stack.back();
                     stack.pop_back();
                     if (std::holds_alternative<double>(a1)) {
                         stack.push_back(dvec3(std::get<double>(a1)));
                         return;
                     } else if (std::holds_alternative<dvec3>(a1)) {
                         stack.push_back(dvec3(std::get<dvec3>(a1)));
                         return;
                     } else if (std::holds_alternative<dvec4>(a1)) {
                         stack.push_back(dvec3(std::get<dvec4>(a1)));
                         return;
                     }
                 }
                 if (nArgs == 3) {
                     auto a3 = stack.back();
                     stack.pop_back();
                     auto a2 = stack.back();
                     stack.pop_back();
                     auto a1 = stack.back();
                     stack.pop_back();
                     if (std::holds_alternative<double>(a1) && std::holds_alternative<double>(a2) &&
                         std::holds_alternative<double>(a3)) {
                         stack.push_back(dvec3(std::get<double>(a1), std::get<double>(a2),
                                               std::get<double>(a3)));
                         return;
                     }
                 }
                 throw Exception("Invalid function");
             }}

        };

    if (auto it = functions.find(f.name); it != functions.end()) {
        it->second(f.nArgs, stack);
    } else {
        throw Exception("Invalid function");
    }
}

void handleMember(Member mem, std::vector<Value>& stack) {

    auto a = stack.back();
    stack.pop_back();
    if (mem.name == "x") {
        if (std::holds_alternative<dvec2>(a)) {
            stack.push_back(std::get<dvec2>(a).x);
            return;
        } else if (std::holds_alternative<dvec3>(a)) {
            stack.push_back(std::get<dvec3>(a).x);
            return;
        } else if (std::holds_alternative<dvec4>(a)) {
            stack.push_back(std::get<dvec4>(a).x);
            return;
        }
    } else if (mem.name == "y") {
        if (std::holds_alternative<dvec2>(a)) {
            stack.push_back(std::get<dvec2>(a).y);
            return;
        } else if (std::holds_alternative<dvec3>(a)) {
            stack.push_back(std::get<dvec3>(a).y);
            return;
        } else if (std::holds_alternative<dvec4>(a)) {
            stack.push_back(std::get<dvec4>(a).y);
            return;
        }
    } else if (mem.name == "z") {
        if (std::holds_alternative<dvec3>(a)) {
            stack.push_back(std::get<dvec3>(a).z);
            return;
        } else if (std::holds_alternative<dvec4>(a)) {
            stack.push_back(std::get<dvec4>(a).z);
            return;
        }
    } else if (mem.name == "w") {
        if (std::holds_alternative<dvec4>(a)) {
            stack.push_back(std::get<dvec4>(a).w);
            return;
        }
    }
    throw Exception("Invalid member");
}

}  // namespace

std::vector<Token> Calculator::toRPN(std::string_view expression) {
    std::vector<Token> rpn;
    std::vector<Token> stack;

    // In one pass, ignore whitespace and parse the expression into RPN
    // using Dijkstra's Shunting-yard algorithm.

    bool first = true;

    while (!expression.empty()) {
        switch (expression.front()) {
            case ' ':
                expression.remove_prefix(1);
                break;
            case '\t':
                expression.remove_prefix(1);
                break;
            case '(':
                stack.push_back(OpenParen{});
                expression.remove_prefix(1);
                first = true;
                break;
            case ')': {
                while (nextIsNot<OpenParen>(stack)) {
                    rpn.emplace_back(stack.back());
                    stack.pop_back();
                }
                expectOpenParen(stack);
                stack.pop_back();
                expression.remove_prefix(1);
                if (nextIs<Function>(stack)) {
                    rpn.emplace_back(stack.back());
                    stack.pop_back();
                }
                break;
            }
            case ',':
                while (nextIsNot<OpenParen>(stack)) {
                    rpn.emplace_back(stack.back());
                    stack.pop_back();
                }
                ++(getOpenParenAndFunc(stack).nArgs);

                expression.remove_prefix(1);
                first = true;
                break;

            case '+':
                [[fallthrough]];
            case '-':
                [[fallthrough]];
            case '*':
                [[fallthrough]];
            case '/':
                [[fallthrough]];
            case '^': {
                const auto op = Operator{expression[0], first};
                while (!first && needsOperatorPop(op, stack)) {
                    rpn.emplace_back(stack.back());
                    stack.pop_back();
                }
                stack.push_back(op);
                expression.remove_prefix(1);
                first = true;
                break;
            }
            case '.': {
                expression.remove_prefix(1);
                if (auto [size, str] = tryParseIdentifier(expression); size > 0) {
                    rpn.emplace_back(Member{str});
                    expression.remove_prefix(size);
                } else {
                    throw Exception("Invalid expression");
                }
                break;
            }
            default: {
                if (auto [n, val] = tryParseNumber(expression); n > 0) {
                    rpn.emplace_back(Number{val});
                    expression.remove_prefix(n);
                    first = false;
                } else if (auto [size, str] = tryParseIdentifier(expression); size > 0) {
                    expression.remove_prefix(size);
                    if (nextIs(expression, '(')) {
                        if (nextIs(expression, ')', 1)) {
                            stack.push_back(Function{str, 0});
                            expression.remove_prefix(2);
                        } else {
                            stack.push_back(Function{str});
                        }
                    } else {
                        rpn.emplace_back(Identifier{str});
                        first = false;
                    }
                } else {
                    throw Exception("Invalid expression");
                }
                break;
            }
        }
    }
    while (!stack.empty()) {
        rpn.emplace_back(stack.back());
        stack.pop_back();
    }

    return rpn;
}

Value Calculator::calculate(std::string_view expression, const VariableMap& vars) {
    return calculate(toRPN(expression), vars);
}

Value Calculator::calculate(const std::vector<Token> rpn, const VariableMap& vars) {
    std::vector<Token> pn{rpn.rbegin(), rpn.rend()};

    // Evaluate the expression in RPN form.
    std::vector<Value> stack;
    while (!pn.empty()) {
        std::visit(util::overloaded{[&](Number n) { stack.emplace_back(n.value); },
                                    [&](Identifier n) {
                                        if (auto it = vars.find(n.name); it != vars.end()) {
                                            stack.emplace_back(it->second);
                                        } else {
                                            throw Exception("Invalid equation");
                                        }
                                    },
                                    [&](Operator n) {
                                        if (n.unary) {
                                            switch (n.name) {
                                                case '+':
                                                    break;
                                                case '-':
                                                    std::visit([](auto& value) { value *= -1.0; },
                                                               stack.back());
                                                    break;
                                            }
                                        } else {
                                            auto right = stack.back();
                                            stack.pop_back();
                                            auto left = stack.back();
                                            stack.pop_back();

                                            std::visit(EvalOp{n.name, stack}, left, right);
                                        }
                                    },
                                    [&](Function fun) { handleFunction(fun, stack); },
                                    [&](Member mem) { handleMember(mem, stack); },
                                    [&](auto other) { throw Exception("Invalid equation"); }},
                   pn.back());
        pn.pop_back();
    }

    return stack.back();
}

std::string Calculator::shaderCode(std::string_view expression, const VariableMap& vars,
                                   const SymbolMap& symbols) {
    return shaderCode(toRPN(expression), vars, symbols);
}

std::string Calculator::shaderCode(const std::vector<Token> rpn, const VariableMap& vars,
                                   const SymbolMap& syms) {

    std::vector<Token> pn{rpn.rbegin(), rpn.rend()};

    // Evaluate the expression in RPN form.
    std::vector<std::string> stack;
    while (!pn.empty()) {
        std::visit(
            util::overloaded{[&](Number n) { stack.push_back(fmt::to_string(n.value)); },
                             [&](Identifier n) {
                                 if (auto it1 = vars.find(n.name); it1 != vars.end()) {
                                     stack.push_back(fmt::to_string(it1->second));
                                 } else if (auto it2 = syms.find(n.name); it2 != syms.end()) {
                                     stack.push_back(it2->second);
                                 } else {
                                     throw Exception("Invalid equation");
                                 }
                             },
                             [&](Operator n) {
                                 if (n.unary) {
                                     switch (n.name) {
                                         case '+':
                                             break;
                                         case '-':
                                             stack.back() = fmt::format("-{}", stack.back());
                                             break;
                                     }
                                 } else {
                                     auto str = fmt::format("({} {} {})", stack[stack.size() - 2],
                                                            n.name, stack[stack.size() - 1]);
                                     stack.pop_back();
                                     stack.pop_back();
                                     stack.push_back(std::move(str));
                                 }
                             },
                             [&](Function fun) {
                                 fmt::memory_buffer b;
                                 auto out = std::back_inserter(b);
                                 fmt::format_to(out, "{}(", fun.name);
                                 for (size_t i = fun.nArgs; i > 0; --i) {
                                     if (i != fun.nArgs) fmt::format_to(out, ", ");
                                     fmt::format_to(out, "{}", stack[stack.size() - i]);
                                 }
                                 fmt::format_to(out, ")");

                                 stack.erase(stack.end() - fun.nArgs, stack.end());
                                 stack.emplace_back(std::string_view{b.data(), b.size()});
                             },
                             [&](Member mem) {
                                 auto str = fmt::format("{}.{}", stack.back(), mem.name);
                                 stack.pop_back();
                                 stack.push_back(std::move(str));
                             },
                             [&](auto other) { throw Exception("Invalid equation"); }},
            pn.back());
        pn.pop_back();
    }

    return stack.back();
}
}  // namespace shuntingyard

}  // namespace inviwo
