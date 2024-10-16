/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <inviwo/webqt/htmlsyntaxhighlight.h>

#include <inviwo/core/properties/optionproperty.h>   // for OptionPropertyString
#include <inviwo/core/properties/ordinalproperty.h>  // for FloatVec4Property, ordinalColor, Int...
#include <inviwo/core/properties/property.h>         // for Property
#include <inviwo/core/util/settings/settings.h>      // for Settings
#include <modules/qtwidgets/inviwoqtutils.h>         // for toQColor, getDefaultMonoSpaceFontIndex
#include <modules/qtwidgets/syntaxhighlighter.h>     // for SyntaxHighlighter, background, comment

#include <array>        // for array
#include <chrono>       // for literals
#include <string>       // for string
#include <string_view>  // for operator""sv, string_view, basic_str...

#include <QColor>           // for QColor
#include <QTextCharFormat>  // for QTextCharFormat

namespace inviwo {

HtmlSyntaxHighlight::HtmlSyntaxHighlight()
    : Settings("Html Syntax Highlighting")
    , font("font", "Font", utilqt::getMonoSpaceFonts(), utilqt::getDefaultMonoSpaceFontIndex())
    , fontSize("fontSize", "Size", syntax::fontSize, 1, 72)
    , textColor("text", "Text", util::ordinalColor(syntax::text))
    , backgroundColor("background", "Background", util::ordinalColor(syntax::background))
    , highLightColor("highLight", "HighLight", util::ordinalColor(syntax::highLight))
    , tagColor("tag", "Tag", util::ordinalColor(syntax::keyword))
    , attributeNameColor("attributeName", "Attribute Name", util::ordinalColor(syntax::literal))
    , attributeValueColor("attributeValue", "Attribute Value", util::ordinalColor(syntax::constant))
    , commentColor("comment", "Comment", util::ordinalColor(syntax::comment)) {

    addProperties(font, fontSize, textColor, backgroundColor, highLightColor, tagColor,
                  attributeNameColor, attributeValueColor, commentColor);

    load();
}

namespace utilqt {

namespace {

using namespace std::literals;

static constexpr std::array htmlTags = {"!DOCTYPE html"sv,
                                        "abbreviation"sv,
                                        "acronym"sv,
                                        "address"sv,
                                        "anchor"sv,
                                        "applet"sv,
                                        "area"sv,
                                        "article"sv,
                                        "aside"sv,
                                        "audio"sv,
                                        "base"sv,
                                        "basefont"sv,
                                        "bdi"sv,
                                        "bdo"sv,
                                        "bgsound"sv,
                                        "big"sv,
                                        "blockquote"sv,
                                        "body"sv,
                                        "bold"sv,
                                        "break"sv,
                                        "button"sv,
                                        "caption"sv,
                                        "canvas"sv,
                                        "center"sv,
                                        "cite"sv,
                                        "code"sv,
                                        "colgroup"sv,
                                        "column"sv,
                                        "comment"sv,
                                        "data"sv,
                                        "datalist"sv,
                                        "dd"sv,
                                        "define"sv,
                                        "delete"sv,
                                        "details"sv,
                                        "dialog"sv,
                                        "dir"sv,
                                        "div"sv,
                                        "dl"sv,
                                        "dt"sv,
                                        "embed"sv,
                                        "fieldset"sv,
                                        "figcaption"sv,
                                        "figure"sv,
                                        "font"sv,
                                        "footer"sv,
                                        "form"sv,
                                        "frame"sv,
                                        "frameset"sv,
                                        "head"sv,
                                        "header"sv,
                                        "heading"sv,
                                        "hgroup"sv,
                                        "hr"sv,
                                        "html"sv,
                                        "iframes"sv,
                                        "image"sv,
                                        "input"sv,
                                        "ins"sv,
                                        "isindex"sv,
                                        "italic"sv,
                                        "kbd"sv,
                                        "keygen"sv,
                                        "label"sv,
                                        "legend"sv,
                                        "list"sv,
                                        "main"sv,
                                        "mark"sv,
                                        "marquee"sv,
                                        "menuitem"sv,
                                        "meta"sv,
                                        "meter"sv,
                                        "nav"sv,
                                        "nobreak"sv,
                                        "noembed"sv,
                                        "noscript"sv,
                                        "object"sv,
                                        "optgroup"sv,
                                        "option"sv,
                                        "output"sv,
                                        "paragraphs"sv,
                                        "param"sv,
                                        "phrase"sv,
                                        "pre"sv,
                                        "progress"sv,
                                        "q"sv,
                                        "rp"sv,
                                        "rt"sv,
                                        "ruby"sv,
                                        "s"sv,
                                        "samp"sv,
                                        "script"sv,
                                        "section"sv,
                                        "small"sv,
                                        "source"sv,
                                        "spacer"sv,
                                        "span"sv,
                                        "strike"sv,
                                        "strong"sv,
                                        "style"sv,
                                        "sup"sv,
                                        "sub"sv,
                                        "summary"sv,
                                        "svg"sv,
                                        "table"sv,
                                        "tbody"sv,
                                        "td"sv,
                                        "template"sv,
                                        "tfoot"sv,
                                        "th"sv,
                                        "thead"sv,
                                        "time"sv,
                                        "title"sv,
                                        "tr"sv,
                                        "track"sv,
                                        "tt"sv,
                                        "underline"sv,
                                        "var"sv,
                                        "video"sv,
                                        "wbr"sv,
                                        "xmp"sv};
}  // namespace

std::vector<std::shared_ptr<std::function<void()>>> setHtmlSyntaxHighlight(
    SyntaxHighlighter& sh, HtmlSyntaxHighlight& settings) {

    QColor bgColor = utilqt::toQColor(settings.backgroundColor);

    QTextCharFormat defaultFormat;
    defaultFormat.setBackground(bgColor);
    defaultFormat.setForeground(utilqt::toQColor(settings.textColor));

    QTextCharFormat tagformat;
    tagformat.setBackground(bgColor);
    tagformat.setForeground(utilqt::toQColor(settings.tagColor));

    QTextCharFormat attributeNameFormat;
    attributeNameFormat.setBackground(bgColor);
    attributeNameFormat.setForeground(utilqt::toQColor(settings.attributeNameColor));

    QTextCharFormat attributeValueFormat;
    attributeValueFormat.setBackground(bgColor);
    attributeValueFormat.setForeground(utilqt::toQColor(settings.attributeValueColor));

    QTextCharFormat commentformat;
    commentformat.setBackground(bgColor);
    commentformat.setForeground(utilqt::toQColor(settings.commentColor));

    sh.setFont(settings.font);
    sh.setFontSize(settings.fontSize);
    sh.setHighlight(settings.highLightColor);
    sh.setDefaultFormat(defaultFormat);

    for (auto pattern : htmlTags) {
        sh.addPattern(tagformat, fmt::format("<{}.*>", pattern));
        sh.addPattern(tagformat, fmt::format("</{}>", pattern));
    }

    sh.addPattern(commentformat, "<!--.*?-->");
    sh.addPattern(attributeNameFormat, "\\b(\\w+)\\s*=");
    sh.addPattern(attributeValueFormat, "\".*?\"|'.*?'");

    sh.update();

    std::vector<std::shared_ptr<std::function<void()>>> callbacks;
    for (auto p : settings) {
        callbacks.emplace_back(p->onChangeScoped(
            [psh = &sh, psettings = &settings]() { setHtmlSyntaxHighlight(*psh, *psettings); }));
    }
    return callbacks;
}

}  // namespace utilqt

}  // namespace inviwo
