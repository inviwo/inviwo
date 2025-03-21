/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <inviwo/core/properties/constraintbehavior.h>  // for ConstraintBehavior, ConstraintBeh...
#include <modules/qtwidgets/numberlineedit.h>           // for NumberLineEdit

#include <limits>  // for numeric_limits

#include <QDoubleSpinBox>  // for QDoubleSpinBox
#include <QHBoxLayout>     // for QHBoxLayout
#include <QSignalBlocker>  // for QSignalBlocker
#include <QSizePolicy>     // for QSizePolicy, QSizePolicy::Minimum

class QHBoxLayout;

namespace inviwo {

BaseOrdinalEditorWidget::BaseOrdinalEditorWidget(bool intMode)
    : editor_{new NumberLineEdit(intMode, this)}
    , minCB_{ConstraintBehavior::Editable}
    , maxCB_{ConstraintBehavior::Editable} {

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
    connect(editor_, static_cast<void (QDoubleSpinBox::*)()>(&QDoubleSpinBox::editingFinished),
            this, &BaseOrdinalEditorWidget::updateFromEditor);
}

BaseOrdinalEditorWidget::~BaseOrdinalEditorWidget() = default;

void BaseOrdinalEditorWidget::updateEditor() {
    QSignalBlocker block{editor_};
    editor_->setValue(transformValueToEditor());
}

void BaseOrdinalEditorWidget::updateFromEditor() {
    newEditorValue(editor_->value());
    emit valueChanged();
}

void BaseOrdinalEditorWidget::applyInit() { updateEditor(); }

void BaseOrdinalEditorWidget::applyValue() {
    applyInit();
    emit valueChanged();
}

void BaseOrdinalEditorWidget::applyMinValue() {
    QSignalBlocker spinBlock(editor_);

    if (minCB_ == ConstraintBehavior::Ignore) {
        editor_->setMinimum(std::numeric_limits<double>::lowest());
    } else {
        editor_->setMinimum(transformMinValueToEditor());
    }
}

void BaseOrdinalEditorWidget::applyMaxValue() {
    QSignalBlocker spinBlock(editor_);

    if (maxCB_ == ConstraintBehavior::Ignore) {
        editor_->setMaximum(std::numeric_limits<double>::max());
    } else {
        editor_->setMaximum(transformMaxValueToEditor());
    }
}

void BaseOrdinalEditorWidget::applyIncrement() {
    QSignalBlocker spinBlock(editor_);

    editor_->setIncrement(transformIncrementToEditor());
    editor_->setDecimals(transformIncrementToEditorDecimals());
}

}  // namespace inviwo
