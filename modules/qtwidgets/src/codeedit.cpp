/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2026 Inviwo Foundation
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

#include <modules/qtwidgets/codeedit.h>

#include <inviwo/core/util/glmvec.h>              // for vec4
#include <modules/qtwidgets/inviwoqtutils.h>      // for toQString, toQColor, toivec3, tovec4
#include <modules/qtwidgets/syntaxhighlighter.h>  // for SyntaxHighlighter, text

#include <utility>  // for move
#include <ranges>

#include <QFont>            // for QFont
#include <QFontMetrics>     // for QFontMetrics
#include <QKeyEvent>        // for QKeyEvent
#include <QList>            // for QList
#include <QPaintEvent>      // for QPaintEvent
#include <QPainter>         // for QPainter, QPainter::TextAntialiasing
#include <QRect>            // for QRect
#include <QRectF>           // for QRectF
#include <QTextBlock>       // for QTextBlock
#include <QTextCharFormat>  // for QTextCharFormat
#include <QTextCursor>      // for QTextCursor
#include <QTextDocument>    // for QTextDocument
#include <QTextEdit>        // for QTextEdit::ExtraSelection, QTextEdit
#include <QTextFormat>      // for QTextFormat, QTextFormat::FullWidthSele...
#include <QTextOption>      // for QTextOption, QTextOption::NoWrap
#include <QtGlobal>         // for qMax, qreal
#include <QBrush>           // for QBrush
#include <Qt>               // for AlignRight, Key_Tab
#include <fmt/core.h>       // for format
#include <glm/vec3.hpp>     // for vec, vec<>::(anonymous)

