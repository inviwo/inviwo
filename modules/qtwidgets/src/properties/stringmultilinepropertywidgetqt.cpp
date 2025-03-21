/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/qtwidgets/properties/stringmultilinepropertywidgetqt.h>

#include <inviwo/core/properties/stringproperty.h>  // for StringProperty
#include <inviwo/core/properties/multifileproperty.h>
#include <modules/qtwidgets/editablelabelqt.h>              // for EditableLabelQt
#include <modules/qtwidgets/inviwoqtutils.h>                // for fromQString, toQString
#include <modules/qtwidgets/properties/propertywidgetqt.h>  // for PropertyWidgetQt

#include <string>  // for string

#include <QAbstractScrollArea>  // for QAbstractScrollArea, QAbstrac...
#include <QFlags>               // for QFlags
#include <QFontMetrics>         // for QFontMetrics
#include <QHBoxLayout>          // for QHBoxLayout
#include <QKeyEvent>            // for QKeyEvent
#include <QMargins>             // for QMargins
#include <QSizeF>               // for QSizeF
#include <QSizePolicy>          // for QSizePolicy, QSizePolicy::Pre...
#include <QString>              // for QString, operator!=
#include <QTextCursor>          // for QTextCursor, QTextCursor::Start
#include <QTextDocument>        // for QTextDocument
#include <Qt>                   // for ControlModifier, Key_Enter
#include <glm/common.hpp>       // for clamp

class QContextMenuEvent;
class QFocusEvent;
class QHBoxLayout;
class QKeyEvent;
class QResizeEvent;
class QWidget;

namespace inviwo {

StringMultilinePropertyWidgetQt::StringMultilinePropertyWidgetQt(StringProperty* property)
    : PropertyWidgetQt(property), property_(property) {

    QHBoxLayout* hLayout = new QHBoxLayout;
    setSpacingAndMargins(hLayout);

    label_ = new EditableLabelQt(this, property_);
    hLayout->addWidget(label_);

    textEdit_ = new MultilineTextEdit;
    setFocusPolicy(textEdit_->focusPolicy());
    setFocusProxy(textEdit_);

    QSizePolicy sp = textEdit_->sizePolicy();
    sp.setHorizontalStretch(3);
    sp.setVerticalPolicy(QSizePolicy::Preferred);
    textEdit_->setSizePolicy(sp);

    hLayout->addWidget(textEdit_);

    setLayout(hLayout);
    connect(textEdit_, &MultilineTextEdit::editingFinished, this,
            &StringMultilinePropertyWidgetQt::setPropertyValue);

    updateFromProperty();
}

void StringMultilinePropertyWidgetQt::setPropertyValue() {
    std::string valueStr = utilqt::fromQString(textEdit_->toPlainText());
    property_->setInitiatingWidget(this);
    property_->set(valueStr);
    property_->clearInitiatingWidget();
}

void StringMultilinePropertyWidgetQt::updateFromProperty() {
    QString text(textEdit_->toPlainText());
    QString newContents(utilqt::toQString(property_->get()));
    if (text != newContents) {
        textEdit_->setPlainText(newContents);
        textEdit_->moveCursor(QTextCursor::Start);

        textEdit_->adjustHeight();
    }
}

MultiFileStringPropertyWidgetQt::MultiFileStringPropertyWidgetQt(MultiFileProperty* property)
    : PropertyWidgetQt(property), property_(property) {

    QHBoxLayout* hLayout = new QHBoxLayout;
    setSpacingAndMargins(hLayout);

    label_ = new EditableLabelQt(this, property_);
    hLayout->addWidget(label_);

    textEdit_ = new MultilineTextEdit;
    setFocusPolicy(textEdit_->focusPolicy());
    setFocusProxy(textEdit_);

    QSizePolicy sp = textEdit_->sizePolicy();
    sp.setHorizontalStretch(3);
    sp.setVerticalPolicy(QSizePolicy::Preferred);
    textEdit_->setSizePolicy(sp);

    hLayout->addWidget(textEdit_);

    setLayout(hLayout);
    connect(textEdit_, &MultilineTextEdit::editingFinished, this,
            &MultiFileStringPropertyWidgetQt::setPropertyValue);

    updateFromProperty();
}

void MultiFileStringPropertyWidgetQt::setPropertyValue() {
    std::string valueStr = utilqt::fromQString(textEdit_->toPlainText());
    std::vector<std::filesystem::path> paths;
    util::forEachStringPart(valueStr, "\n", [&](std::string_view str) { paths.emplace_back(str); });

    property_->setInitiatingWidget(this);
    property_->set(paths);
    property_->clearInitiatingWidget();
}

void MultiFileStringPropertyWidgetQt::updateFromProperty() {
    QString text(textEdit_->toPlainText());
    QString newContents{};

    for (const auto& path : property_->get()) {
        newContents.append(utilqt::toQString(path));
        newContents.append('\n');
    }
    if (text != newContents) {
        textEdit_->setPlainText(newContents);
        textEdit_->moveCursor(QTextCursor::Start);

        textEdit_->adjustHeight();
    }
}

MultilineTextEdit::MultilineTextEdit(QWidget* parent)
    : QPlainTextEdit(parent), minLineCount_(2), maxLineCount_(10), showContextMenu_(false) {
    QFontMetrics fontMetric(font());
    lineHeight_ = static_cast<int>(fontMetric.lineSpacing() * 1.5);

    // enable focus switch by tab (leaving the editor and committing the changes)
    setTabChangesFocus(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // need to enable the scrollbar for adjusting to its contents
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    // make the text edit show at least n lines of text
    setMinimumHeight(minLineCount_ * lineHeight_);
    setMaximumHeight(maxLineCount_ * lineHeight_);
}

MultilineTextEdit::~MultilineTextEdit() = default;

void MultilineTextEdit::focusOutEvent(QFocusEvent* e) {
    if (showContextMenu_) {
        return;
    }

    emit editingFinished();
    adjustHeight();
    // clear text selection
    QTextCursor cursor(textCursor());
    cursor.clearSelection();
    setTextCursor(cursor);

    QPlainTextEdit::focusOutEvent(e);
}

void MultilineTextEdit::resizeEvent(QResizeEvent* e) {
    QPlainTextEdit::resizeEvent(e);
    adjustHeight();
}

void MultilineTextEdit::keyPressEvent(QKeyEvent* e) {
    // commit text changes when pressing CTRL + Return or Enter
    if (((e->key() == Qt::Key_Return) || (e->key() == Qt::Key_Enter)) &&
        (e->modifiers() & Qt::ControlModifier)) {
        emit editingFinished();
        adjustHeight();
        e->accept();
    } else {
        QPlainTextEdit::keyPressEvent(e);
    }
}

void MultilineTextEdit::contextMenuEvent(QContextMenuEvent* e) {
    showContextMenu_ = true;
    QPlainTextEdit::contextMenuEvent(e);
    showContextMenu_ = false;
}

void MultilineTextEdit::adjustHeight() {
    qreal textHeight =
        glm::clamp(static_cast<int>(document()->size().height()), minLineCount_, maxLineCount_) *
        lineHeight_;
    QMargins margins(contentsMargins());
    setFixedHeight(static_cast<int>(textHeight + margins.top() + margins.bottom()));
}

}  // namespace inviwo
