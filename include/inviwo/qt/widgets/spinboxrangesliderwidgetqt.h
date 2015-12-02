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

#ifndef IVW_SPINBOXRANGESLIDERQT_H
#define IVW_SPINBOXRANGESLIDERQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/rangesliderqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <QSpinBox>
#include <warn/pop>

namespace inviwo {

class IVW_QTWIDGETS_API SpinBoxRangeSliderQt : public QWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    SpinBoxRangeSliderQt();
    virtual ~SpinBoxRangeSliderQt();
    const int getMinValue();
    const int getMaxValue();
    const int getMinRange();
    const int getMaxRange();
    void setMinValue(const int value);
    void setMaxValue(const int value);
    void setMinRange(const int minRange);
    void setMaxRange(const int maxRange);
    void setMinMaxRange(const int minRange, const int maxRange);

private slots:
    void updateFromSpinBoxMin();
    void updateFromSpinBoxMax();
    void updateFromSlider(int valMin, int valMax);

signals:
    void valuesChanged();

private:
    RangeSliderQt* slider_;
    QSpinBox* spinBoxMin_;
    QSpinBox* spinBoxMax_;
    int minRange_;
    int maxRange_;
};

}//namespace

#endif // IVW_SPINBOXRANGESLIDERQT_H