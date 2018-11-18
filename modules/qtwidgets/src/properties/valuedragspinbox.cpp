/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/qtwidgets/properties/valuedragspinbox.h>
#include <modules/qtwidgets/properties/valuedragger.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QSpinBox>
#include <QHBoxLayout>
#include <warn/pop>

namespace inviwo {

ValueDragSpinBox::ValueDragSpinBox(QWidget *parent)
    : QWidget(parent), spinBox_(new QSpinBox()), valueDragger_(new ValueDragger<int>(spinBox_)) {
    setObjectName("valueDragSpinBox");
    auto layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(valueDragger_);
    layout->addWidget(spinBox_);

    setLayout(layout);

    setFocusProxy(spinBox_);
    spinBox_->setFocusPolicy(Qt::WheelFocus);
    valueDragger_->setFocusPolicy(Qt::ClickFocus);
    spinBox_->setButtonSymbols(QAbstractSpinBox::NoButtons);

    connect(spinBox_, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            static_cast<void (ValueDragSpinBox::*)(int)>(&ValueDragSpinBox::valueChanged));
    connect(
        spinBox_, static_cast<void (QSpinBox::*)(const QString &)>(&QSpinBox::valueChanged), this,
        static_cast<void (ValueDragSpinBox::*)(const QString &)>(&ValueDragSpinBox::valueChanged));
    connect(spinBox_, &QSpinBox::editingFinished, this, &ValueDragSpinBox::editingFinished);
}

void ValueDragSpinBox::setReadOnly(bool r) { spinBox_->setReadOnly(r); }

bool ValueDragSpinBox::isReadOnly() const { return spinBox_->isReadOnly(); }

void ValueDragSpinBox::setSpecialValueText(const QString &txt) {
    spinBox_->setSpecialValueText(txt);
}

QString ValueDragSpinBox::specialValueText() const { return spinBox_->specialValueText(); }

void ValueDragSpinBox::setWrapping(bool w) { spinBox_->setWrapping(w); }

bool ValueDragSpinBox::wrapping() const { return spinBox_->wrapping(); }

QString ValueDragSpinBox::text() const { return spinBox_->text(); }

QString ValueDragSpinBox::cleanText() const { return spinBox_->cleanText(); }

int ValueDragSpinBox::displayIntegerBase() const { return spinBox_->displayIntegerBase(); }

int ValueDragSpinBox::maximum() const { return spinBox_->maximum(); }

int ValueDragSpinBox::minimum() const { return spinBox_->minimum(); }

QString ValueDragSpinBox::prefix() const { return spinBox_->prefix(); }

void ValueDragSpinBox::setDisplayIntegerBase(int base) { spinBox_->setDisplayIntegerBase(base); }

void ValueDragSpinBox::setMaximum(int max) { spinBox_->setMaximum(max); }

void ValueDragSpinBox::setMinimum(int min) { spinBox_->setMinimum(min); }

void ValueDragSpinBox::setPrefix(const QString &prefix) { spinBox_->setPrefix(prefix); }

void ValueDragSpinBox::setRange(int minimum, int maximum) { spinBox_->setRange(minimum, maximum); }

void ValueDragSpinBox::setSingleStep(int val) { spinBox_->setSingleStep(val); }

void ValueDragSpinBox::setSuffix(const QString &suffix) { spinBox_->setSuffix(suffix); }

int ValueDragSpinBox::singleStep() const { return spinBox_->singleStep(); }

QString ValueDragSpinBox::suffix() const { return spinBox_->suffix(); }

int ValueDragSpinBox::value() const { return spinBox_->value(); }

void ValueDragSpinBox::setValue(int value) { spinBox_->setValue(value); }

void ValueDragSpinBox::selectAll() { spinBox_->selectAll(); }

void ValueDragSpinBox::stepDown() { spinBox_->stepDown(); }

void ValueDragSpinBox::stepUp() { spinBox_->stepUp(); }

}  // namespace inviwo
