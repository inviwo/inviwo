/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/qtwidgets/properties/ordinalspinboxwidget.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <warn/pop>

namespace inviwo {

BaseOrdinalSpinBoxWidget::BaseOrdinalSpinBoxWidget() : editor_{new DoubleValueDragSpinBox(this)} {

    setFocusPolicy(editor_->focusPolicy());
    setFocusProxy(editor_);

    QHBoxLayout* hLayout = new QHBoxLayout();

    QSizePolicy sp = editor_->sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Minimum);
    editor_->setSizePolicy(sp);
    editor_->setDecimals(3);
    hLayout->addWidget(editor_);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    setLayout(hLayout);
    connect(editor_, &DoubleValueDragSpinBox::editingFinished, this,
            &BaseOrdinalSpinBoxWidget::updateFromEditor);
    connect(editor_,
            static_cast<void (DoubleValueDragSpinBox::*)(double)>(
                &DoubleValueDragSpinBox::valueChanged),
            this, [&](double) { updateFromEditor(); });
}

BaseOrdinalSpinBoxWidget::~BaseOrdinalSpinBoxWidget() = default;

void BaseOrdinalSpinBoxWidget::setWrapping(bool wrap) { editor_->setWrapping(wrap); }

bool BaseOrdinalSpinBoxWidget::wrapping() const { return editor_->wrapping(); }

void BaseOrdinalSpinBoxWidget::updateEditor() {
    QSignalBlocker block{editor_};
    editor_->setRange(minimumValue(), maximumValue());
    editor_->setValue(transformValueToEditor());
    editor_->setSingleStep(increment());
    editor_->setDecimals(spinnerDecimals());
}

void BaseOrdinalSpinBoxWidget::updateFromEditor() {
    newEditorValue(editor_->value());
    emit valueChanged();
}

void BaseOrdinalSpinBoxWidget::applyInit() { updateEditor(); }

void BaseOrdinalSpinBoxWidget::applyValue() {
    applyInit();
    emit valueChanged();
}

void BaseOrdinalSpinBoxWidget::applyRange() {
    QSignalBlocker block{editor_};
    editor_->setRange(minimumValue(), maximumValue());
}

void BaseOrdinalSpinBoxWidget::applyIncrement() {
    QSignalBlocker block{editor_};
    editor_->setSingleStep(increment());
    editor_->setDecimals(spinnerDecimals());
}

}  // namespace inviwo
