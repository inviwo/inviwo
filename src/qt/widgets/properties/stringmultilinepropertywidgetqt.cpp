/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/properties/stringmultilinepropertywidgetqt.h>
#include <inviwo/qt/widgets/properties/compositepropertywidgetqt.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QFontMetrics>
#include <warn/pop>

namespace inviwo {

StringMultilinePropertyWidgetQt::StringMultilinePropertyWidgetQt(StringProperty *property)
    : PropertyWidgetQt(property), property_(property) {
    generateWidget();
    updateFromProperty();
}

void StringMultilinePropertyWidgetQt::generateWidget() {
    QHBoxLayout *hLayout = new QHBoxLayout;
    setSpacingAndMargins(hLayout);

    // QVBoxLayout* vLayout = new QVBoxLayout;
    // vLayout->setContentsMargins(0, 0, 0, 0);
    // vLayout->setSpacing(0);

    label_ = new EditableLabelQt(this, property_);
    // vLayout->addWidget(label_);
    // vLayout->addStretch();
    // hLayout->addLayout(vLayout);

    hLayout->addWidget(label_);

    textEdit_ = new MultilineTextEdit;
    // if(property_->getSemantics().getString() == "Password"){
    //    textEdit_->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    //}

    QSizePolicy sp = textEdit_->sizePolicy();
    sp.setHorizontalStretch(3);
    sp.setVerticalPolicy(QSizePolicy::Preferred);
    textEdit_->setSizePolicy(sp);

    hLayout->addWidget(textEdit_);

    setLayout(hLayout);
    connect(textEdit_, SIGNAL(editingFinished()), this, SLOT(setPropertyValue()));
}

void StringMultilinePropertyWidgetQt::setPropertyValue() {
    std::string valueStr = textEdit_->toPlainText().toLocal8Bit().constData();
    property_->setInitiatingWidget(this);
    property_->set(valueStr);
    property_->clearInitiatingWidget();
}

void StringMultilinePropertyWidgetQt::updateFromProperty() {
    QString text(textEdit_->toPlainText());
    QString newContents(QString::fromStdString(property_->get()));
    if (text != newContents) {
        textEdit_->setPlainText(newContents);
        textEdit_->moveCursor(QTextCursor::Start);

        textEdit_->adjustHeight();
    }
}

MultilineTextEdit::MultilineTextEdit(QWidget *parent)
    : QPlainTextEdit(parent), minLineCount_(2), maxLineCount_(10), showContextMenu_(false) {
    QFontMetrics fontMetric(font());
    lineHeight_ = fontMetric.lineSpacing();

    // enable focus switch by tab (leaving the editor and committing the changes)
    setTabChangesFocus(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    // need to enable the scrollbar for adjusting to its contents
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
#endif
    // make the text edit show at least n lines of text
    setMinimumHeight(minLineCount_ * lineHeight_);
    setMaximumHeight(maxLineCount_ * lineHeight_);
}
MultilineTextEdit::~MultilineTextEdit() {}

void MultilineTextEdit::focusOutEvent(QFocusEvent *e) {
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

void MultilineTextEdit::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);
    adjustHeight();
}

void MultilineTextEdit::keyPressEvent(QKeyEvent *e) {
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

void MultilineTextEdit::contextMenuEvent(QContextMenuEvent *e) {
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

}  // namespace
