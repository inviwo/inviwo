/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#pragma once

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/util/glmvec.h>  // for vec4

#include <QObject>             // for Q_OBJECT, signals
#include <QRegularExpression>  // for QRegularExpression
#include <QString>             // for QString
#include <QSyntaxHighlighter>  // for QSyntaxHighlighter
#include <QTextCharFormat>     // for QTextCharFormat
#include <glm/vec4.hpp>        // for operator/

#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector

#include <tcb/span.hpp>  // for span

class QTextCharFormat;
class QTextDocument;

namespace inviwo {

enum class SyntaxType {
    None = 0,
    GLSL = 1,
    Python = 2,
};

namespace syntax {
#include <warn/push>
#include <warn/ignore/unused-variable>
constexpr vec4 ghost_white = vec4(248.f, 248.f, 240.f, 255.f) / vec4(255.0f);
constexpr vec4 light_ghost_white = vec4(248.f, 248.f, 242.f, 255.f) / vec4(255.0f);
constexpr vec4 light_gray = vec4(204.f, 204.f, 204.f, 255.f) / vec4(255.0f);
constexpr vec4 gray = vec4(136.f, 136.f, 136.f, 255.f) / vec4(255.0f);
constexpr vec4 brown_gray = vec4(73.f, 72.f, 62.f, 255.f) / vec4(255.0f);
constexpr vec4 dark_gray = vec4(43.f, 44.f, 39.f, 255.f) / vec4(255.0f);

constexpr vec4 yellow = vec4(230.f, 219.f, 116.f, 255.f) / vec4(255.0f);
constexpr vec4 blue = vec4(102.f, 217.f, 239.f, 255.f) / vec4(255.0f);
constexpr vec4 pink = vec4(249.f, 38.f, 114.f, 255.f) / vec4(255.0f);
constexpr vec4 purple = vec4(174.f, 129.f, 255.f, 255.f) / vec4(255.0f);
constexpr vec4 brown = vec4(117.f, 113.f, 94.f, 255.f) / vec4(255.0f);
constexpr vec4 orange = vec4(253.f, 151.f, 31.f, 255.f) / vec4(255.0f);
constexpr vec4 light_orange = vec4(255.f, 213.f, 105.f, 255.f) / vec4(255.0f);
constexpr vec4 green = vec4(166.f, 226.f, 46.f, 255.f) / vec4(255.0f);
constexpr vec4 sea_green = vec4(166.f, 228.f, 48.f, 255.f) / vec4(255.0f);
#include <warn/pop>

constexpr int fontSize = 11;
constexpr vec4 text = light_ghost_white;
constexpr vec4 background = dark_gray;
constexpr vec4 highLight = vec4{33.f, 34.f, 29.f, 255.f} / vec4(255.f);
constexpr vec4 keyword = blue;
constexpr vec4 builtinVars = orange;
constexpr vec4 builtinFuncs = blue;
constexpr vec4 type = green;
constexpr vec4 comment = gray;
constexpr vec4 preProcessor = pink;
constexpr vec4 literal = light_orange;
constexpr vec4 constant = purple;
constexpr vec4 main = sea_green;

}  // namespace syntax

class IVW_MODULE_QTWIDGETS_API SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    SyntaxHighlighter(QTextDocument* parent);
    virtual ~SyntaxHighlighter();

    const QTextCharFormat& defaultFormat() const;
    void setDefaultFormat(QTextCharFormat format);

    int fontSize() const { return fontSize_; }
    void setFontSize(int size) { fontSize_ = size; }

    const std::string& font() const { return font_; }
    void setFont(const std::string& font) { font_ = font; }

    const vec4& highlight() const { return highlight_; }
    void setHighlight(const vec4& highlight) { highlight_ = highlight; }

    void addPattern(QTextCharFormat format, std::string_view pattern);
    void addPatterns(QTextCharFormat format, util::span<const std::string_view> patterns);
    void addPatternWithFormatStr(QTextCharFormat format,
                                 util::span<const std::string_view> patterns,
                                 std::string_view formatStr);
    void addWordBoundaryPattern(QTextCharFormat format,
                                util::span<const std::string_view> patterns);
    void addMultBlockPattern(QTextCharFormat format, std::string_view startPattern,
                             std::string_view endPattern);

    void clear();

signals:
    void update();

protected:
    void highlightBlock(const QString& text);

private:
    struct Rule {
        QTextCharFormat format;
        QRegularExpression pattern;
    };

    struct MultiBlockRule {
        QTextCharFormat format;
        QRegularExpression begin;
        QRegularExpression end;
        int name;
    };

    QTextCharFormat defaultFormat_;
    int fontSize_;
    std::string font_;
    vec4 highlight_;

    std::vector<Rule> rules_;
    std::vector<MultiBlockRule> multiblockRules_;
};

}  // namespace inviwo
