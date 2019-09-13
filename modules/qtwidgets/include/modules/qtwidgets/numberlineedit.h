/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QDoubleSpinBox>
#include <warn/pop>

namespace inviwo {

class NumberLineEditPrivate;

/**
 * \brief widget for entering numbers with spinbox functionality. It uses less horizontal space than
 * a QSpinBox and allows entering numbers in scientific notation.
 *
 * The widget supports the functionality of a regular QSpinBox, i.e. the value can also be adjusted
 * using arrow keys and mouse wheel. If the widget cannot accomodate the current number
 * representation of the value, the representation will be changed to scientific notation once the
 * widget looses focus. While the widget is in focues the number is shown in regular notation,
 * except for values less than 1e-6 which are depicted in scientific representation.
 */
class IVW_MODULE_QTWIDGETS_API NumberLineEdit : public QDoubleSpinBox {
public:
    explicit NumberLineEdit(QWidget *parent = nullptr);
    NumberLineEdit(bool intMode, QWidget *parent = nullptr);

    virtual ~NumberLineEdit() override;

    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;

    // consider the current size of the widget in order to determine the best suitable number
    // representation, i.e. either regular floating point notation or scientific
    virtual QString textFromValue(double value) const override;
    virtual double valueFromText(const QString &str) const override;

    void setDecimals(int decimals);
    void setMinimum(double min);
    void setMaximum(double max);
    void setRange(double min, double max);

    /**
     * \brief Overrides the timerEvent to prevent
     * spinbox to be updated twice in case of
     * calculations being slow
     */
    virtual void timerEvent(QTimerEvent *event) override;

    bool isValid() const;
    void setInvalid(bool invalid = true);

protected:
    virtual QValidator::State validate(QString &text, int &pos) const override;
    virtual void focusInEvent(QFocusEvent *e) override;
    virtual void focusOutEvent(QFocusEvent *e) override;
    virtual void resizeEvent(QResizeEvent *e) override;
    virtual void changeEvent(QEvent *e) override;
    virtual void wheelEvent(QWheelEvent *e) override;

private:
    const bool integerMode_;
    const int minimumWidth_ = 40;

    QDoubleValidator *validator_;
    mutable QSize cachedMinimumSizeHint_;
    int visibleDecimals_ = 2;
    bool abbreviated_ = true;
    bool invalid_ = false;

    static std::unique_ptr<NumberLineEditPrivate> nlePrivate_;
};

}  // namespace inviwo
