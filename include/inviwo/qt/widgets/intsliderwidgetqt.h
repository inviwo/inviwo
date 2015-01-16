/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_INTSLIDERQT_H
#define IVW_INTSLIDERQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/customsliderwidgetqt.h>
#include <QSpinBox>
#include <QHBoxLayout>
#include <math.h>

namespace inviwo {

//class IVW_QTWIDGETS_API IntSliderWidgetQt : public QWidget {

//    Q_OBJECT
//public:
//
//    IntSliderWidgetQt(int minValue_, int maxValue_, int increment_);
//
//    virtual ~IntSliderWidgetQt();
//
//
//    /**
//     * \brief Returns the integer value of the slider
//     **/
//    int getValue();
//
//    /**
//     * \brief Sets the value of the slider and spinbox
//     */
//    void setValue(int tmpValue);
//
//    /**
//     * \brief sets the maximum value of the spin box and slider
//     */
//    void setMaxValue(int max);
//    /**
//     * \brief sets the minimum value of the spin box and slider
//     */
//    void setMinValue(int min);
//
//    /**
//     * \brief Sets the minimum and maximum values of the slider and spin box
//     */
//    void setRange(int min,int max);
//
//    void setIncrement(int increment);
//
//    /**
//     * \brief updates the value of the spin box from the slider value
//     */
//    void updateValueSpinBox();
//
//    /**
//     * \brief updates the value of the slider from the spin box value
//     */
//    void updateValueSlider();
//
//
//    QSlider* getSlider();
//    QSpinBox* getSpinBox();
//
//    void updateSlider();
//
//private slots:
//    void updateFromSlider();
//    void updateFromSpinBox();
//
//
//signals:
//    void valueChanged(int value);
//
//
//
//private:
//    int value_;
//    int sliderValue_;
//    int maxValue_;
//    int minValue_;
//    int increment_;
//    bool fromSlider_;
//    QSpinBox* spinBox_;
//    CustomSliderWidgetQt* slider_;
//    void generateWidget();
//    static const int SLIDER_MAX = 10000;
//
//};

}//namespace

#endif // IVW_INTSLIDERQT_H