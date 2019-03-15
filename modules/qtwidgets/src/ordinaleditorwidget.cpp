/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/qtwidgets/ordinaleditorwidget.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <warn/pop>

namespace inviwo {

BaseOrdinalEditorWidget::BaseOrdinalEditorWidget() : editor_{new IvwLineEdit(this)} {

    QHBoxLayout* hLayout = new QHBoxLayout();

    setFocusPolicy(editor_->focusPolicy());
    setFocusProxy(editor_);

    QSizePolicy sp = editor_->sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Minimum);
    editor_->setSizePolicy(sp);
    hLayout->addWidget(editor_);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    setLayout(hLayout);
    connect(editor_, &IvwLineEdit::editingFinished, this,
            &BaseOrdinalEditorWidget::updateFromEditor);
}

BaseOrdinalEditorWidget::~BaseOrdinalEditorWidget() = default;

void BaseOrdinalEditorWidget::updateEditor() {
    QSignalBlocker block{editor_};
    editor_->setText(transformValueToEditor());
}

void BaseOrdinalEditorWidget::updateFromEditor() {
    newEditorValue(editor_->text());
    emit valueChanged();
}

void BaseOrdinalEditorWidget::applyInit() { updateEditor(); }

void BaseOrdinalEditorWidget::applyValue() {
    applyInit();
    emit valueChanged();
}

}  // namespace inviwo
