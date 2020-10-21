/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/zip.h>
#include <modules/opengl/shader/shaderobject.h>

#include <fmt/format.h>

namespace inviwo {

namespace {

const std::string code1{R"(out vec4 color_;
out vec3 texCoord_;

// Comment

/*
Block comment
*/

/*
**/

#include "inc1"
#include "incX"
#include "inc2"
//#include "inc2"

/*
#include "inc2"
*/

void main() {
    color_ = in_Color;
    MAGIC_REPLACEMENT
    texCoord_ = in_TexCoord;
    gl_Position = in_Vertex;
}
)"};

const std::string code1Parsed{R"(out vec4 color_;
out vec3 texCoord_;

// Comment

/*
Block comment
*/

/*
**/

Inc1

Inc2a
Inc2b

//#include "inc2"

/*
#include "inc2"
*/

void main() {
    color_ = in_Color;
    replacement code1;
    replacement code2;
    replacement code3;
    replacement code4;
    texCoord_ = in_TexCoord;
    gl_Position = in_Vertex;
}
)"};

}  // namespace

TEST(ShaderObject, parseSource) {

    LineNumberResolver lnr;

    auto getSource =
        [](std::string_view path) -> std::optional<std::pair<std::string, std::string>> {
        if (path == "inc1") {
            return std::pair<std::string, std::string>{path, "Inc1"};
        }
        if (path == "inc2") {
            return std::pair<std::string, std::string>{path, "Inc2a\nInc2b\n"};
        }

        return std::nullopt;
    };

    using SegType = typename ShaderSegment::Type;
    std::unordered_map<SegType, std::vector<ShaderSegment>> replacements{
        {SegType{"MAGIC_REPLACEMENT"},
         {ShaderSegment{SegType{"MAGIC_REPLACEMENT"}, "Repl1",
                        "replacement code1;\nreplacement code2;", 900},
          ShaderSegment{SegType{"MAGIC_REPLACEMENT"}, "Repl2",
                        "replacement code3;\nreplacement code4;", 1100}}}

    };

    std::ostringstream oss;

    utilgl::parseShaderSource("Code1", code1, oss, lnr, replacements, getSource);
    auto parsed = oss.str();

    std::stringstream ss;
    for (auto&& [key, line] : util::zip(lnr, splitString(parsed, '\n'))) {
        ss << fmt::format("{:<12} {:>2}: {}\n", key.first, key.second, line);
    }
    auto pre = ss.str();

    auto expectedLines = splitString(code1Parsed, '\n');
    auto parsedLines = splitString(parsed, '\n');

    EXPECT_EQ(expectedLines.size(), parsedLines.size());

    for (const auto& [expectedLine, parsedLine] : util::zip(expectedLines, parsedLines)) {
        EXPECT_EQ(expectedLine, parsedLine);
    }

    std::vector<size_t> lineNum{1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 1,  14, 1,  2, 3,
                                16, 17, 18, 19, 20, 21, 22, 23, 1, 2,  1,  2,  25, 26, 27, 28};
    std::vector<std::string> names{"Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "inc1",
                                   "Code1",
                                   "inc2",
                                   "inc2",
                                   "inc2",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Repl1[MAGIC_REPLACEMENT,900]",
                                   "Repl1[MAGIC_REPLACEMENT,900]",
                                   "Repl2[MAGIC_REPLACEMENT,1100]",
                                   "Repl2[MAGIC_REPLACEMENT,1100]",
                                   "Code1",
                                   "Code1",
                                   "Code1",
                                   "Code1"};

    for (const auto& [exp, refname, refline] : util::zip(lnr, names, lineNum)) {
        EXPECT_EQ(exp.first, refname);
        EXPECT_EQ(exp.second, refline);
    }
}

}  // namespace inviwo
