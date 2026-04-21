/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2026 Inviwo Foundation
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

#include <inviwo/core/algorithm/syntaxhighlight.h>
#include <inviwo/core/util/stringconversion.h>

#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo::util {

namespace {

// ---------------------------------------------------------------------------
// Colour palette (mirrors the GitHub .pl-* CSS classes)
// ---------------------------------------------------------------------------
constexpr std::string_view kKeywordColor  = "#66d9ef";  // .pl-k
constexpr std::string_view kStringColor   = "#e6db74";  // .pl-s
constexpr std::string_view kNumberColor   = "#f92672";  // .pl-c1
constexpr std::string_view kCommentColor  = "#f6f8fa";  // .pl-c
constexpr std::string_view kFunctionColor = "#a6e22e";  // .pl-en

// ---------------------------------------------------------------------------
// Character helpers
// ---------------------------------------------------------------------------
constexpr bool isIdStart(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
constexpr bool isDigit(char c) { return c >= '0' && c <= '9'; }
constexpr bool isHexDigit(char c) {
    return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}
constexpr bool isIdContinue(char c) { return isIdStart(c) || isDigit(c); }

// ---------------------------------------------------------------------------
// Keyword lists (must remain lexicographically sorted – verified by
// static_assert below each array)
// ---------------------------------------------------------------------------

// C++ keywords and fundamental types
constexpr auto kCppKeywords = std::to_array<std::string_view>({
    "alignas",        "alignof",     "and",          "and_eq",     "asm",
    "auto",           "bitand",      "bitor",        "bool",       "break",
    "case",           "catch",       "char",         "char16_t",   "char32_t",
    "char8_t",        "class",       "co_await",     "co_return",  "co_yield",
    "compl",          "concept",     "const",        "const_cast", "consteval",
    "constexpr",      "constinit",   "continue",     "decltype",   "default",
    "delete",         "do",          "double",       "dynamic_cast", "else",
    "enum",           "explicit",    "export",       "extern",     "false",
    "float",          "for",         "friend",       "goto",       "if",
    "inline",         "int",         "long",         "mutable",    "namespace",
    "new",            "noexcept",    "not",          "not_eq",     "nullptr",
    "operator",       "or",          "or_eq",        "private",    "protected",
    "public",         "register",    "reinterpret_cast", "requires", "return",
    "short",          "signed",      "sizeof",       "static",     "static_assert",
    "static_cast",    "struct",      "switch",       "template",   "this",
    "thread_local",   "throw",       "true",         "try",        "typedef",
    "typeid",         "typename",    "union",        "unsigned",   "using",
    "virtual",        "void",        "volatile",     "wchar_t",    "while",
    "xor",            "xor_eq",
});
static_assert(std::ranges::is_sorted(kCppKeywords));

// Python keywords
constexpr auto kPythonKeywords = std::to_array<std::string_view>({
    "False",  "None",   "True",    "and",    "as",     "assert", "async",
    "await",  "break",  "class",   "continue","def",   "del",    "elif",
    "else",   "except", "finally", "for",    "from",   "global", "if",
    "import", "in",     "is",      "lambda", "nonlocal","not",   "or",
    "pass",   "raise",  "return",  "try",    "while",  "with",   "yield",
});
static_assert(std::ranges::is_sorted(kPythonKeywords));

// GLSL keywords and built-in types
constexpr auto kGlslKeywords = std::to_array<std::string_view>({
    "attribute",
    "bool",       "break",      "bvec2",      "bvec3",         "bvec4",
    "case",       "centroid",   "coherent",   "const",         "continue",
    "default",    "discard",    "do",         "double",        "dvec2",
    "dvec3",      "dvec4",
    "else",
    "flat",       "float",      "for",
    "highp",
    "if",         "in",         "inout",      "int",           "invariant",
    "isampler1D", "isampler2D", "isampler2DArray", "isampler3D", "isamplerCube",
    "ivec2",      "ivec3",      "ivec4",
    "layout",     "lowp",
    "mat2",       "mat2x2",     "mat2x3",     "mat2x4",        "mat3",
    "mat3x2",     "mat3x3",     "mat3x4",     "mat4",          "mat4x2",
    "mat4x3",     "mat4x4",     "mediump",
    "noperspective",
    "out",
    "patch",      "precision",
    "return",
    "sampler1D",  "sampler1DArray", "sampler1DArrayShadow", "sampler1DShadow",
    "sampler2D",  "sampler2DArray", "sampler2DArrayShadow", "sampler2DMS",
    "sampler2DMSArray", "sampler2DRect", "sampler2DRectShadow", "sampler2DShadow",
    "sampler3D",  "samplerBuffer", "samplerCube", "samplerCubeShadow",
    "smooth",     "struct",     "subroutine", "switch",
    "uint",       "uniform",    "usampler2D", "uvec2",         "uvec3",
    "uvec4",
    "varying",    "vec2",       "vec3",       "vec4",          "void",
    "volatile",
    "while",
});
static_assert(std::ranges::is_sorted(kGlslKeywords));

// ---------------------------------------------------------------------------
// Language detection
// ---------------------------------------------------------------------------
enum class Language { Cpp, Python, Glsl, Unknown };

Language detectLang(std::string_view lang) {
    if (lang == "cpp" || lang == "c++" || lang == "cxx" || lang == "cc" || lang == "c") {
        return Language::Cpp;
    }
    if (lang == "python" || lang == "py") {
        return Language::Python;
    }
    if (lang == "glsl" || lang == "frag" || lang == "vert" || lang == "comp") {
        return Language::Glsl;
    }
    return Language::Unknown;
}

bool isKeyword(std::string_view word, Language lang) {
    switch (lang) {
        case Language::Cpp:
            return std::ranges::binary_search(kCppKeywords, word);
        case Language::Python:
            return std::ranges::binary_search(kPythonKeywords, word);
        case Language::Glsl:
            return std::ranges::binary_search(kGlslKeywords, word);
        default:
            return false;
    }
}

// ---------------------------------------------------------------------------
// Token representation
// ---------------------------------------------------------------------------
enum class TokenKind { Default, Keyword, String, Number, Comment, Function };

struct Token {
    std::string_view text;
    TokenKind kind;
};

constexpr std::string_view tokenColor(TokenKind kind) {
    switch (kind) {
        case TokenKind::Keyword:  return kKeywordColor;
        case TokenKind::String:   return kStringColor;
        case TokenKind::Number:   return kNumberColor;
        case TokenKind::Comment:  return kCommentColor;
        case TokenKind::Function: return kFunctionColor;
        default:                  return {};
    }
}

// ---------------------------------------------------------------------------
// Tokenizer
// ---------------------------------------------------------------------------

std::vector<Token> tokenize(std::string_view code, Language lang) {
    std::vector<Token> tokens;
    const size_t n = code.size();
    size_t i = 0;

    // Flush a [start, i) range of "plain" characters into the token list,
    // merging with the previous Default token when possible.
    auto emitDefault = [&](size_t start) {
        if (start >= i) return;
        std::string_view text = code.substr(start, i - start);
        if (!tokens.empty() && tokens.back().kind == TokenKind::Default) {
            // Extend the previous default token in-place (both views are into `code`).
            tokens.back().text =
                code.substr(tokens.back().text.data() - code.data(),
                            text.data() + text.size() - tokens.back().text.data());
        } else {
            tokens.push_back({text, TokenKind::Default});
        }
    };

    while (i < n) {
        const size_t start = i;

        // ----------------------------------------------------------------
        // Single-line comment: // (C++/GLSL)
        // ----------------------------------------------------------------
        if ((lang == Language::Cpp || lang == Language::Glsl) && code[i] == '/' &&
            i + 1 < n && code[i + 1] == '/') {
            i += 2;
            while (i < n && code[i] != '\n') ++i;
            tokens.push_back({code.substr(start, i - start), TokenKind::Comment});
            continue;
        }

        // ----------------------------------------------------------------
        // Block comment: /* ... */ (C++/GLSL)
        // ----------------------------------------------------------------
        if ((lang == Language::Cpp || lang == Language::Glsl) && code[i] == '/' &&
            i + 1 < n && code[i + 1] == '*') {
            i += 2;
            while (i + 1 < n && !(code[i] == '*' && code[i + 1] == '/')) ++i;
            if (i + 1 < n) i += 2;  // consume '*/'
            tokens.push_back({code.substr(start, i - start), TokenKind::Comment});
            continue;
        }

        // ----------------------------------------------------------------
        // Python comment: # ...
        // ----------------------------------------------------------------
        if (lang == Language::Python && code[i] == '#') {
            ++i;
            while (i < n && code[i] != '\n') ++i;
            tokens.push_back({code.substr(start, i - start), TokenKind::Comment});
            continue;
        }

        // ----------------------------------------------------------------
        // C++/GLSL preprocessor directive: # at the start of a line
        // ----------------------------------------------------------------
        if ((lang == Language::Cpp || lang == Language::Glsl) && code[i] == '#' &&
            (i == 0 || code[i - 1] == '\n')) {
            ++i;
            while (i < n && code[i] != '\n') {
                if (code[i] == '\\' && i + 1 < n && code[i + 1] == '\n') {
                    i += 2;  // line continuation
                } else {
                    ++i;
                }
            }
            tokens.push_back({code.substr(start, i - start), TokenKind::Keyword});
            continue;
        }

        // ----------------------------------------------------------------
        // C++ raw string: R"delim(...)delim"
        // Optional prefix chars before R are handled below in the identifier
        // branch; here we handle bare R"
        // ----------------------------------------------------------------
        if (lang == Language::Cpp && code[i] == 'R' && i + 1 < n && code[i + 1] == '"') {
            i += 2;
            const size_t delimStart = i;
            while (i < n && code[i] != '(') ++i;
            const std::string_view delim = code.substr(delimStart, i - delimStart);
            if (i < n) ++i;  // consume '('
            std::string closing = ")";
            closing += delim;
            closing += '"';
            while (i < n) {
                if (code.substr(i).starts_with(closing)) {
                    i += closing.size();
                    break;
                }
                ++i;
            }
            tokens.push_back({code.substr(start, i - start), TokenKind::String});
            continue;
        }

        // ----------------------------------------------------------------
        // Python triple-quoted strings: """ ... """ or ''' ... '''
        // ----------------------------------------------------------------
        if (lang == Language::Python &&
            ((code[i] == '"' && i + 2 < n && code[i + 1] == '"' && code[i + 2] == '"') ||
             (code[i] == '\'' && i + 2 < n && code[i + 1] == '\'' && code[i + 2] == '\''))) {
            const char q = code[i];
            i += 3;
            while (i + 2 < n &&
                   !(code[i] == q && code[i + 1] == q && code[i + 2] == q)) {
                ++i;
            }
            if (i + 2 < n) i += 3;
            tokens.push_back({code.substr(start, i - start), TokenKind::String});
            continue;
        }

        // ----------------------------------------------------------------
        // Double-quoted string: "..."
        // ----------------------------------------------------------------
        if (code[i] == '"') {
            ++i;
            while (i < n && code[i] != '"' && code[i] != '\n') {
                if (code[i] == '\\' && i + 1 < n) ++i;
                ++i;
            }
            if (i < n && code[i] == '"') ++i;
            tokens.push_back({code.substr(start, i - start), TokenKind::String});
            continue;
        }

        // ----------------------------------------------------------------
        // Single-quoted literal: '...'
        // ----------------------------------------------------------------
        if (code[i] == '\'') {
            ++i;
            while (i < n && code[i] != '\'' && code[i] != '\n') {
                if (code[i] == '\\' && i + 1 < n) ++i;
                ++i;
            }
            if (i < n && code[i] == '\'') ++i;
            tokens.push_back({code.substr(start, i - start), TokenKind::String});
            continue;
        }

        // ----------------------------------------------------------------
        // Numeric literal: digit, or '.' followed by a digit
        // ----------------------------------------------------------------
        if (isDigit(code[i]) || (code[i] == '.' && i + 1 < n && isDigit(code[i + 1]))) {
            if (code[i] == '0' && i + 1 < n &&
                (code[i + 1] == 'x' || code[i + 1] == 'X')) {
                // Hexadecimal
                i += 2;
                while (i < n && isHexDigit(code[i])) ++i;
            } else if (code[i] == '0' && i + 1 < n &&
                       (code[i + 1] == 'b' || code[i + 1] == 'B')) {
                // Binary
                i += 2;
                while (i < n && (code[i] == '0' || code[i] == '1')) ++i;
            } else {
                // Decimal / float
                while (i < n && (isDigit(code[i]) || code[i] == '_')) ++i;
                if (i < n && code[i] == '.') {
                    ++i;
                    while (i < n && isDigit(code[i])) ++i;
                }
                if (i < n && (code[i] == 'e' || code[i] == 'E')) {
                    ++i;
                    if (i < n && (code[i] == '+' || code[i] == '-')) ++i;
                    while (i < n && isDigit(code[i])) ++i;
                }
            }
            // Optional suffix: f, u, l, ul, ull, etc.
            while (i < n && isIdContinue(code[i])) ++i;
            tokens.push_back({code.substr(start, i - start), TokenKind::Number});
            continue;
        }

        // ----------------------------------------------------------------
        // Identifier, keyword, or string-prefix + literal
        // ----------------------------------------------------------------
        if (isIdStart(code[i])) {
            ++i;
            while (i < n && isIdContinue(code[i])) ++i;
            const std::string_view word = code.substr(start, i - start);

            // Detect a string prefix that immediately precedes a quote.
            // Handles: L"", u"", U"", u8"", f"", r"", b"", rb"", fr"", …
            if (i < n && (code[i] == '"' || code[i] == '\'')) {
                // Consume the string after the prefix
                const char q = code[i];
                ++i;
                if (lang == Language::Python && q == '"' && i + 1 < n &&
                    code[i] == '"' && code[i + 1] == '"') {
                    // Triple-quoted after prefix
                    i += 2;
                    while (i + 2 < n &&
                           !(code[i] == q && code[i + 1] == q && code[i + 2] == q))
                        ++i;
                    if (i + 2 < n) i += 3;
                } else if (lang == Language::Python && q == '\'' && i + 1 < n &&
                           code[i] == '\'' && code[i + 1] == '\'') {
                    i += 2;
                    while (i + 2 < n &&
                           !(code[i] == q && code[i + 1] == q && code[i + 2] == q))
                        ++i;
                    if (i + 2 < n) i += 3;
                } else {
                    while (i < n && code[i] != q && code[i] != '\n') {
                        if (code[i] == '\\' && i + 1 < n) ++i;
                        ++i;
                    }
                    if (i < n && code[i] == q) ++i;
                }
                tokens.push_back({code.substr(start, i - start), TokenKind::String});
                continue;
            }

            // Keyword check
            if (isKeyword(word, lang)) {
                tokens.push_back({word, TokenKind::Keyword});
                continue;
            }

            // Function-name check: identifier immediately followed by '(' (ignoring spaces)
            size_t j = i;
            while (j < n && (code[j] == ' ' || code[j] == '\t')) ++j;
            if (j < n && code[j] == '(') {
                tokens.push_back({word, TokenKind::Function});
                continue;
            }

            // Plain identifier
            tokens.push_back({word, TokenKind::Default});
            continue;
        }

        // ----------------------------------------------------------------
        // Default: single character (operators, punctuation, whitespace)
        // Merge with the previous Default token when possible.
        // ----------------------------------------------------------------
        ++i;
        emitDefault(start);
    }

    return tokens;
}

}  // namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
void highlightCode(Document::DocumentHandle handle, std::string_view code,
                   std::string_view lang) {
    const Language language = detectLang(lang);

    if (language == Language::Unknown) {
        // No highlighting – append HTML-escaped plain text
        handle += htmlEncode(code);
        return;
    }

    for (const Token& token : tokenize(code, language)) {
        const std::string_view color = tokenColor(token.kind);
        const std::string encoded = htmlEncode(token.text);
        if (color.empty()) {
            handle += encoded;
        } else {
            handle.append("span", encoded,
                          {{"style", fmt::format("color:{}", color)}});
        }
    }
}

}  // namespace inviwo::util
