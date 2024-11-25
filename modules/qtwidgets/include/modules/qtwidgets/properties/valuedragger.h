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

#include <QWidget>
#include <QPoint>        // for QPoint
#include <QSize>         // for QSize

class QMouseEvent;
class QPaintEvent;
class QTimerEvent;

namespace inviwo {

class NumberLineEdit;
class IndicatorWidget;

class ValueDragger : public QWidget {
public:
    explicit ValueDragger(NumberLineEdit* spinBox, QWidget* parent = nullptr);
    virtual ~ValueDragger();
    virtual QSize sizeHint() const override;

    void setIncrement(int t);     //!< in percent [0,100]
    void setIncrement(double t);  //!< percentage [0,1]
    void setExponent(double e);

protected:
    virtual void paintEvent(QPaintEvent* e) override;
    virtual void mousePressEvent(QMouseEvent* e) override;
    virtual void mouseReleaseEvent(QMouseEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;

    virtual void timerEvent(QTimerEvent* e) override;

private:
    void reset();

    void setValue(double val);

    const int timerInterval_ = 100;
    double defaultIncrement_ = 0.01;
    double exponent_ = 1.2;

    NumberLineEdit* spinBox_;
    std::unique_ptr<IndicatorWidget> indicator_;
    int spinDeltaTimerId_ = -1;
    QPoint clickPos_;
    bool dragging_ = false;
    double delta_ = 0.0;
    double currentValue_ = 0.0;

    double increment_;
};

}  // namespace inviwo
