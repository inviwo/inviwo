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

#ifndef IVW_DOUBLEVALUEDRAGSPINBOX_H
#define IVW_DOUBLEVALUEDRAGSPINBOX_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <warn/pop>

namespace inviwo {

class CustomDoubleSpinBoxQt;

template <typename T>
class ValueDragger;

class IVW_MODULE_QTWIDGETS_API DoubleValueDragSpinBox : public QWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    explicit DoubleValueDragSpinBox(QWidget *parent = nullptr);
    virtual ~DoubleValueDragSpinBox() override = default;

    void setReadOnly(bool r);
    bool isReadOnly() const;

    void setSpecialValueText(const QString &txt);
    QString specialValueText() const;

    void setWrapping(bool w);
    bool wrapping() const;

    QString text() const;

    QString cleanText() const;
    int decimals() const;
    double maximum() const;
    double minimum() const;
    QString prefix() const;
    void setDecimals(int prec);
    void setMaximum(double max);
    void setMinimum(double min);
    void setPrefix(const QString &prefix);
    void setRange(double minimum, double maximum);
    void setSingleStep(double val);
    void setSuffix(const QString &suffix);
    double singleStep() const;
    QString suffix() const;
    double value() const;

signals:
    void valueChanged(double d);
    void valueChanged(const QString &text);
    void editingFinished();

public slots:
    void setValue(double value);
    void selectAll();
    void stepDown();
    void stepUp();

private:
    CustomDoubleSpinBoxQt *spinBox_;
    ValueDragger<double> *valueDragger_;
};

}  // namespace inviwo

#endif  // IVW_DOUBLEVALUEDRAGSPINBOX_H
