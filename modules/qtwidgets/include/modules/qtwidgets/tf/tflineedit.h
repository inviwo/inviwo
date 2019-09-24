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

#ifndef IVW_TFLINEEDIT_H
#define IVW_TFLINEEDIT_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/qtwidgets/properties/doublevaluedragspinbox.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QDoubleValidator>
#include <warn/pop>

namespace inviwo {

/**
 * \class TFLineEdit
 * \brief widget for entering double values within certain bounds and optional range mapping
 */
class IVW_MODULE_QTWIDGETS_API TFLineEdit : public QWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    TFLineEdit(QWidget* parent = nullptr);
    virtual ~TFLineEdit() = default;

    virtual QSize sizeHint() const override;

    /**
     * set the upper and lower bounds of values the line edit should accept
     *
     * @param range   lower and upper bound used for input validation of doubles
     * @param inc      increment for the spinbox
     */
    void setValidRange(const dvec2& range, double inc = 0.01);
    dvec2 getValidRange() const;

    /**
     * enable mapping of normalized values to a different range for display purposes
     *
     * @param enable   if true, values will be mapped to the given range while displayed
     *                 in the line edit
     * @param range    value range used for mapping if enabled
     * @param inc      increment for the spinbox
     */
    void setValueMapping(bool enable, const dvec2& range = dvec2(0.0, 1.0), double inc = 0.01);

    /**
     * set the value of the line edit, if the value is ambiguous nothing will be shown.
     * If value mapping is enabled, the given value will be mapped to the value range
     * prior display.
     */
    void setValue(double value, bool ambiguous);

    double value() const;

signals:
    /**
     * signal when a new valid value has been entered.
     * The given value is already mapped back to the original range if value mapping is enabled.
     */
    void valueChanged(double value);

private:
    bool valueMappingEnabled_ = false;
    dvec2 valueRange_ = dvec2(0.0, 1.0);  //!< used for mapping relative TF positions to values

    double value_;
    bool ambiguous_;

    DoubleValueDragSpinBox spinbox_;
};

}  // namespace inviwo

#endif  // IVW_TFLINEEDIT_H
