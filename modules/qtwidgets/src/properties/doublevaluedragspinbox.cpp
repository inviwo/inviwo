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

#include <modules/qtwidgets/properties/doublevaluedragspinbox.h>
#include <modules/qtwidgets/properties/valuedragger.h>
#include <modules/qtwidgets/customdoublespinboxqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <warn/pop>

namespace inviwo {

DoubleValueDragSpinBox::DoubleValueDragSpinBox(QWidget *parent)
    : QWidget(parent)
    , spinBox_(new CustomDoubleSpinBoxQt())
    , valueDragger_(new ValueDragger<double>(spinBox_)) {
    setObjectName("valueDragSpinBox");
    auto layout = new QHBoxLayout();
    layout->setSpacing(2);
    layout->setMargin(0);
    layout->addWidget(valueDragger_);
    layout->addWidget(spinBox_);

    setLayout(layout);

    setFocusProxy(spinBox_);
    spinBox_->setFocusPolicy(Qt::WheelFocus);
    valueDragger_->setFocusPolicy(Qt::ClickFocus);
    spinBox_->setButtonSymbols(QAbstractSpinBox::NoButtons);

    connect(spinBox_, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this,
            static_cast<void (DoubleValueDragSpinBox::*)(double)>(
                &DoubleValueDragSpinBox::valueChanged));
    connect(spinBox_,
            static_cast<void (QDoubleSpinBox::*)(const QString &)>(&QDoubleSpinBox::valueChanged),
            this,
            static_cast<void (DoubleValueDragSpinBox::*)(const QString &)>(
                &DoubleValueDragSpinBox::valueChanged));
    connect(spinBox_, &QSpinBox::editingFinished, this, &DoubleValueDragSpinBox::editingFinished);
}

void DoubleValueDragSpinBox::setReadOnly(bool r) { spinBox_->setReadOnly(r); }

bool DoubleValueDragSpinBox::isReadOnly() const { return spinBox_->isReadOnly(); }

void DoubleValueDragSpinBox::setSpecialValueText(const QString &txt) {
    spinBox_->setSpecialValueText(txt);
}

QString DoubleValueDragSpinBox::specialValueText() const { return spinBox_->specialValueText(); }

void DoubleValueDragSpinBox::setWrapping(bool w) { spinBox_->setWrapping(w); }

bool DoubleValueDragSpinBox::wrapping() const { return spinBox_->wrapping(); }

QString DoubleValueDragSpinBox::text() const { return spinBox_->text(); }

QString DoubleValueDragSpinBox::cleanText() const { return spinBox_->cleanText(); }

int DoubleValueDragSpinBox::decimals() const { return spinBox_->decimals(); }

double DoubleValueDragSpinBox::maximum() const { return spinBox_->maximum(); }

double DoubleValueDragSpinBox::minimum() const { return spinBox_->minimum(); }

QString DoubleValueDragSpinBox::prefix() const { return spinBox_->prefix(); }

void DoubleValueDragSpinBox::setDecimals(int prec) { spinBox_->setDecimals(prec); }

void DoubleValueDragSpinBox::setMaximum(double max) { spinBox_->setMaximum(max); }

void DoubleValueDragSpinBox::setMinimum(double min) { spinBox_->setMinimum(min); }

void DoubleValueDragSpinBox::setPrefix(const QString &prefix) { spinBox_->setPrefix(prefix); }

void DoubleValueDragSpinBox::setRange(double minimum, double maximum) {
    spinBox_->setRange(minimum, maximum);
}

void DoubleValueDragSpinBox::setSingleStep(double val) { spinBox_->setSingleStep(val); }

void DoubleValueDragSpinBox::setSuffix(const QString &suffix) { spinBox_->setSuffix(suffix); }

double DoubleValueDragSpinBox::singleStep() const { return spinBox_->singleStep(); }

QString DoubleValueDragSpinBox::suffix() const { return spinBox_->suffix(); }

double DoubleValueDragSpinBox::value() const { return spinBox_->value(); }

void DoubleValueDragSpinBox::setValue(double value) { spinBox_->setValue(value); }

void DoubleValueDragSpinBox::selectAll() { spinBox_->selectAll(); }

void DoubleValueDragSpinBox::stepDown() { spinBox_->stepDown(); }

void DoubleValueDragSpinBox::stepUp() { spinBox_->stepUp(); }

}  // namespace inviwo