namespace inviwo {

CodeEdit::CodeEdit(QWidget* parent)
    : QPlainTextEdit(parent)
    , lineNumberArea_{new LineNumberArea(this)}
    , textColor_{syntax::text}
    , highlightColor_{1.0f}
    , sh_{new SyntaxHighlighter(document())}
    , annotateLine_{[](int line) { return std::to_string(line); }}
    , annotateColor_{[](int, vec4 orgColor) { return orgColor; }}
    , annotationSpace_{[](int maxDigits) { return maxDigits; }} {

    setObjectName("CodeEdit");
    setReadOnly(false);
    setWordWrapMode(QTextOption::NoWrap);
    setUndoRedoEnabled(true);

    createStandardContextMenu();

    connect(this, &CodeEdit::blockCountChanged, this, &CodeEdit::updateLineNumberAreaWidth);
    connect(this, &CodeEdit::updateRequest, this, &CodeEdit::updateLineNumberArea);
    connect(this, &CodeEdit::cursorPositionChanged, this, &CodeEdit::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    auto updateSyntax = [this]() {
        auto color = utilqt::toivec3(sh_->defaultFormat().background().color());

        auto css = fmt::format("background-color: rgb({}, {}, {});font-size: {}pt;font-family: {};",
                               color.r, color.g, color.b, sh_->fontSize(), sh_->font());
        setStyleSheet(utilqt::toQString(css));

        textColor_ = utilqt::tovec4(sh_->defaultFormat().foreground().color());
        highlightColor_ = sh_->highlight();

        QFontMetrics metrics(QFont(utilqt::toQString(sh_->font()), sh_->fontSize()));
        setTabStopDistance(static_cast<qreal>(4 * metrics.horizontalAdvance(' ')));
        sh_->rehighlight();
    };

    connect(sh_, &SyntaxHighlighter::update, this, updateSyntax);
    updateSyntax();
}

void CodeEdit::setLineAnnotation(std::function<std::string(int)> func) {
    annotateLine_ = std::move(func);
    updateLineNumberAreaWidth(0);
}
void CodeEdit::setLineAnnotationColor(std::function<vec4(int, vec4)> annotateColor) {
    annotateColor_ = std::move(annotateColor);
    updateLineNumberAreaWidth(0);
}
void CodeEdit::setAnnotationSpace(std::function<int(int)> func) {
    annotationSpace_ = std::move(func);
    updateLineNumberAreaWidth(0);
}

SyntaxHighlighter& CodeEdit::syntaxHighlighter() { return *sh_; }

namespace detail {

constexpr int singleIndent = 4;

void addIndent(QTextCursor cursor) {
    if (cursor.hasSelection()) {
        const int start = cursor.position();
        const int end = cursor.anchor();

        cursor.setPosition(start, QTextCursor::MoveAnchor);
        const int startBlock = cursor.block().blockNumber();
        cursor.setPosition(end, QTextCursor::MoveAnchor);
        const int endBlock = cursor.block().blockNumber();

        const int lineCount = std::abs(endBlock - startBlock) + 1;
        const QTextCursor::MoveOperation direction =
            start < end ? QTextCursor::PreviousBlock : QTextCursor::NextBlock;

        cursor.movePosition(QTextCursor::StartOfBlock);
        for (int i = 0; i < lineCount; ++i) {
            cursor.insertText(QString{" "}.repeated(singleIndent));
            cursor.movePosition(direction);
        }

        // restore previous selection
        if (start > end) {
            cursor.setPosition(end + singleIndent, QTextCursor::MoveAnchor);
            cursor.setPosition(start + singleIndent * lineCount, QTextCursor::KeepAnchor);
        } else {
            cursor.setPosition(end + singleIndent * lineCount, QTextCursor::MoveAnchor);
            cursor.setPosition(start + singleIndent, QTextCursor::KeepAnchor);
        }
    } else {
        cursor.insertText(QString{" "}.repeated(singleIndent));
    }
}

int spacesToRemove(const QString& str) {
    if (str.isEmpty()) return 0;
    int spaces = 0;
    while (str[spaces] == ' ' && spaces < singleIndent) {
        ++spaces;
    }
    return spaces;
}

void removeIndent(QTextCursor cursor, QPlainTextEdit* parent) {
    if (cursor.hasSelection()) {
        const int start = cursor.position();
        const int end = cursor.anchor();

        // select entire lines based on the selection
        if (start > end) {
            cursor.setPosition(end);
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.setPosition(start, QTextCursor::KeepAnchor);
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        } else {
            cursor.setPosition(end);
            cursor.movePosition(QTextCursor::EndOfBlock);
            cursor.setPosition(start, QTextCursor::KeepAnchor);
            cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        }

        // selectedText() contains paragraph separators U+2029 instead of line breaks
        QStringList lines =
            cursor.selectedText().replace(QChar{QChar::ParagraphSeparator}, "\n").split('\n');
        const int removedFromFirstLine = !lines.empty() ? spacesToRemove(lines.front()) : 0;

        int removedSpaces = 0;
        // remove up to singleIndent spaces from the beginning of each line
        for (auto& line : lines) {
            const int spaces = spacesToRemove(line);
            line.remove(0, spaces);
            removedSpaces += spaces;
        }

        cursor.insertText(lines.join('\n'));

        // restore previous selection
        if (start > end) {
            cursor.setPosition(end - removedFromFirstLine, QTextCursor::MoveAnchor);
            cursor.setPosition(start - removedSpaces, QTextCursor::KeepAnchor);
        } else {
            cursor.setPosition(end - removedSpaces, QTextCursor::MoveAnchor);
            cursor.setPosition(start - removedFromFirstLine, QTextCursor::KeepAnchor);
        }
        // update visual selection in text editor
        parent->setTextCursor(cursor);
    } else {
        const int start = cursor.position();
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, singleIndent);
        const QString str = cursor.selectedText();
        // determine the number of spaces to remove from the end of the string
        int spaces = 0;
        for (const auto& c : str | std::views::reverse) {
            if (c != ' ') {
                break;
            }
            ++spaces;
        }
        if (spaces > 0) {
            cursor.setPosition(start - spaces, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }
    }
}

}  // namespace detail

bool CodeEdit::event(QEvent* event) {
    // need to handle tab key events here, since QWidget moves the focus with Shift + Tab
    if (event->type() == QEvent::KeyPress) {
        auto* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (!(keyEvent->modifiers().testFlags(Qt::ControlModifier | Qt::AltModifier))) {
            if (keyEvent->key() == Qt::Key_Backtab ||
                (keyEvent->key() == Qt::Key_Tab &&
                 keyEvent->modifiers().testFlag(Qt::ShiftModifier))) {
                keyEvent->accept();
                detail::removeIndent(textCursor(), this);
                return true;
            } else if (keyEvent->key() == Qt::Key_Tab) {
                keyEvent->accept();
                detail::addIndent(textCursor());
                return true;
            }
        }
    }

    return QPlainTextEdit::event(event);
}

int CodeEdit::lineNumberAreaWidth() const {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    const int w = fontMetrics().horizontalAdvance(' ');
    return (annotationSpace_(digits) + 2) * w;
}

void CodeEdit::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEdit::updateLineNumberArea(const QRect& rect, int dy) {
    if (dy) {
        lineNumberArea_->scroll(0, dy);
    } else {
        lineNumberArea_->update(0, rect.y(), lineNumberArea_->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void CodeEdit::resizeEvent(QResizeEvent* e) {
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea_->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEdit::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(utilqt::toQColor(highlightColor_));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEdit::lineNumberAreaPaintEvent(QPaintEvent* event) {
    if (document()->isEmpty()) return;

    QPainter painter(lineNumberArea_);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    const int offset = fontMetrics().horizontalAdvance(' ');
    const int height = fontMetrics().height();
    const int width = lineNumberArea_->width() - offset;

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const auto number = utilqt::toQString(annotateLine_(blockNumber + 1));
            painter.setPen(utilqt::toQColor(annotateColor_(blockNumber + 1, textColor_)));
            painter.drawText(0, top, width, height, Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
        ++blockNumber;
    }
}

}  // namespace inviwo
