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

#ifndef IVW_FLOATSLIDERQT_H
#define IVW_FLOATSLIDERQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/customsliderwidgetqt.h>
#include <inviwo/qt/widgets/customdoublespinboxqt.h>
#include <QSlider>
#include <QHBoxLayout>
#include <math.h>
#include <sstream>

namespace inviwo {
    
//class IVW_QTWIDGETS_API FloatSliderWidgetQt : public QWidget {
//    Q_OBJECT
//public:
//
//    FloatSliderWidgetQt();
//
//    virtual ~FloatSliderWidgetQt();
//
//    /**
//     * \brief Returns the float value of the slider
//     *
//     * @return float the current value of the slider
//     */
//    float getValue();
//    /**
//     * \brief Sets the float value of the slider.
//     *
//     * Sets the float value of the slider, in order to update the spinbox updateValueSpinbox needs to be called after setting the value
//     *
//     * @param float tmpValue Must be between minValue and maxValue of the slider
//     */
//    void setValue(float value);
//
//    /**
//     * \brief In contrast to setValue(float), initValue(float) does not emit signals. Also this is added due to lack of constructor.
//     *
//     * Sets the float value
//     *
//     * @param float tmpValue Must be between minValue and maxValue of the slider
//     */
//    void initValue(float value);
//    void setMaxValue(float maxValue);
//    void setMinValue(float minValue);
//    void setRange(float minValue, float maxValue);
//    void setIncrement(float increment);
//
//private slots:
//    void updateFromSlider();
//    void updateFromSpinBox();
//
//signals:
//    void valueChanged(float value);
//
//private:
//    float value_;
//    float minValue_;
//    float maxValue_;
//    float increment_;
//    static const int SLIDER_MAX = 10000;
//
//    CustomDoubleSpinBoxQt* spinBox_;
//    CustomSliderWidgetQt* slider_;
//
//    void generateWidget();
//    /**
//     * \brief updates the value of the spin box from the slider value
//     */
//    void updateSpinBox();
//    /**
//     * \brief updates the value of the slider from the spin box value
//     */
//    void updateSlider();
//
//    void setSpinBoxDecimals(float increment);
    
//};

}//namespace

#endif // IVW_FLOATSLIDERQT_H