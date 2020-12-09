/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2020 Inviwo Foundation
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

#include <modules/qtwidgets/syntaxhighlighter.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/filesystem.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QTextDocument>
#include <QTextBlock>
#include <warn/pop>

#include <array>
#include <string_view>

using namespace std::literals;

namespace inviwo {

SyntaxHighligther::SyntaxHighligther(QTextDocument* document)
    : QSyntaxHighlighter(document)
    , defaultFormat_{}
    , fontSize_{syntax::fontSize}
    , font_{utilqt::getMonoSpaceFonts()[utilqt::getDefaultFontIndex()]}
    , highlight_{syntax::highLight}
    , rules_{}
    , multiblockRules_{} {
    defaultFormat_.setForeground(utilqt::toQColor(syntax::text));
    defaultFormat_.setBackground(utilqt::toQColor(syntax::background));
}

SyntaxHighligther::~SyntaxHighligther() = default;

const QTextCharFormat& SyntaxHighligther::defaultFormat() const { return defaultFormat_; }

void SyntaxHighligther::highlightBlock(const QString& text) {
    setFormat(0, text.size(), defaultFormat_);

    for (auto& rule : rules_) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            auto m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);
    const auto prevState = previousBlockState();

    for (auto& rule : multiblockRules_) {

        int startIndex = 0;
        int currPos = 0;

        if (prevState <= 0) {
            if (auto beginMatch = rule.begin.match(text); beginMatch.hasMatch()) {
                startIndex = beginMatch.capturedStart();
                currPos = beginMatch.capturedEnd();
            } else {
                startIndex = -1;
            }
        } else if (prevState != rule.name) {
            continue;
        }

        while (startIndex >= 0) {
            auto endMatch = rule.end.match(text, currPos);
            int commentLength;

            if (endMatch.hasMatch()) {
                commentLength = endMatch.capturedEnd() - startIndex;
            } else {
                setCurrentBlockState(rule.name);
                commentLength = text.length() - startIndex;
            }
            setFormat(startIndex, commentLength, rule.format);

            if (auto beginMatch = rule.begin.match(text, startIndex + commentLength);
                beginMatch.hasMatch()) {
                startIndex = beginMatch.capturedStart();
                currPos = beginMatch.capturedEnd();
            } else {
                startIndex = -1;
            }
        }
    }
}

void SyntaxHighligther::addMultBlockPattern(QTextCharFormat format, std::string_view startPattern,
                                            std::string_view endPattern) {
    multiblockRules_.push_back(MultiBlockRule{format,
                                              QRegularExpression{utilqt::toQString(startPattern)},
                                              QRegularExpression{utilqt::toQString(endPattern)},
                                              static_cast<int>(multiblockRules_.size() + 1)

    });
}

void SyntaxHighligther::setDefaultFormat(QTextCharFormat format) { defaultFormat_ = format; }

void SyntaxHighligther::addPattern(QTextCharFormat format, std::string_view pattern) {
    rules_.push_back(Rule{format, QRegularExpression{utilqt::toQString(pattern)}});
}

void SyntaxHighligther::addPatterns(QTextCharFormat format,
                                    util::span<const std::string_view> patterns) {
    for (auto pattern : patterns) {
        rules_.push_back(Rule{format, QRegularExpression{utilqt::toQString(pattern)}});
    }
}

void SyntaxHighligther::addPatternWithFormatStr(QTextCharFormat format,
                                                util::span<const std::string_view> patterns,
                                                std::string_view formatStr) {
    for (auto pattern : patterns) {
        auto str = utilqt::toQString(fmt::format(formatStr, pattern));
        rules_.push_back(Rule{format, QRegularExpression{str}});
    }
}

void SyntaxHighligther::addWordBoundaryPattern(QTextCharFormat format,
                                               util::span<const std::string_view> patterns) {
    addPatternWithFormatStr(format, patterns, "\\b{}\\b");
}

void SyntaxHighligther::clear() {
    rules_.clear();
    multiblockRules_.clear();
}

}  // namespace inviwo
