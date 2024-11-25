/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <QObject>  // for Q_OBJECT, signals, slots
#include <QString>  // for QString
#include <QWidget>  // for QWidget

namespace inviwo {

class NumberLineEdit;
class ValueDragger;

class IVW_MODULE_QTWIDGETS_API ValueDragSpinBox : public QWidget {
    Q_OBJECT
public:
    explicit ValueDragSpinBox(QWidget* parent = nullptr);
    virtual ~ValueDragSpinBox() override = default;

    void setReadOnly(bool r);
    bool isReadOnly() const;

    void setSpecialValueText(const QString& txt);
    QString specialValueText() const;

    void setWrapping(bool w);
    bool wrapping() const;

    QString text() const;

    QString cleanText() const;
    int displayIntegerBase() const;
    int maximum() const;
    int minimum() const;
    QString prefix() const;
    void setDisplayIntegerBase(int base);
    void setMaximum(int max);
    void setMinimum(int min);
    void setPrefix(const QString& prefix);
    void setRange(int minimum, int maximum);
    /**
     * Sets the increment of a single step to @p inc.
     * If @p inc is zero, the spinbox buttons will be hidden.
     * @see QDoubleSpinBox::setSingleStep
     */
    void setSingleStep(int val);
    void setSuffix(const QString& suffix);
    int singleStep() const;
    QString suffix() const;
    int value() const;

signals:
    void valueChanged(int i);
    void valueChanged(const QString& text);
    void editingFinished();

public slots:
    void setValue(int value);
    void selectAll();
    void stepDown();
    void stepUp();

private:
    NumberLineEdit* spinBox_;
    ValueDragger* valueDragger_;
};

}  // namespace inviwo
